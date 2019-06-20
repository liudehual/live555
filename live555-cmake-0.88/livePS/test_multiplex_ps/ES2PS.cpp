#include "ES2PS.h"

#define MAX_PES_HEADER_SIZE 512
#define MAX_PS_HEADER_SIZE 512

#ifndef MAX_UNIT_SIZE
#define MAX_UNIT_SIZE (1024*1024)
#endif
#ifndef MAX_PES_DATA_SIZE
#define MAX_PES_DATA_SIZE (60*1024)
#endif

#define RELEASE_A_OBJECT(x){ if (x) {delete x;} }
#define RELEASE_OBJECTS(x){ if (x) {delete[] x;} }

CES2PS::CES2PS(void)
	: m_pbPSBuffer(NULL),m_iPSBufferSize(0),
	m_pbESBuffer(NULL), m_iESBufferSize(0), m_iESDataBegin(0), 
	m_iESDataEnd(0),m_ubiPrevPTSTime(0),m_ubiPTSTimeOffset(0),m_bInputEnd(false)
{

}

CES2PS::~CES2PS(void)
{
	ReleasePSBuffer();
	ReleaseESBuffer();
}

// Convert an unit from es to ps.
int CES2PS::ES2PS(byte* bpInESData, int iInESDataLen, byte* &bpOutPSData, int& iOutPSDataSize, bool bIsVideoStream, uint64_t nInPTSTime)
{
	bpOutPSData = NULL;
	iOutPSDataSize = 0;

	// 根据音视频区分stream_id
	byte bStreamId = (bIsVideoStream==true)?0xE0:0xC0;

	// 计算当前系统时间
	if ( 0==nInPTSTime )
	{
		timeb tNow;
		ftime(&tNow);
		tm *tmNow = localtime(&(tNow.time));
		nInPTSTime = (tmNow->tm_hour*60 + tmNow->tm_min)*60 + tmNow->tm_sec;
		nInPTSTime *= 1000;
		nInPTSTime += tNow.millitm;
	}

	// 检查并分配PS缓存区
	if (IsPSBufferEmpty())
	{
		if (AllocatePSBuffer(MAX_UNIT_SIZE*2))
		{
			return -1;
		}
	}

	int iOutPSDataLen = 0;
	if (!bIsVideoStream)
	{
		iOutPSDataLen = m_iPSBufferSize;
		// 音频数据直接封装成PES包
		int iRet = es2ps(bpInESData, iInESDataLen, m_pbPSBuffer, iOutPSDataLen, bStreamId, nInPTSTime);
		if (iRet!=0)
		{
			return iRet;
		}
	}
	else
	{
		// 将数据存入ES缓冲区中
		InputESData(bpInESData, iInESDataLen, false);

		while (m_iESDataBegin<=(m_iESDataEnd - 9))
		{
			// 查找第一个ES头
			int iFirst = 0;
			for (iFirst = m_iESDataBegin; iFirst<=(m_iESDataEnd - 9); iFirst++)
			{
				if ( *(m_pbESBuffer + iFirst) == 0x00 
					&& *(m_pbESBuffer + iFirst + 1) == 0x00 
					&& *(m_pbESBuffer + iFirst + 2) == 0x00 
					&& *(m_pbESBuffer + iFirst + 3) == 0x01 
					&& 
					!(*(m_pbESBuffer + iFirst + 5) == 0x00 
					&& *(m_pbESBuffer + iFirst + 6) == 0x00 
					&& *(m_pbESBuffer + iFirst + 7) == 0x00 
					&& *(m_pbESBuffer + iFirst + 8) == 0x01) )
				{
					// 找到一个ES头
					int nal_unit_type = ( *(m_pbESBuffer + iFirst + 4) & 0x1F );
					if ( nal_unit_type==7 || ((nal_unit_type==1) && (*(m_pbESBuffer + iFirst + 5) >= 0x80)))
					{
						break;
					}
				}
			}
			// 从头到尾都没有找到ES头
			if ( iFirst!=m_iESDataBegin && iFirst>(m_iESDataEnd - 9) )
			{
				// 获取数据长度
				int iESDataLen = m_iESDataEnd - m_iESDataBegin;

				// 视频数据直接封装成PES包
				int iPSBufSize = (m_iPSBufferSize - iOutPSDataLen);
				int iRet = es2ps((m_pbESBuffer + m_iESDataBegin), iESDataLen, 
					(m_pbPSBuffer + iOutPSDataLen), iPSBufSize, bStreamId, nInPTSTime);
				if (iRet==0)
				{
					iOutPSDataLen += iPSBufSize;
				}
				break;
			}
			// 移动缓存起始位置
			m_iESDataBegin = iFirst;
			// 未找到PS头
			if (iFirst>(m_iESDataEnd - 9))
			{
				break;
			}

			// 查找第二个ES头
			int iSecond = 0;
			for (iSecond = (m_iESDataBegin + 5); iSecond<=(m_iESDataEnd - 9); iSecond++)
			{
				if ( *(m_pbESBuffer + iSecond) == 0x00 
					&& *(m_pbESBuffer + iSecond + 1) == 0x00 
					&& *(m_pbESBuffer + iSecond + 2) == 0x00 
					&& *(m_pbESBuffer + iSecond + 3) == 0x01 
					&& 
					!(*(m_pbESBuffer + iSecond + 5) == 0x00 
					&& *(m_pbESBuffer + iSecond + 6) == 0x00 
					&& *(m_pbESBuffer + iSecond + 7) == 0x00 
					&& *(m_pbESBuffer + iSecond + 8) == 0x01))
				{
					// 找到一个ES头
					int nal_unit_type = ( *(m_pbESBuffer + iSecond + 4) & 0x1F );
					if ( nal_unit_type==7 || ((nal_unit_type==1) && (*(m_pbESBuffer + iSecond + 5) >= 0x80)))
					{
						break;
					}
				}
			}
			if (iSecond>(m_iESDataEnd - 9))
			{
				// 输入结束标志
				if (m_bInputEnd)
				{
					iSecond = m_iESDataEnd;
				}
				else
				{
					// 数据量不够
					break;
				}
			}
			int iESDataLen = iSecond - m_iESDataBegin;

			// 视频数据直接封装成PES包
			int iPSBufSize = (m_iPSBufferSize - iOutPSDataLen);
			int iRet = es2ps((m_pbESBuffer + m_iESDataBegin), iESDataLen, 
				(m_pbPSBuffer + iOutPSDataLen), iPSBufSize, bStreamId, nInPTSTime);
			m_iESDataBegin = iSecond;

			if (iRet==0)
			{
				iOutPSDataLen += iPSBufSize;
			}
		}
	}

	bpOutPSData = m_pbPSBuffer;
	iOutPSDataSize = iOutPSDataLen;

	return 0;
}

