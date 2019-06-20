#include "ES2TS.h"

#define RELEASE_A_OBJECT(x){ if (x) {delete x;} }
#define RELEASE_OBJECTS(x){ if (x) {delete[] x;} }

#define CRC32_POLY 0x04c11db7L

CES2TS::CES2TS(void)
	: m_bInputEnd(false)
	, m_pbTSBuffer(NULL), m_iTSBufferSize(0), m_iESDataBegin(0), m_iESDataEnd(0)
	, m_pbESBuffer(NULL), m_iESBufferSize(0)
	, m_pbAudioTSBuffer(NULL), m_iAudioTSBufferSize(0), m_iAudioTSBufferLen(0)
	, m_got_PCR(false), m_PCR_base(0)
{
	memset(continuity_counter, 0, sizeof(continuity_counter));
	make_crc_table();

	// 25帧出现一次PAT或PMT
	pat_frame_frequency = 15;
	frame_index = 0;

	m_pmt_pid = 0x20;
	m_pid = 0x25;
	video_pid = 0xA0;
	video_stream_id = 0xE0;
	video_stream_type = MPEG2_VIDEO_STREAM_TYPE;
	audio_pid = 0x50;
	audio_stream_id = 0xC0;
	audio_stream_type = MPEG1_AUDIO_STREAM_TYPE;
}


CES2TS::~CES2TS(void)
{
	ReleaseTSBuffer();
	ReleaseESBuffer();
	ReleaseAudioTSBuffer();
}

int CES2TS::SetStreamType(int video_type, int audio_type)
{
	video_stream_type = video_type;
	audio_stream_type = audio_type;
	return 0;
}

int CES2TS::InputESData(byte* pbESData, int iESDataLen, bool bVideo, bool bInputEnd, time_t tNow, int nMiliSeconds)
{
	// 音频数据直接转换成TS
	if (!bVideo)
	{
		if (iESDataLen>MAX_UNIT_SIZE)
		{
			return -1;
		}
		// 检查并分配AudioTS输出缓存区
		if (IsAudioTSBufferEmpty())
		{
			if (AllocateAudioTSBuffer(MAX_UNIT_SIZE)!=0)
			{
				return -1;
			}
		}

		m_iAudioTSBufferLen = m_iAudioTSBufferSize;
		return ESUnit2TS(pbESData, iESDataLen, m_pbAudioTSBuffer, m_iAudioTSBufferLen, false);
	}

	if (tNow == 0)
	{
		timeb tbNow;
		ftime(&tbNow);
		tNow = tbNow.time;
		nMiliSeconds = tbNow.millitm;
	}
	tm *tmNow = localtime(&tNow);
	m_got_PCR = true;
	m_PCR_base = ((tmNow->tm_hour * 60 + tmNow->tm_min)*60 + tmNow->tm_sec)*1000 + nMiliSeconds;
	m_PCR_base = m_PCR_base*90;

	m_bInputEnd = bInputEnd;

	// 检查并分配缓存区
	if (IsESBufferEmpty())
	{
		AllocateESBuffer(MAX_UNIT_SIZE);
	}

	// 将缓存区中剩余的数据移动到开始位置
	if (m_iESDataBegin>0 )
	{
		if (m_iESDataEnd>m_iESDataBegin && m_iESDataEnd<m_iESBufferSize)
		{
			int iMoveLen = (m_iESDataEnd - m_iESDataBegin);
			memmove(m_pbESBuffer, (m_pbESBuffer + m_iESDataBegin), iMoveLen);
			m_iESDataBegin = 0;
			m_iESDataEnd = iMoveLen;
		}
		else
		{
			m_iESDataBegin = 0;
			m_iESDataEnd = 0;
		}
	}

	// 检查缓存区的剩余容量
	if (iESDataLen>m_iESBufferSize)
	{
		return -1;
	}
	if ((m_iESDataEnd + iESDataLen)>m_iESBufferSize)
	{
		// 剩余缓存区内存不够，清除缓存区中已有的数据
		m_iESDataBegin = 0;
		m_iESDataEnd = 0;
	}

	// 将数据复制到缓存区的剩余位置
	memcpy( (m_pbESBuffer + m_iESDataEnd), pbESData, iESDataLen);
	m_iESDataEnd += iESDataLen;

	return 0;
}

int CES2TS::GetTSData(byte* &pbTSData, int &iTSDataLen)
{
	int nRet = GetVideoTSData(pbTSData, iTSDataLen);
	if (nRet!=0)
	{
		nRet = GetAudioTSData(pbTSData, iTSDataLen);
	}
	return nRet;
}

