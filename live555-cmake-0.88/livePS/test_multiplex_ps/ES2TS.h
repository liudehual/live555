#ifndef __ES_2_TS_H__
#define __ES_2_TS_H__

#include "comdef.h"
extern "C" 
{
	#include "pidint_defns.h"
	#include "pidint_fns.h"
}

#define TS_PACKET_SIZE 188

#define IS_AUDIO_STREAM_ID(id) ((id)==0xBD || ((id) >= 0xC0 && (id) <= 0xDF))
#define IS_VIDEO_STREAM_ID(id) ((id) >= 0xE0 && (id) <= 0xEF)

#define DEFAULT_VIDEO_STREAM_ID  0xE0   // i.e., stream 0
#define DEFAULT_AUDIO_STREAM_ID  0xC0   // i.e., stream 0

#define TS_PACKET_SIZE 188
#define MAX_TS_PAYLOAD_SIZE  (TS_PACKET_SIZE-4)

#include <stdio.h>
#include <stdlib.h>

#define MPEG1_VIDEO_STREAM_TYPE			0x01
#define MPEG2_VIDEO_STREAM_TYPE			0x02  // H.262
#define AVC_VIDEO_STREAM_TYPE			0x1B  // MPEG-4 part10 - H.264
#define AVS_VIDEO_STREAM_TYPE           0x42  // AVS -- Chinese standard
#define DVB_DOLBY_AUDIO_STREAM_TYPE		0x06  // [1]
#define ATSC_DOLBY_AUDIO_STREAM_TYPE	0x81  // [1]
#define MPEG2_AUDIO_STREAM_TYPE			0x04
#define MPEG1_AUDIO_STREAM_TYPE			0x03
#define ADTS_AUDIO_STREAM_TYPE			0x0F  // AAC ADTS
#define MPEG4_PART2_VIDEO_STREAM_TYPE   0x10
#define LATM_AUDIO_STREAM_TYPE          0x11

#define DOLBY_DVB_STREAM_TYPE           0x06  // [1]
#define DOLBY_ATSC_STREAM_TYPE          0x81  // [1]

#define G711_AUDIO_STREAM_TYPE          0x90  // [1]


class CES2TS
{
public:
	CES2TS(void);
	~CES2TS(void);

public:
	int SetStreamType(int video_type, int audio_type = 0);
	// 传入ES数据,数据包长度需小于MAX_UNIT_SIZE
	// bInputEnd：输入结束标志，避免最后一个TS包数据的丢失
	int InputESData(byte* pbESData, int iESDataLen, bool bVideo = true, bool bInputEnd = false, 
		time_t tNow = 0, int nMiliSeconds = 0);
	// 读取TS数据, eCodecType类型定义见enumCodecType
	int GetTSData(byte* &pbTSData, int &iTSDataLen);

private:
	bool m_got_PCR;
	uint64_t m_PCR_base;

private:
	// 读取TS视频数据, eCodecType类型定义见enumCodecType
	int GetVideoTSData(byte* &pbTSData, int &iTSDataLen);
	// 读取TS音频数据, eCodecType类型定义见enumCodecType
	int GetAudioTSData(byte* &pbTSData, int &iTSDataLen);

	// 将ES转换成TS
	int ESUnit2TS(byte* pbESData, int iESDataLen, byte* pbTSData, int &iTSDataLen, bool bVideo=true);

	void PES_header(uint32_t data_len, byte stream_id,
		int with_PTS, uint64_t pts, int with_DTS, uint64_t dts,
		byte *PES_hdr, int *PES_hdr_len);

	void encode_pts_dts(byte data[], int guard_bits, uint64_t value);

	int write_some_TS_PES_packet(byte* pbTSData, int &iTSDataSize, 
		byte *pes_hdr, int pes_hdr_len,
		byte *data, uint32_t data_len,
		int start, int set_pusi,
		uint32_t pid, byte stream_id,
		int got_PCR, uint64_t PCR_base, uint32_t PCR_extn);

	inline int next_continuity_count(uint32_t pid)
	{
		uint32_t next = (continuity_counter[pid] + 1) & 0x0f;
		continuity_counter[pid] = next;
		return next;
	}
	int continuity_counter[0x1fff+1];