int CES2PS::ES2PS(byte* bpInESData, int iInESDataLen, byte* &bpOutPSData, int& iOutPSDataSize, bool bIsVideoStream, uint64_t nPTSTimeOffset, bool bIsRestart)
{
	uint64_t nInPTSTime = 0;

	if(bIsRestart)
	{
		m_ubiPrevPTSTime = 0;
		m_ubiPTSTimeOffset = nPTSTimeOffset;
	}


	if(bIsRestart)
	{
		nInPTSTime = 0;
	}
	else
	{
		nInPTSTime = m_ubiPrevPTSTime + m_ubiPTSTimeOffset;
	}
	
	bpOutPSData = NULL;
	iOutPSDataSize = 0;

	// 根据音视频区分stream_id
	byte bStreamId = (bIsVideoStream==true)?0xE0:0xC0;

	// 检查并分配PS缓存区
	if (IsPSBufferEmpty())
	{
		if (AllocatePSBuffer(MAX_UNIT_SIZE*2))
		{
			return -1;
		}
	}

	int iOutPSDataLen = 0;
	if (!bIsVideoStream)
	{
		iOutPSDataLen = m_iPSBufferSize;
		// 音频数据直接封装成PES包
		int iRet = es2ps(bpInESData, iInESDataLen, m_pbPSBuffer, iOutPSDataLen, bStreamId, nInPTSTime);
		m_ubiPrevPTSTime = nInPTSTime;
		
		if (iRet!=0)
		{
			return iRet;
		}
	}
	else
	{
		// 将数据存入ES缓冲区中
		InputESData(bpInESData, iInESDataLen, false);

		while (m_iESDataBegin<=(m_iESDataEnd - 9))
		{
			// 查找第一个ES头
			int iFirst = 0;
			for (iFirst = m_iESDataBegin; iFirst<=(m_iESDataEnd - 9); iFirst++)
			{
				if ( *(m_pbESBuffer + iFirst) == 0x00 
					&& *(m_pbESBuffer + iFirst + 1) == 0x00 
					&& *(m_pbESBuffer + iFirst + 2) == 0x00 
					&& *(m_pbESBuffer + iFirst + 3) == 0x01 
					&& 
					!(*(m_pbESBuffer + iFirst + 5) == 0x00 
					&& *(m_pbESBuffer + iFirst + 6) == 0x00 
					&& *(m_pbESBuffer + iFirst + 7) == 0x00 
					&& *(m_pbESBuffer + iFirst + 8) == 0x01) )
				{
					// 找到一个ES头
					int nal_unit_type = ( *(m_pbESBuffer + iFirst + 4) & 0x1F );
					if ( nal_unit_type==7 || ((nal_unit_type==1) && (*(m_pbESBuffer + iFirst + 5) >= 0x80)))
					{
						break;
					}
				}
			}
			// 从头到尾都没有找到ES头
			if ( iFirst!=m_iESDataBegin && iFirst>(m_iESDataEnd - 9) )
			{
				// 获取数据长度
				int iESDataLen = m_iESDataEnd - m_iESDataBegin;

				// 视频数据直接封装成PES包
				int iPSBufSize = (m_iPSBufferSize - iOutPSDataLen);
				int iRet = es2ps((m_pbESBuffer + m_iESDataBegin), iESDataLen, 
					(m_pbPSBuffer + iOutPSDataLen), iPSBufSize, bStreamId, nInPTSTime);

				
				m_ubiPrevPTSTime = nInPTSTime;

				if (iRet==0)
				{
					iOutPSDataLen += iPSBufSize;
				}
				break;
			}
			// 移动缓存起始位置
			m_iESDataBegin = iFirst;
			// 未找到PS头
			if (iFirst>(m_iESDataEnd - 9))
			{
				break;
			}

			// 查找第二个ES头
			int iSecond = 0;
			for (iSecond = (m_iESDataBegin + 5); iSecond<=(m_iESDataEnd - 9); iSecond++)
			{
				if ( *(m_pbESBuffer + iSecond) == 0x00 
					&& *(m_pbESBuffer + iSecond + 1) == 0x00 
					&& *(m_pbESBuffer + iSecond + 2) == 0x00 
					&& *(m_pbESBuffer + iSecond + 3) == 0x01 
					&& 
					!(*(m_pbESBuffer + iSecond + 5) == 0x00 
					&& *(m_pbESBuffer + iSecond + 6) == 0x00 
					&& *(m_pbESBuffer + iSecond + 7) == 0x00 
					&& *(m_pbESBuffer + iSecond + 8) == 0x01) )
				{
					// 找到一个ES头
					int nal_unit_type = ( *(m_pbESBuffer + iSecond + 4) & 0x1F );
					if ( nal_unit_type==7 || ((nal_unit_type==1) && (*(m_pbESBuffer + iSecond + 5) >= 0x80)))
					{
						break;
					}
				}
			}
			if (iSecond>(m_iESDataEnd - 9))
			{
				// 输入结束标志
				if (m_bInputEnd)
				{
					iSecond = m_iESDataEnd;
				}
				else
				{
					// 数据量不够
					break;
				}
			}
			int iESDataLen = iSecond - m_iESDataBegin;

			// 视频数据直接封装成PES包
			int iPSBufSize = (m_iPSBufferSize - iOutPSDataLen);
			int iRet = es2ps((m_pbESBuffer + m_iESDataBegin), iESDataLen, 
				(m_pbPSBuffer + iOutPSDataLen), iPSBufSize, bStreamId, nInPTSTime);
			m_ubiPrevPTSTime = nInPTSTime;

			nInPTSTime = m_ubiPrevPTSTime + m_ubiPTSTimeOffset;
			m_iESDataBegin = iSecond;

			if (iRet==0)
			{
				iOutPSDataLen += iPSBufSize;
			}
		}
	}

	bpOutPSData = m_pbPSBuffer;
	iOutPSDataSize = iOutPSDataLen;

	return 0;
}