int CES2TS::GetVideoTSData(byte* &pbTSData, int &iTSDataLen)
{
	pbTSData = NULL;
	iTSDataLen = 0;

	// 检查ES数据
	if (IsESBufferEmpty() || !IsESBufferDataIn())
	{
		return -1;
	}

	// 查找第一个ES头
	int iFirst = 0;
	for (iFirst = m_iESDataBegin; iFirst<=(m_iESDataEnd - 7); iFirst++)
	{
		if ( *(m_pbESBuffer + iFirst) == 0x00 
			&& *(m_pbESBuffer + iFirst + 1) == 0x00 
			&& *(m_pbESBuffer + iFirst + 2) == 0x00 
			&& *(m_pbESBuffer + iFirst + 3) == 0x01 )
		{
			break;
		}
	}
	// 移动缓存起始位置
	m_iESDataBegin = iFirst;
	// 未找到ES头
	if (iFirst>(m_iESDataEnd - 7))
	{
		return -1;
	}

	// 查找第二个ES头
	int iSecond = 0;
	for (iSecond = (m_iESDataBegin + 4); iSecond<=(m_iESDataEnd - 7); iSecond++)
	{
		if ( *(m_pbESBuffer + iSecond) == 0x00 
			&& *(m_pbESBuffer + iSecond + 1) == 0x00 
			&& *(m_pbESBuffer + iSecond + 2) == 0x00
			&& *(m_pbESBuffer + iSecond + 3) == 0x01 )
		{
			break;
		}
	}
	if (iSecond>(m_iESDataEnd - 7))
	{
		// 输入结束标志
		if (m_bInputEnd)
		{
			iSecond = m_iESDataEnd;
		}
		else
		{
			// 数据量不够
			return -1;
		}
	}
	int iESDataLen = iSecond - m_iESDataBegin;

	// 检查并分配ES输出缓存区
	if (IsTSBufferEmpty())
	{
		if (AllocateTSBuffer(2*MAX_UNIT_SIZE)!=0)
		{
			return -1;
		}
	}

	// es转换成ts
	iTSDataLen = m_iTSBufferSize;
	int iRet = ESUnit2TS((m_pbESBuffer + m_iESDataBegin), iESDataLen, m_pbTSBuffer, iTSDataLen);
	if (iRet==0)
	{
		pbTSData = m_pbTSBuffer;
	}
	m_iESDataBegin = iSecond;

	return iRet;
}

// 读取TS音频数据, eCodecType类型定义见enumCodecType
int CES2TS::GetAudioTSData(byte* &pbTSData, int &iTSDataLen)
{
	pbTSData = NULL;
	iTSDataLen = 0;

	if (m_pbAudioTSBuffer!=NULL && m_iAudioTSBufferLen>0 && m_iAudioTSBufferLen<=m_iAudioTSBufferSize)
	{
		pbTSData = m_pbAudioTSBuffer;
		iTSDataLen = m_iAudioTSBufferLen;
		m_iAudioTSBufferLen = 0;
		return 0;
	}
	return -1;
}

// 将ES转换成TS
int CES2TS::ESUnit2TS(byte* pbESData, int iESDataLen, byte* pbTSData, int &iTSDataLen, bool bVideo)
{
	byte pes_hdr[TS_PACKET_SIZE];
	int  pes_hdr_len = 0;

	byte stream_id = bVideo?video_stream_id:audio_stream_id;
	//PES_header(iESDataLen, stream_id, false, 0, false, 0, pes_hdr, &pes_hdr_len);
	PES_header(iESDataLen, stream_id, true, m_PCR_base, false, 0, pes_hdr, &pes_hdr_len);

	byte* pbData = pbTSData;
	int nDataRemainLen = iTSDataLen;

	// 写入PAT和PMT
	int nStart = 0;
	int nRet = 0;
	if (bVideo)
	{
		if (frame_index==0)
		{
			nRet = write_TS_program_data(pbData, nDataRemainLen, 1, 1, m_pmt_pid);
			if (nRet==0 && nDataRemainLen>=0 && nDataRemainLen<iTSDataLen)
			{
				nStart = iTSDataLen - nDataRemainLen;
			}
		}
		frame_index++;
		frame_index = frame_index%pat_frame_frequency;
	}
	
	int pid = bVideo?video_pid:audio_pid;
	nRet = write_some_TS_PES_packet((pbData + nStart), nDataRemainLen, pes_hdr, pes_hdr_len, pbESData, iESDataLen, 
		true, true, pid, stream_id, m_got_PCR, m_PCR_base, 0);
	if (nRet==0 && nDataRemainLen>=0 && nDataRemainLen<iTSDataLen)
	{
		m_got_PCR = false;
		iTSDataLen -= nDataRemainLen;
	}
	return nRet;
}

