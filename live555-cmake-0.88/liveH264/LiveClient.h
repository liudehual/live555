#ifndef _LIVE_CLIENT_H__
#define _LIVE_CLIENT_H__

typedef enum{
	JVLIVE_ERR_OK,
	JVLIVE_ERR_OVER,//�������
	JVLIVE_ERR_UNKNOWN,
	JVLIVE_ERR_TIMEOUT,
	JVLIVE_ERR_BAD_REQUEST,
	JVLIVE_ERR_WRONG_PASSWD,
	JVLIVE_ERR_NOT_FOUND,

}jvlive_err_e;
typedef struct
{
	char m_streamID[32];
	int m_LinkID;
	char acManufacture[32];

}m_streamID_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ���Ž������������ض˽����˲���
 */
typedef void (afterPlayingFunc_t)(void* param, jvlive_err_e errcode, const char *errStr);

/**
 * ���ݻص�
 *
 *@param param �û�����
 *@param channelHandle ͨ������������ڻ�ȡͨ����ص���Ϣ����Ҫ���ǵ����п����ж�·��Ƶ������Ƶ���Լ����ǵ�����
 *@param codecName ���뷽ʽ��H264, PCMU, PCMA��
 *@param buffer ֡λ��
 *@param frameSize ֡����
 *@param presentationTime ��ǰ֡ʱ��
 *@param durationInMicroseconds ��δʹ��
 */
typedef void (afterGettingFrame_t)(void* param,
								void *channelHandle,
								unsigned char *buffer,
								unsigned frameSize,
								struct timeval presentationTime,
								unsigned durationInMicroseconds);

/**
 * ��һ·����
 * ע�⣺�����������أ���������ֱ������jvlive_client_stop
 *   �����;���أ���һ���ǳ�����
 *
 *@param streamID �����ƣ�Ψһ��ʶ�򿪵���
 *@param url Ҫ�򿪵��������ĵ�ַ�����ƣ�rtsp://192.168.11.192:8554/live0.264
 *@param maxVideoStreamSize ��Ƶ���֡��֡��С
 *@param user �û���������ΪNULL
 *@param passwd ���룬����ΪNULL
 *@param afterPlaying ���ؽ�������������ԭ����ֹͣʱ�Ļص�
 *@param afterGetting �յ����ݺ�Ļص�����
 *@param param �û����ݣ�����afterPlaying��afterGettingʱ����
 */
int jvlive_client_run(
		const m_streamID_t *streamID,
		const char *url,
		int maxVideoStreamSize,
		const char *user,
		const char *passwd,
		afterPlayingFunc_t *afterPlaying,
		afterGettingFrame_t *afterGetting,
		void *param);

/**
 * �ر�֮ǰ�򿪵�����
 */
int jvlive_client_stop(const m_streamID_t *streamID);

typedef struct {
	char mediaName[32];	//video  audio
	char codecName[32]; //H264 PCMA PCMU ...
	char protocolName[32]; //RTP
} jvliveChannelInfo_t;

/**
 * ��ȡRTSP���ŵ�ĳ�� ��������Ϣ
 *
 *@param channelHandle #afterGettingFrame_t �Ĳ���֮һ
 *@param info ��ȡ������Ϣ
 */
int jvlive_client_get_channel_info(void *channelHandle, jvliveChannelInfo_t *info);

int GetParameter_session(
		const m_streamID_t *streamID,
		char const* parameterName,
		const char *user,
		const char *passwd);

int SetParameter_session(
		const m_streamID_t *streamID,
		char const* parameterValue,
		const char *user,
		const char *passwd) ;

char *GetParameter_String();





#ifdef __cplusplus
}
#endif



#endif