// Convert an unit from es to ps.
int CES2PS::es2ps(byte* bpInESData, int iInESDataLen, byte* bpOutPSData, int& iOutPSDataSize, byte bInStreamId, uint64_t nInPTSTime)
{
	// 检查输入参数
	if (NULL==bpInESData || NULL==bpOutPSData || iInESDataLen<128)
	{
		return -1;
	}

	uint64_t nPTSTime = nInPTSTime*90;

	// 输出PS数据的长度
	int iOutPSDataLen = 0;

	// 视频
	if (bInStreamId==0xE0)
	{
		int nal_unit_type = (bpInESData[4] & 0x1F);

		if (nal_unit_type==1)
		{
			// 生成PS头
			byte ps_header[] = 
			{
				0x00, 0x00, 0x01, 0xBA, 0x44, 0x00, 0x04, 0x00, 0x04, 0x01, 0x00, 0x00, 0x03, 0xF8, 
				//0x00, 0x00, 0x01, 0xBA, 0x44, 0x0A, 0xB4, 0x15, 0x64, 0x01, 0x00, 0x3A, 0x9B, 0xF8, 
				//0x00, 0x00, 0x01, 0xBB, 0x00, 0x0C, 0x80, 0x1E, 0xFF, 0xFE, 0xE1, 0x7F, 0xE0, 0xE0, 0xE8, 0xC0, 0xC0, 0x20, 
				//0x00, 0x00, 0x01, 0xBC, 0x00, 0x18, 0xE1, 0xFF, 0x00, 0x00, 0x00, 0x08, 0x1B, 0xE0, 0x00, 0x06, 0x0A, 0x04, 0x65, 0x6E, 0x67, 0x00, 0x90, 0xC0, 0x00, 0x00, 0x3E, 0xB9, 0x0F, 0x3D
			};
			int ps_header_len = sizeof(ps_header);

			ps_header[4] |= (nPTSTime>>27 & 0x38);
			ps_header[4] |= (nPTSTime>>28 & 0x03);
			ps_header[5] |= (nPTSTime>>20);
			ps_header[6] |= (nPTSTime>>12 & 0xF8);
			ps_header[6] |= (nPTSTime>>13 & 0x03);
			ps_header[7] |= (nPTSTime>>5);
			ps_header[8] |= (nPTSTime<<3 & 0xF8);

			int mutex = 3600;
			if(m_ubiPTSTimeOffset != 0)
			{
				mutex = (m_ubiPTSTimeOffset*90);
			}

			mutex = (mutex & 0x003fffff);
			ps_header[10] = (unsigned char)(mutex >> 14);
			ps_header[11] = (unsigned char)((mutex >> 6) & 0x000000ff);
			unsigned char tmpchr = (unsigned char)(mutex & 0x0000003f);
			ps_header[12] |= (tmpchr << 2);
			// 复制PS头到输出缓存中
			if ( (iOutPSDataSize - iOutPSDataLen)<ps_header_len )
			{
				return -1;
			}
			memcpy( (bpOutPSData + iOutPSDataLen), ps_header, ps_header_len );
			iOutPSDataLen += ps_header_len;
		}
		else if ( nal_unit_type == 7)
		{

			// 生成PS头
			byte ps_header[] = 
			{

					0x00, 0x00, 0x01, 0xBA,    0x44, 0x00, 0x04, 0x00, 
					0x04, 0x01, 0x00, 0x00,    0x03, 0xF8, 

					0x00, 0x00, 0x01, 0xBB,     0x00, 0x09, 0x80, 0x00,
	                0x01, 0x08, 0xE1, 0x7F,     0xE0, 0xE0, 0x80,/*buffer size*/
	           
					
	              0x00, 0x00, 0x01, 0xBC,    0x00, 0x0E, 0xE0/*increase by times*/, 0xFF, 
	              
	                       0x00, 0x00, 
	                              
								0x00, 0x04,
								0x1B, 0xE0, 0x00, 0x00,

								0x00, 0x00, 0x00, 0x00,
			};

			int ps_header_len = sizeof(ps_header);

			ps_header[4] |= (nPTSTime>>27 & 0x38);
			ps_header[4] |= (nPTSTime>>28 & 0x03);
			ps_header[5] |= (nPTSTime>>20);
			ps_header[6] |= (nPTSTime>>12 & 0xF8);
			ps_header[6] |= (nPTSTime>>13 & 0x03);
			ps_header[7] |= (nPTSTime>>5);
			ps_header[8] |= (nPTSTime<<3 & 0xF8);

			int mutex = (mutex & 0x003fffff);
			ps_header[10] = (unsigned char)(mutex >> 14);
			ps_header[11] = (unsigned char)((mutex >> 6) & 0x000000ff);
			unsigned char tmpchr = (unsigned char)(mutex & 0x0000003f);
			ps_header[12] |= (tmpchr << 2);

			/*static byte counter = 0;
		   ps_header[44] = (0xe0 + counter);

			counter++;
			counter = counter % 32;
			for(int i=0;i < 8; i++)
			{
				printf("-------------------->%02X ", bpOutPSData[iOutPSDataLen+i+38]);
			}
			printf("\n");
			*/

			
			// 复制PS头到输出缓存中
			if ( (iOutPSDataSize - iOutPSDataLen)<ps_header_len )
			{
				return -1;
			}
			memcpy( (bpOutPSData + iOutPSDataLen), ps_header, ps_header_len );						
			iOutPSDataLen += ps_header_len;
		}
	}

	// 用于生成PES头
	byte pes_header[MAX_PES_HEADER_SIZE];
	int  pes_header_len = 0;

	// 将整个ES包拆成若干个小的PES包
	int iCurPESDataStart = 0;
	int iCurPESDataLen = 0;
	int fgHasPackedPSPAndPPS = -2;
	while(iCurPESDataStart<iInESDataLen)
	{
		if(fgHasPackedPSPAndPPS < 0 && (bpInESData[4] & 0x1F) == 7)
		{
			if(fgHasPackedPSPAndPPS < -1)
			{
				int spsUnitLen = -1;
				int k=iCurPESDataStart;
				while ( (iInESDataLen - k) > 4 )
				{
					if(bpInESData[k] == 0x00 && bpInESData[k+1] == 0x00 && 
						(bpInESData[k+2] == 0x01 || (bpInESData[k+2] == 0x00 && bpInESData[k+3] == 0x01)))
					{
						   if(k > ( iCurPESDataStart + 3 ))
						   {
							   spsUnitLen = (k - iCurPESDataStart);
							   break;	
						   }
					}
					
					k++;
				}

				if(spsUnitLen == -1)//retrive sps data false
				{
					fgHasPackedPSPAndPPS = 1;
					iCurPESDataLen = (iInESDataLen - iCurPESDataStart)<MAX_PES_DATA_SIZE?(iInESDataLen - iCurPESDataStart):MAX_PES_DATA_SIZE;
				}
				else
				{
					iCurPESDataLen = spsUnitLen;
					if(((iInESDataLen - iCurPESDataStart - iCurPESDataLen) > 5) && ((bpInESData[iCurPESDataStart+iCurPESDataLen+4] & 0x1F) == 7))
					{
						//has duplicated sps header
					}
					else
					{
						fgHasPackedPSPAndPPS++;
					}
				}
			}
			else
			{
				int ppsUnitLen = -1;
				int k=iCurPESDataStart;
				do
				{
					if((iInESDataLen - iCurPESDataStart) > 5)
					{
							bool cond1 = (bpInESData[iCurPESDataStart] == 0x00 && bpInESData[iCurPESDataStart+1] == 0x00 && 
							(bpInESData[iCurPESDataStart+2] == 0x01 || (bpInESData[iCurPESDataStart+2] == 0x00 && bpInESData[iCurPESDataStart+3] == 0x01)));
							bool cond2 = ((bpInESData[iCurPESDataStart+4] & 0x1F) == 8);

							if(cond1 && cond2)
							{
							}
							else
							{
								break;
							}
					}
					else
					{
						break;
					}
					
					while ( (iInESDataLen - k) > 4 )
					{
						if(bpInESData[k] == 0x00 && bpInESData[k+1] == 0x00 && 
							(bpInESData[k+2] == 0x01 || (bpInESData[k+2] == 0x00 && bpInESData[k+3] == 0x01)))
						{
							if(k > (iCurPESDataStart+3))
							{
							   ppsUnitLen = (k - iCurPESDataStart);
							   break;	
							}
						}
						
						k++;
					}
				}while(false);

				if(ppsUnitLen == -1)//retrive psp data false
				{
					fgHasPackedPSPAndPPS = 1;
					iCurPESDataLen = (iInESDataLen - iCurPESDataStart)<MAX_PES_DATA_SIZE?(iInESDataLen - iCurPESDataStart):MAX_PES_DATA_SIZE;
				}
				else
				{
					iCurPESDataLen = ppsUnitLen;
					if(((iInESDataLen - iCurPESDataStart - iCurPESDataLen) > 5) && ((bpInESData[iCurPESDataStart+iCurPESDataLen+4] & 0x1F) == 8))
					{
						//has duplicated sps header
					}
					else
					{
						fgHasPackedPSPAndPPS++;
					}
				}
			}
		}
		else
		{
			// 计算每个PES包的数据长度
				iCurPESDataLen = (iInESDataLen - iCurPESDataStart)<MAX_PES_DATA_SIZE?(iInESDataLen - iCurPESDataStart):MAX_PES_DATA_SIZE;
		}

		
		iCurPESDataLen = (iInESDataLen - iCurPESDataStart)<MAX_PES_DATA_SIZE?(iInESDataLen - iCurPESDataStart):MAX_PES_DATA_SIZE;
	
		// 生成PES头
		pes_header_len = 0;
		PES_header(iCurPESDataLen, bInStreamId, (nPTSTime!=0), nPTSTime, false, 0, pes_header, &pes_header_len);
		// PTS时间只需要放到第一个PES包
		nPTSTime = 0;

		// 判断输出缓存是否有足够的空间
		if ( (iOutPSDataSize - iOutPSDataLen)<(pes_header_len + iCurPESDataLen) )
		{
			return -1;
		}
		// 复制数据到PES包中
		memcpy( (bpOutPSData + iOutPSDataLen), pes_header, pes_header_len);
		iOutPSDataLen += pes_header_len;
		memcpy( (bpOutPSData + iOutPSDataLen), (bpInESData + iCurPESDataStart), iCurPESDataLen);
		iOutPSDataLen += iCurPESDataLen;

		// 计算下一个PES数据的起始位置
		iCurPESDataStart += iCurPESDataLen;
	}

	iOutPSDataSize = iOutPSDataLen;

	return 0;
}