void CES2TS::PES_header(uint32_t data_len, byte stream_id,
	int with_PTS, uint64_t pts, int with_DTS, uint64_t dts,
	byte *PES_hdr, int *PES_hdr_len)
{
	int  extra_len = 0;

	if (with_DTS && !with_PTS)
	{
		with_PTS = true;
		pts = dts;
	}

	// If PTS=DTS then there is no point explictly coding the DTS so junk it
	if (with_DTS && pts == dts)
	{
		with_DTS = false;
	}

	// packet_start_code_prefix
	PES_hdr[0] = 0x00;
	PES_hdr[1] = 0x00;
	PES_hdr[2] = 0x01;

	PES_hdr[3] = stream_id;

	// PES_packet_length comes next, but we'll actually sort it out
	// at the end, when we know what else we've put into our header

	// Flags: '10' then PES_scrambling_control .. original_or_copy
	// If it appears to be an audio stream, we set the data alignment indicator
	// flag, to indicate that the audio data starts with its syncword. For video
	// data, we leave the flag unset.
	if (IS_AUDIO_STREAM_ID(stream_id))
	{
		PES_hdr[6] = 0x84;     // just data alignment indicator flag set
	}
	else
	{
		PES_hdr[6] = 0x80;     // no flags set
	}

	// Flags: PTS_DTS_flags .. PES_extension_flag
	if (with_DTS && with_PTS)
	{
		PES_hdr[7] = 0xC0;
	}
	else if (with_PTS)
	{
		PES_hdr[7] = 0x80;
	}
	else
	{
		PES_hdr[7] = 0x00;     // yet more unset flags (nb: no PTS/DTS info)
	}

	// PES_header_data_length
	if (with_DTS && with_PTS)
	{
		PES_hdr[8] = 0x0A;
		encode_pts_dts(&(PES_hdr[9]), 3, pts);
		encode_pts_dts(&(PES_hdr[14]), 1, dts);
		*PES_hdr_len = 9 + 10;
		extra_len = 3 + 10; // 3 bytes after the length field, plus our PTS & DTS
	}
	else if (with_PTS)
	{
		PES_hdr[8] = 0x05;
		encode_pts_dts(&(PES_hdr[9]), 2, pts);
		*PES_hdr_len = 9 + 5;
		extra_len = 3 + 5; // 3 bytes after the length field, plus our PTS
	}
	else
	{
		PES_hdr[8] = 0x00; // 0 means there is no more data
		*PES_hdr_len = 9;
		extra_len = 3; // just the basic 3 bytes after the length field
	}

	// So now we can set the length field itself...
	if (data_len > 0xFFFF || (data_len + extra_len) > 0xFFFF)
	{
		// If the length is too great, we just set it "unset"
		// @@@ (this should only really be done for TS-wrapped video, so perhaps
		//     we should complain if this is not video?)
		PES_hdr[4] = 0;
		PES_hdr[5] = 0;
	}
	else
	{
		// The packet length doesn't include the bytes up to and including the
		// packet length field, but it *does* include any bytes of the PES header
		// after it.
		data_len += extra_len;
		PES_hdr[4] = (byte) ((data_len & 0xFF00) >> 8);
		PES_hdr[5] = (byte) ((data_len & 0x00FF));
	}
}

void CES2TS::encode_pts_dts(byte data[], int guard_bits, uint64_t value)
{
	int   pts1,pts2,pts3;

#define MAX_PTS_VALUE 0x1FFFFFFFFLL

	if (value > MAX_PTS_VALUE)
	{
		char        *what;
		uint64_t     temp = value;
		while (temp > MAX_PTS_VALUE)
			temp -= MAX_PTS_VALUE;
		switch (guard_bits)
		{
		case 2:  what = "PTS alone"; break;
		case 3:  what = "PTS before DTS"; break;
		case 1:  what = "DTS after PTS"; break;
		default: what = "PTS/DTS/???"; break;
		}
		value = temp;
	}

	pts1 = (int)((value >> 30) & 0x07);
	pts2 = (int)((value >> 15) & 0x7FFF);
	pts3 = (int)( value        & 0x7FFF);

	data[0] =  (guard_bits << 4) | (pts1 << 1) | 0x01;
	data[1] =  (pts2 & 0x7F80) >> 7;
	data[2] = ((pts2 & 0x007F) << 1) | 0x01;
	data[3] =  (pts3 & 0x7F80) >> 7;
	data[4] = ((pts3 & 0x007F) << 1) | 0x01;
}

