#ifndef __ES_2_PS_H__
#define __ES_2_PS_H__

#include <stdio.h>

#include "comdef.h"

// ES vs PS
class CES2PS
{
public:
	CES2PS(void);
	~CES2PS(void);

public:
	// Convert an unit from es to ps.
	//	bOutPSData[In/Out]:		PS缓冲区，无需调用者释放
	//	iOutPSDataSize[In/Out]: [IN]bOutPSData的分配长度，[OUT]输出转换后的长度
	//  bIsVideoStream[In]:		true-视频数据，false-音频数据
	//  nInPTSTime[In]:			PTS时间(单位为ms)，传入0时将以当前系统时间作为PTS时间
	int ES2PS(byte* bpInESData, int iInESDataLen, byte* &bpOutPSData, int& iOutPSDataSize, bool bIsVideoStream, uint64_t nInPTSTime = 0);
	int ES2PS(byte* bpInESData, int iInESDataLen, byte* &bpOutPSData, int& iOutPSDataSize, bool bIsVideoStream, uint64_t nPTSTimeOffset, bool bIsRestart);

private:
	int AllocatePSBuffer(int nSize)
	{
		m_pbPSBuffer = new byte[nSize];
		if (NULL==m_pbPSBuffer)
		{
			return -1;
		}
		m_iPSBufferSize = nSize;
		return 0;
	}
	void ReleasePSBuffer()
	{
		if (NULL!=m_pbPSBuffer)
		{
			delete[] m_pbPSBuffer;
			m_pbPSBuffer = NULL;
		}
		m_iPSBufferSize = 0;
	}
	bool IsPSBufferEmpty()
	{
		return (m_pbPSBuffer==NULL)?true:false;
	}
	byte* m_pbPSBuffer;
	int m_iPSBufferSize;

private:
	int AllocateESBuffer(int nSize)
	{
		m_pbESBuffer = new byte[nSize];
		if (NULL==m_pbESBuffer)
		{
			return -1;
		}
		m_iESBufferSize = nSize;
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
	}
	bool IsESBufferEmpty()
	{
		return (m_pbESBuffer==NULL)?true:false;
	}
	byte* m_pbESBuffer;
	int m_iESBufferSize;

	int m_iESDataBegin;
	int m_iESDataEnd;
	uint64_t m_ubiPrevPTSTime;
	uint64_t m_ubiPTSTimeOffset;
private:
	// Convert an unit from es to ps.
	//	bOutPSData[in/Out]:		PS缓冲区，由调用分配
	//	iOutPSDataSize[In/Out]: [IN]bOutPSData的分配长度，[OUT]输出转换后的长度
	//  nInPTSTime[in]:			PTS时间戳，以ms为单位
	int es2ps(byte* bpInESData, int iInESDataLen, byte* bpOutPSData, int& iOutPSDataSize, byte bInStreamId, uint64_t nInPTSTime);

	// 传入ES数据,数据包长度需小于MAX_UNIT_SIZE
	// bInputEnd：输入结束标志，避免最后一个PS包数据的丢失
	int InputESData(byte* pbESData, int iESDataLen, bool bInputEnd);
	// 输入结束标志，避免避免最后一个ES包数据的丢失
	bool m_bInputEnd;

// for test
public:
	// 将es文件转换成ps文件(按固定长度转换)
	int ES2PS_2(char* pInESFilePath, char* pOutPSFilePath);

	// 将es文件转换成ps文件(按固定长度转换)
	int ES2PS_3(char* pInESFilePath, char* pOutPSFilePath);

private:
	// Create a PES header for our data.
	void PES_header(uint32_t  data_len,
		byte      stream_id,
		int       with_PTS,
		uint64_t  pts,
		int       with_DTS,
		uint64_t  dts,
		byte     *PES_hdr,
		int      *PES_hdr_len);
	// Encode a PTS or DTS.
	void encode_pts_dts(
		byte    data[],
		int     guard_bits,
		uint64_t value);
};
#endif