	int write_TS_packet_parts(byte* &pbTSData, int &iTSDataSize, 
		byte TS_packet[TS_PACKET_SIZE], int TS_hdr_len,
		byte pes_hdr[], int pes_hdr_len,
		byte data[], int data_len,
		uint32_t pid, int got_pcr, uint64_t pcr);

private:
	// 输入结束标志，避免避免最后一个ES包数据的丢失
	bool m_bInputEnd;

private:
	int AllocateESBuffer(int nSize)
	{
		m_pbESBuffer = new byte[nSize];
		if (NULL==m_pbESBuffer)
		{
			return -1;
		}
		m_iESBufferSize = nSize;
		m_iESDataBegin = 0;
		m_iESDataEnd = 0;
		return 0;
	}
	void ReleaseESBuffer()
	{
		if (NULL!=m_pbESBuffer)
		{
			delete[] m_pbESBuffer;
			m_pbESBuffer = NULL;
		}
		m_iESBufferSize = 0;
		m_iESDataBegin = 0;
		m_iESDataEnd = 0;
	}
	bool IsESBufferEmpty()
	{
		return (m_pbESBuffer==NULL)?true:false;
	}
	bool IsESBufferDataIn()
	{
		if ( m_iESDataBegin>=0 && m_iESDataEnd<=m_iESBufferSize && (m_iESDataEnd - m_iESDataBegin)>0 )
		{
			return true;
		}
		return false;
	}
	byte* m_pbESBuffer;
	int m_iESBufferSize;

	int m_iESDataBegin;
	int m_iESDataEnd;

private:
	int AllocateTSBuffer(int nSize)
	{
		m_pbTSBuffer = new byte[nSize];
		if (NULL==m_pbTSBuffer)
		{
			return -1;
		}
		m_iTSBufferSize = nSize;
		return 0;
	}
	void ReleaseTSBuffer()
	{
		if (NULL!=m_pbTSBuffer)
		{
			delete[] m_pbTSBuffer;
			m_pbTSBuffer = NULL;
		}
		m_iTSBufferSize = 0;
	}
	bool IsTSBufferEmpty()
	{
		return (m_pbTSBuffer==NULL)?true:false;
	}
	byte* m_pbTSBuffer;
	int m_iTSBufferSize;

private:
	int AllocateAudioTSBuffer(int nSize)
	{
		m_pbAudioTSBuffer = new byte[nSize];
		if (NULL==m_pbAudioTSBuffer)
		{
			return -1;
		}
		m_iAudioTSBufferSize = nSize;
		return 0;
	}
	void ReleaseAudioTSBuffer()
	{
		if (NULL!=m_pbAudioTSBuffer)
		{
			delete[] m_pbAudioTSBuffer;
			m_pbAudioTSBuffer = NULL;
		}
		m_iAudioTSBufferSize = 0;
	}
	bool IsAudioTSBufferEmpty()
	{
		return (m_pbAudioTSBuffer==NULL)?true:false;
	}
	byte* m_pbAudioTSBuffer;
	int m_iAudioTSBufferSize;
	int m_iAudioTSBufferLen;

// For test
public:
	// 将ES文件转换成TS文件
	int ES2TS(char* pInESFilePath, char* pOutTSFilePath);

private:
	int write_TS_program_data(byte* pbTSData, int &iTSDataSize, 
		uint32_t    transport_stream_id,
		uint32_t    program_number,
		uint32_t    pmt_pid);
	int write_pat_and_pmt(byte* pbTSData, int &iTSDataSize, 
		uint32_t       transport_stream_id,
		pidint_list_p  prog_list,
		uint32_t       pmt_pid,
		pmt_p          pmt);
	int write_pat(byte* &pbTSData, int &iTSDataSize, 
		uint32_t       transport_stream_id,
		pidint_list_p  prog_list);
	int write_pmt(byte* &pbTSData, int &iTSDataSize, 
		uint32_t    pmt_pid,
		pmt_p       pmt);
	int TS_program_packet_hdr(uint32_t pid,
		int      data_len,
		byte     TS_hdr[TS_PACKET_SIZE],
		int     *TS_hdr_len);

	uint32_t crc32_block(uint32_t crc, byte *pData, int blk_len);
	void make_crc_table();
	uint32_t crc_table[256];

private:
	// pat和pmt出现的频率(多少帧出现一个)
	int pat_frame_frequency;
	int frame_index;

private:
	int m_pmt_pid;
	int m_pid;
	int video_pid;
	byte video_stream_id;
	int video_stream_type;
	int audio_pid;
	byte audio_stream_id;
	int audio_stream_type;
};
#endif