int CES2TS::write_some_TS_PES_packet(byte* pbTSData, int &iTSDataSize, 
	byte *pes_hdr, int pes_hdr_len,
	byte *data, uint32_t data_len,
	int start, int set_pusi,
	uint32_t pid, byte stream_id,
	int got_PCR, uint64_t PCR_base, uint32_t PCR_extn)
{
#define DEBUG_THIS 0
	byte    TS_packet[TS_PACKET_SIZE];
	int     TS_hdr_len;
	uint32_t controls = 0;
	uint32_t pes_data_len = 0;
	int     err;
	int     got_adaptation_field = false;
	uint32_t space_left;  // Bytes available for payload, after the TS header

	if (pid < 0x0010 || pid > 0x1ffe)
	{
		//fprintf(stderr,"### PID %03x is outside legal program stream range",pid);
		return 1;
	}

	// If this is the first time we've "seen" this data, and it is not
	// already wrapped up as PES, then we need to remember its PES header
	// in our calculations
	if (pes_hdr)
		pes_data_len = data_len + pes_hdr_len;
	else
	{
		pes_hdr_len = 0;
		pes_data_len = data_len;
	}

#if DEBUG_THIS
	if (start)
		printf("TS_PES ");
	else
		printf("       ");
	print_data(stdout,"",data,data_len,20);
#endif

	// We always start with a sync_byte to identify this as a
	// Transport Stream packet
	TS_packet[0] = 0x47;
	// Should we set the "payload_unit_start_indicator" bit?
	// Only for the first packet containing our data.
	if (start && set_pusi)
		TS_packet[1] = (byte)(0x40 | ((pid & 0x1f00) >> 8));
	else
		TS_packet[1] = (byte)(0x00 | ((pid & 0x1f00) >> 8));
	TS_packet[2] = (byte)(pid & 0xff);

	// Sort out the adaptation field, if any
	if (start && got_PCR)
	{
		// This is the start of the data, and we have a PCR value to output,
		// so we know we have an adaptation field
		controls = 0x30;  // adaptation field control = '11' = both
		TS_packet[3] = (byte) (controls | next_continuity_count(pid));
		// And construct said adaptation field...
		TS_packet[4]  = 7; // initial adaptation field length
		TS_packet[5]  = 0x10;  // flag bits 0001 0000 -> got PCR
		TS_packet[6]  = (byte)   (PCR_base >> 25);
		TS_packet[7]  = (byte)  ((PCR_base >> 17) & 0xFF);
		TS_packet[8]  = (byte)  ((PCR_base >>  9) & 0xFF);
		TS_packet[9]  = (byte)  ((PCR_base >>  1) & 0xFF);
		TS_packet[10] = (byte) (((PCR_base & 0x1) << 7) | 0x7E | (PCR_extn >> 8));
		TS_packet[11] = (byte)  (PCR_extn >>  1);
		TS_hdr_len = 12;
		space_left = MAX_TS_PAYLOAD_SIZE - 8;
		got_adaptation_field = true;
#if DEBUG_THIS
		printf("       start & got_PCR -> with adaptation field, space left %d, TS_packet[4] %d\n",space_left,TS_packet[4]);
#endif
	}
	else if (pes_data_len < MAX_TS_PAYLOAD_SIZE)
	{
		// Our data is less than 184 bytes long, which means it won't fill
		// the payload, so we need to pad it out with an (empty) adaptation
		// field, padded to the appropriate length
		controls = 0x30;  // adaptation field control = '11' = both
		TS_packet[3] = (byte)(controls | next_continuity_count(pid));
		if (pes_data_len == (MAX_TS_PAYLOAD_SIZE - 1))  // i.e., 183
		{
			TS_packet[4] = 0; // just the length used to pad
			TS_hdr_len = 5;
			space_left = MAX_TS_PAYLOAD_SIZE - 1;
		}
		else
		{
			TS_packet[4] = 1; // initial length
			TS_packet[5] = 0;  // unset flag bits
			TS_hdr_len = 6;
			space_left = MAX_TS_PAYLOAD_SIZE - 2;  // i.e., 182
		}
		got_adaptation_field = true;
#if DEBUG_THIS
		printf("       <184, pad with empty adaptation field, space left %d, TS_packet[4] %d\n",space_left,TS_packet[4]);
#endif
	}
	else
	{
		// The data either fits exactly, or is too long and will need to be
		// continued in further TS packets. In either case, we don't need an
		// adaptation field
		controls = 0x10;  // adaptation field control = '01' = payload only
		TS_packet[3] = (byte)(controls | next_continuity_count(pid));
		TS_hdr_len = 4;
		space_left = MAX_TS_PAYLOAD_SIZE;
#if DEBUG_THIS
		printf("       >=184, space left %d\n",space_left);
#endif
	}

	if (got_adaptation_field)
	{
		// Do we need to add stuffing bytes to allow for short PES data?
		if (pes_data_len < space_left)
		{
			int ii;
			int padlen = space_left - pes_data_len;
			for (ii = 0; ii < padlen; ii++)
				TS_packet[TS_hdr_len+ii] = 0xFF;
			TS_packet[4] += padlen;
			TS_hdr_len   += padlen;
			space_left   -= padlen;
#if DEBUG_THIS
			printf("       stuffing %d, space left %d, TS_packet[4] %d\n",padlen,space_left,TS_packet[4]);
#endif
		}
	}

	if (pes_data_len == space_left)
	{
#if DEBUG_THIS
		printf("       == fits exactly\n");
#endif
		// Our data fits exactly
		err = write_TS_packet_parts(pbTSData, iTSDataSize, 
			TS_packet,TS_hdr_len,
			pes_hdr,pes_hdr_len,
			data,data_len,
			pid,got_PCR,(PCR_base*300)+PCR_extn);
		if (err) return err;
	}
	else
	{
		// We need to look at more than one packet...
		// Write out the first 184-pes_hdr_len bytes
		int increment = space_left - pes_hdr_len;
		err = write_TS_packet_parts(pbTSData, iTSDataSize, 
			TS_packet,TS_hdr_len,
			pes_hdr,pes_hdr_len,
			data,increment,
			pid,got_PCR,(PCR_base*300)+PCR_extn);
		if (err) return err;
#if DEBUG_THIS
		printf("       == wrote %d, leaving %d\n",increment,data_len-increment);
#endif
		// Leaving data_len - (184-pes_hdr_len) bytes still to go
		// Is recursion going to be efficient enough?
		if ((data_len - increment) > 0)
		{
			err = write_some_TS_PES_packet(pbTSData, iTSDataSize,
				NULL, 0, 
				&(data[increment]),data_len-increment,
				false,false,pid,stream_id,false,0,0);
			if (err) return err;
		}
	}
	return 0;
}