// 传入ES数据,数据包长度需小于MAX_UNIT_SIZE
// bInputEnd：输入结束标志，避免最后一个PS包数据的丢失
int CES2PS::InputESData(byte* pbESData, int iESDataLen, bool bInputEnd)
{
	m_bInputEnd = bInputEnd;

	// 检查并分配缓存区
	if (IsESBufferEmpty())
	{
		AllocateESBuffer(4*MAX_UNIT_SIZE);
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

// 将es文件转换成ps文件
int CES2PS::ES2PS_2(char* pInESFilePath, char* pOutPSFilePath)
{
	// 分配输出缓存区
	int32_t iPSUnitBufSize = MAX_UNIT_SIZE*2;
	byte *pbPSUnitBuf = new byte[iPSUnitBufSize];
	if (NULL==pbPSUnitBuf)
	{
		return -1;
	}

	// 分配输入缓存区
	int32_t iESUnitBufSize = MAX_UNIT_SIZE;
	byte *pbESUnitBuf = new byte[iESUnitBufSize];
	if (NULL==pbESUnitBuf)
	{
		// 释放输出缓存区
		RELEASE_OBJECTS(pbPSUnitBuf);
		return -1;
	}

	// 打开写入的ES文件
	FILE *input = fopen(pInESFilePath, "rb");
	if (NULL==input)
	{
		// 释放输出缓存区
		RELEASE_OBJECTS(pbPSUnitBuf);
		// 释放输入缓存区
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	// 打开写入的PS文件
	FILE *output = fopen(pOutPSFilePath, "wb");
	if (NULL==output)
	{
		// 关闭输入文件
		if (NULL!=input)
		{
			fclose(input);
			input = NULL;
		}
		// 释放输出缓存区
		RELEASE_OBJECTS(pbPSUnitBuf);
		// 释放输入缓存区
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	while(!feof(input))
	{
		int iReadLen = fread(pbESUnitBuf, 1, iESUnitBufSize, input);
		if ( ferror(input) )
		{
			break;
		}
		int iOutPSDataLen = iPSUnitBufSize;
		if (0==es2ps(pbESUnitBuf, iReadLen, pbPSUnitBuf, iOutPSDataLen, 0xE0, 0))
		{
			if (NULL!=output && iOutPSDataLen>0 && iOutPSDataLen<iPSUnitBufSize)
			{
				fwrite(pbPSUnitBuf, 1, iOutPSDataLen, output);
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
	// 释放输出缓存区
	RELEASE_OBJECTS(pbPSUnitBuf);
	// 释放输入缓存区
	RELEASE_OBJECTS(pbESUnitBuf);

	return 0;
}

// 将es文件转换成ps文件(按固定长度转换)
int CES2PS::ES2PS_3(char* pInESFilePath, char* pOutPSFilePath)
{
	// 分配输入缓存区
	int32_t iESUnitBufSize = 10*1024;
	byte *pbESUnitBuf = new byte[iESUnitBufSize];
	if (NULL==pbESUnitBuf)
	{
		return -1;
	}

	// 打开写入的ES文件
	FILE *input = fopen(pInESFilePath, "rb");
	if (NULL==input)
	{
		// 释放输入缓存区
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	// 打开写入的PS文件
	FILE *output = fopen(pOutPSFilePath, "wb");
	if (NULL==output)
	{
		// 关闭输入文件
		if (NULL!=input)
		{
			fclose(input);
			input = NULL;
		}
		// 释放输入缓存区
		RELEASE_OBJECTS(pbESUnitBuf);
		return -1;
	}

	time_t tNow = time(NULL);
	int nMiliSeconds = 0;

	while(!feof(input))
	{
		int iReadLen = fread(pbESUnitBuf, 1, iESUnitBufSize, input);
		if ( ferror(input) )
		{
			break;
		}
		byte *pbPSUnitBuf = NULL;
		int iOutPSDataLen = 0;

		nMiliSeconds += 40;
		tNow += nMiliSeconds/1000;
		nMiliSeconds %= 1000;
		tm *tmNow = localtime(&tNow);
		uint64_t nInPTSTime = ((tmNow->tm_hour * 60 + tmNow->tm_min)*60 + tmNow->tm_sec)*1000 + nMiliSeconds;

		if (0==ES2PS(pbESUnitBuf, iReadLen, pbPSUnitBuf, iOutPSDataLen, true, nInPTSTime))
		{
			if (NULL!=output && iOutPSDataLen>0 )
			{
				fwrite(pbPSUnitBuf, 1, iOutPSDataLen, output);
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
	// 释放输入缓存区
	RELEASE_OBJECTS(pbESUnitBuf);

	return 0;
}

/*
 * Create a PES header for our data.
 *
 * - `data_len` is the length of our ES data
 *   If this is too long to fit into 16 bits, then we will create a header
 *   with 0 as its length. Note this is only allowed (by the standard) for
 *   video data.
 * - `stream_id` is the elementary stream id to use (see H.222 Table 2-18).
 *   If the stream id indicates an audio stream (as elucidated by Note 2 in
 *   that same table), then the data_alignment_indicator flag will be set
 *   in the PES header - i.e., we assume that the audio frame *starts*
 *   (has its syncword) at the start of the PES packet payload.
 * - `with_PTS` should be TRUE if the PTS value in `pts` should be written
 *   to the PES header.
 * - `with_DTS` should be TRUE if the DTS value in `dts` should be written
 *   to the PES header. Note that if `with_DTS` is TRUE, then `with_PTS`
 *   must also be TRUE. If it is not, then the DTS value will be used for
 *   the PTS.
 * - `PES_hdr` is the resultant PES packet header, and
 * - `PES_hdr_len` its length (at the moment that's always the same, as
 *   we're not yet outputting any timing information (PTS/DTS), and so
 *   can get away with a minimal PES header).
 */
void CES2PS::PES_header(
	uint32_t  data_len,
	byte      stream_id,
	int       with_PTS,
	uint64_t  pts,
	int       with_DTS,
	uint64_t  dts,
	byte     *PES_hdr,
	int      *PES_hdr_len)
{
	int  extra_len = 0;

	if (with_DTS && !with_PTS)
	{
		with_PTS = true;
		pts = dts;
	}

	// If PTS=DTS then there is no point explictly coding the DTS so junk it
	if (with_DTS && pts == dts)
		with_DTS = false;

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
		PES_hdr[6] = 0x84;     // just data alignment indicator flag set
	else
		PES_hdr[6] = 0x80;     // no flags set

	// Flags: PTS_DTS_flags .. PES_extension_flag
	if (with_DTS && with_PTS)
		PES_hdr[7] = 0xC0;
	else if (with_PTS)
		PES_hdr[7] = 0x80;
	else
		PES_hdr[7] = 0x00;     // yet more unset flags (nb: no PTS/DTS info)

	// PES_header_data_length
	if (with_DTS && with_PTS)
	{
		PES_hdr[8] = 0x0A;
		encode_pts_dts(&(PES_hdr[9]),3,pts);
		encode_pts_dts(&(PES_hdr[14]),1,dts);
		*PES_hdr_len = 9 + 10;
		extra_len = 3 + 10; // 3 bytes after the length field, plus our PTS & DTS
		PES_hdr[6] |= 0x0c;
	}
	else if (with_PTS)
	{
		PES_hdr[8] = 0x05;
		encode_pts_dts(&(PES_hdr[9]),2,pts);
		*PES_hdr_len = 9 + 5;
		extra_len = 3 + 5; // 3 bytes after the length field, plus our PTS
		PES_hdr[6] |= 0x0c;
	}
	else
	{
		PES_hdr[8] = 0x00; // 0 means there is no more data
		*PES_hdr_len = 9;
		extra_len = 3; // just the basic 3 bytes after the length field
		PES_hdr[6] |= 0x08;
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

/*
 * Encode a PTS or DTS.
 *
 * - `data` is the array of 5 bytes into which to encode the PTS/DTS
 * - `guard_bits` are the required guard bits: 2 for a PTS alone, 3 for
 *   a PTS before a DTS, or 1 for a DTS after a PTS
 * - `value` is the PTS or DTS value to be encoded
 */
void CES2PS::encode_pts_dts(
	byte    data[],
	int     guard_bits,
	uint64_t value)
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