int CES2TS::write_TS_packet_parts(byte* &pbTSData, int &iTSDataSize, 
	byte TS_packet[TS_PACKET_SIZE], int TS_hdr_len,
	byte pes_hdr[], int pes_hdr_len,
	byte data[], int data_len,
	uint32_t pid, int got_pcr, uint64_t pcr)
{
	//int err;
	int total_len  = TS_hdr_len + pes_hdr_len + data_len;

	if (total_len != TS_PACKET_SIZE)
	{
		fprintf(stderr,
			"### TS packet length is %d, not 188 (composed of %d + %d + %d)\n",
			total_len,TS_hdr_len,pes_hdr_len,data_len);
		return 1;
	}

	// We want to make a single write, so we need to assemble the package
	// into our packet buffer
	if (pes_hdr_len > 0)
		memcpy(&(TS_packet[TS_hdr_len]),pes_hdr,pes_hdr_len);

	if (data_len > 0)
		memcpy(&(TS_packet[TS_hdr_len+pes_hdr_len]),data,data_len);

	//err = write_to_buffered_TS_output(tswriter->writer, TS_packet, (tswriter->count)++, pid, got_pcr, pcr);
	if (iTSDataSize>TS_PACKET_SIZE)
	{
		memcpy(pbTSData, TS_packet, TS_PACKET_SIZE);
		pbTSData += TS_PACKET_SIZE;
		iTSDataSize -= TS_PACKET_SIZE;
	}
	else
	{
		//fprintf(stderr,"### Error writing out TS packet: %s\n",strerror(errno));
		return 1;
	}
	return 0;
}

// 将es文件转换成ts文件
int CES2TS::ES2TS(char* pInESFilePath, char* pOutTSFilePath)
{
	int32_t iESUnitBufSize = MAX_UNIT_SIZE;
	byte *pbESUnitBuf = new byte[iESUnitBufSize];
	if (NULL==pbESUnitBuf)
	{
		return -1;
	}

	// 打开写入的ES文件
	FILE *input = fopen(pInESFilePath, "rb");
	if (NULL==input)
	{
		// 释放缓存
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	// 打开写入的TS文件
	FILE *output = fopen(pOutTSFilePath, "wb");
	if (NULL==output)
	{
		// 关闭输入文件
		if (NULL!=input)
		{
			fclose(input);
			input = NULL;
		}
		// 释放缓存
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	time_t tNow = time(NULL);
	int nMiliSeconds = 0;

	SetStreamType(AVC_VIDEO_STREAM_TYPE);
	while(!feof(input))
	{
		int iReadLen = fread(pbESUnitBuf, 1, 10*1024, input);
		if ( ferror(input) )
		{
			break;
		}

		// 传入ES数据
		InputESData(pbESUnitBuf, iReadLen, true, (feof(input)==0)?false:true, tNow, nMiliSeconds);
		nMiliSeconds += 40;
		tNow += nMiliSeconds/1000;
		nMiliSeconds %= 1000;

		byte* pbTSData = NULL;
		int iTSDataLen = 0;
 		while( GetTSData(pbTSData, iTSDataLen)==0 )
		{
			if (pbTSData!=NULL && iTSDataLen>0)
			{
				fwrite(pbTSData, 1, iTSDataLen, output);
				fflush(output);
			}
		}
	}

	// 关闭输入文件
	if (NULL!=input)
	{
		fclose(input);
		input = NULL;
	}
	// 关闭输出文件
	if (NULL!=output)
	{
		fclose(output);
		output = NULL;
	}
	// 释放缓存
	RELEASE_OBJECTS(pbESUnitBuf);

	return 0;
}

int CES2TS::write_TS_program_data(byte* pbTSData, int &iTSDataSize, 
	uint32_t    transport_stream_id,
	uint32_t    program_number,
	uint32_t    pmt_pid)
{
	int                   err;
	pidint_list_p         prog_list;
	pmt_p                 pmt;

	// We have a single program stream
	err = build_pidint_list(&prog_list);
	if (err) return 1;
	err = append_to_pidint_list(prog_list,pmt_pid,program_number);
	if (err)
	{
		free_pidint_list(&prog_list);
		return 1;
	}

	pmt = build_pmt((uint16_t)program_number,0,video_pid);  // Use program stream PID as PCR PID
	if (pmt == NULL)
	{
		free_pidint_list(&prog_list);
		return 1;
	}
	err = add_stream_to_pmt(pmt,video_pid,video_stream_type,0,NULL);
	if (audio_stream_type!=0)
	{
		err = add_stream_to_pmt(pmt,audio_pid,audio_stream_type,0,NULL);
	}
	if (err)
	{
		free_pidint_list(&prog_list);
		free_pmt(&pmt);
		return 1;
	}

	err = write_pat_and_pmt(pbTSData, iTSDataSize, transport_stream_id,prog_list,pmt_pid,pmt);
	if (err)
	{
		free_pidint_list(&prog_list);
		free_pmt(&pmt);
		return 1;
	}

	free_pidint_list(&prog_list);
	free_pmt(&pmt);
	return 0;
}

int CES2TS::write_pat_and_pmt(byte* pbTSData, int &iTSDataSize, 
	uint32_t       transport_stream_id,
	pidint_list_p  prog_list,
	uint32_t       pmt_pid,
	pmt_p          pmt)
{
	int err;
	err = write_pat(pbTSData, iTSDataSize, transport_stream_id, prog_list);
	if (err) return 1;
	err = write_pmt(pbTSData, iTSDataSize, pmt_pid,pmt);
	if (err) return 1;
	return 0;
}

void CES2TS::make_crc_table()
{
	int i, j;
	int already_done = 0;
	uint32_t crc;

	if (already_done)
		return;
	else
		already_done = 1;

	for (i = 0; i < 256; i++)
	{
		crc = i << 24;
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x80000000L)
				crc = (crc << 1) ^ CRC32_POLY;
			else
				crc = ( crc << 1 );
		}
		crc_table[i] = crc;
	}

}

uint32_t CES2TS::crc32_block(uint32_t crc, byte *pData, int blk_len)
{
	int i, j;
	for (j = 0; j < blk_len; j++)
	{
		i = ((crc >> 24) ^ *pData++) & 0xff;
		crc = (crc << 8) ^ crc_table[i];
	}
	return crc;
}

int CES2TS::write_pat(byte* &pbTSData, int &iTSDataSize, 
	uint32_t       transport_stream_id,
	pidint_list_p  prog_list)
{
	int      ii;
	byte     data[1021+3];
	byte     TS_packet[TS_PACKET_SIZE];
	int      TS_hdr_len;
	int      err;
	int      section_length;
	int      offset, data_length;
	uint32_t crc32;

#if DEBUG_WRITE_PACKETS
	printf("|| PAT pid 0\n");
#endif

	section_length = 9 + prog_list->length * 4;
	if (section_length > 1021)
	{
		fprintf(stderr,"### PAT data is too long - will not fit in 1021 bytes\n");
		// TODO: Ideally, would be to stderr
		report_pidint_list(prog_list,"Program list","Program",FALSE);
		return 1;
	}

	data[0] = 0x00;
	// The section length is fixed because our data is fixed
	data[1] = (byte) (0xb0 | ((section_length & 0x0F00) >> 8));
	data[2] = (byte) (section_length & 0x0FF);
	data[3] = (byte) ((transport_stream_id & 0xFF00) >> 8);
	data[4] = (byte)  (transport_stream_id & 0x00FF);
	// For simplicity, we'll have a version_id of 0
	data[5] = 0xc1;
	// First section of the PAT has section number 0, and there is only
	// that section
	data[6] = 0x00;
	data[7] = 0x00;

	offset = 8;
	for (ii = 0; ii < prog_list->length; ii++)
	{
		data[offset+0] = (byte) ((prog_list->number[ii] & 0xFF00) >> 8);
		data[offset+1] = (byte)  (prog_list->number[ii] & 0x00FF);
		data[offset+2] = (byte) (0xE0 | ((prog_list->pid[ii] & 0x1F00) >> 8));
		data[offset+3] = (byte) (prog_list->pid[ii] & 0x00FF);
		offset += 4;
	}

	crc32 = crc32_block(0xffffffff,data,offset);
	data[12] = (byte) ((crc32 & 0xff000000) >> 24);
	data[13] = (byte) ((crc32 & 0x00ff0000) >> 16);
	data[14] = (byte) ((crc32 & 0x0000ff00) >>  8);
	data[15] = (byte)  (crc32 & 0x000000ff);
	data_length = offset+4;

#if 1
	if (data_length != section_length + 3)
	{
		fprintf(stderr,"### PAT length %d, section length+3 %d\n",
			data_length,section_length+3);
		return 1;
	}
#endif

	crc32 = crc32_block(0xffffffff,data,data_length);
	if (crc32 != 0)
	{
		fprintf(stderr,"### PAT CRC does not self-cancel\n");
		return 1;
	}
	err = TS_program_packet_hdr(0x00,data_length,TS_packet,&TS_hdr_len);
	if (err)
	{
		fprintf(stderr,"### Error constructing PAT packet header\n");
		return 1;
	}
	err = write_TS_packet_parts(pbTSData, iTSDataSize, TS_packet,TS_hdr_len,NULL,0,data,data_length,0x00,FALSE,0);
	if (err)
	{
		fprintf(stderr,"### Error writing PAT\n");
		return 1;
	}
	return 0;
}

int CES2TS::write_pmt(byte* &pbTSData, int &iTSDataSize, 
	uint32_t    pmt_pid,
	pmt_p       pmt)
{
	int      ii;
	byte     data[3+1021];	// maximum PMT size
	byte     TS_packet[TS_PACKET_SIZE];
	int      TS_hdr_len;
	int      err;
	int      section_length;
	int      offset, data_length;
	uint32_t crc32;

#if DEBUG_WRITE_PACKETS
	printf("|| PMT pid %x (%d)\n",pmt_pid,pmt_pid);
#endif

	if (pmt_pid < 0x0010 || pmt_pid > 0x1ffe)
	{
		fprintf(stderr,"### PMT PID %03x is outside legal range\n",pmt_pid);
		return 1;
	}
	if (pid_in_pmt(pmt,pmt_pid))
	{
		fprintf(stderr,"### PMT PID and program %d PID are both %03x\n",
			pid_index_in_pmt(pmt,pmt_pid),pmt_pid);
		return 1;
	}

	// Much of the PMT should look very familiar, after the PAT

	// Calculate the length of the section
	section_length = 13 + pmt->program_info_length;
	for (ii = 0; ii < pmt->num_streams; ii++)
		section_length += 5 + pmt->streams[ii].ES_info_length;
	if (section_length > 1021)
	{
		fprintf(stderr,"### PMT data is too long - will not fit in 1021 bytes\n");
		report_pmt(stderr,"    ",pmt);
		return 1;
	}

	data[0] = 0x02;
	data[1] = (byte) (0xb0 | ((section_length & 0x0F00) >> 8));
	data[2] = (byte) (section_length & 0x0FF);
	data[3] = (byte) ((pmt->program_number & 0xFF00) >> 8);
	data[4] = (byte)  (pmt->program_number & 0x00FF);
	data[5] = 0xc1;
	data[6] = 0x00; // section number
	data[7] = 0x00; // last section number
	data[8] = (byte) (0xE0 | ((pmt->PCR_pid & 0x1F00) >> 8));
	data[9] = (byte) (pmt->PCR_pid & 0x00FF);
	data[10] = 0xF0;
	data[11] = (byte)pmt->program_info_length;
	if (pmt->program_info_length > 0)
		memcpy(data+12,pmt->program_info,pmt->program_info_length);

	offset = 12 + pmt->program_info_length;

	for (ii=0; ii < pmt->num_streams; ii++)
	{
		uint32_t pid = pmt->streams[ii].elementary_PID;
		uint16_t len = pmt->streams[ii].ES_info_length;
		data[offset+0] = pmt->streams[ii].stream_type;
		data[offset+1] = (byte) (0xE0 | ((pid & 0x1F00) >> 8));
		data[offset+2] = (byte) (pid & 0x00FF);
		data[offset+3] = ((len & 0xFF00) >> 8) | 0xF0;
		data[offset+4] =   len & 0x00FF;
		memcpy(data+offset+5,pmt->streams[ii].ES_info,len);
		offset += 5 + len;
	}

	crc32 = crc32_block(0xffffffff,data,offset);
	data[offset+0] = (byte) ((crc32 & 0xff000000) >> 24);
	data[offset+1] = (byte) ((crc32 & 0x00ff0000) >> 16);
	data[offset+2] = (byte) ((crc32 & 0x0000ff00) >>  8);
	data[offset+3] = (byte)  (crc32 & 0x000000ff);
	data_length = offset + 4;

#if 1
	if (data_length != section_length + 3)
	{
		fprintf(stderr,"### PMT length %d, section length+3 %d\n",
			data_length,section_length+3);
		return 1;
	}
#endif

	crc32 = crc32_block(0xffffffff,data,data_length);
	if (crc32 != 0)
	{
		fprintf(stderr,"### PMT CRC does not self-cancel\n");
		return 1;
	}
	err = TS_program_packet_hdr(pmt_pid,data_length,TS_packet,&TS_hdr_len);
	if (err)
	{
		fprintf(stderr,"### Error constructing PMT packet header\n");
		return 1;
	}
	err = write_TS_packet_parts(pbTSData, iTSDataSize, TS_packet,TS_hdr_len,NULL,0,
		data,data_length,0x02,FALSE,0);
	if (err)
	{
		fprintf(stderr,"### Error writing PMT\n");
		return 1;
	}
	return 0;
}

int CES2TS::TS_program_packet_hdr(uint32_t pid,
	int      data_len,
	byte     TS_hdr[TS_PACKET_SIZE],
	int     *TS_hdr_len)
{
	uint32_t controls = 0;
	int     pointer, ii;

	if (data_len > (TS_PACKET_SIZE - 5))  // i.e., 183
	{
		fprintf(stderr,"### PMT/PAT data for PID %02x is too long (%d > 183)",
			pid,data_len);
		return 1;
	}

	// We always start with a sync_byte to identify this as a
	// Transport Stream packet

	TS_hdr[0] = 0x47;
	// We want the "payload_unit_start_indicator" bit set
	TS_hdr[1] = (byte)(0x40 | ((pid & 0x1f00) >> 8));
	TS_hdr[2] = (byte)(pid & 0xff);
	// We don't need any adaptation field controls
	controls = 0x10;
	TS_hdr[3] = (byte)(controls | next_continuity_count(pid));

	// Next comes a pointer to the actual payload data
	// (i.e., 0 if the data is 183 bytes long)
	// followed by pad bytes until we *get* to the data
	pointer = (byte)(TS_PACKET_SIZE - 5 - data_len);
	TS_hdr[4] = pointer;
	for (ii=0; ii<pointer; ii++)
		TS_hdr[5+ii] = 0xff;

	*TS_hdr_len = 5+pointer;
	return 0;
}
