#include "SimpleRTPSink.hh"

#include "AudioServerMediaSubsession.h"

#include "FramedAudioSource.h"

AudioServerMediaSubsession* AudioServerMediaSubsession::createNew(
		UsageEnvironment& env, char const* audioType, Boolean reuseFirstSource,
		int bitrateKbps , int timeStampFrequency) {
	AudioServerMediaSubsession* pSMS = new AudioServerMediaSubsession(env,
			audioType, reuseFirstSource, bitrateKbps, timeStampFrequency);

	return pSMS;
}

AudioServerMediaSubsession::AudioServerMediaSubsession(UsageEnvironment& env,
		char const* audioType, Boolean reuseFirstSource, int bitrateKbps ,
		int timeStampFrequency ) :
		OnDemandServerMediaSubsession(env, reuseFirstSource),
		m_fAudioReveiver(NULL),
		m_bitrateKbps(bitrateKbps),
		m_timeStampFrequency(timeStampFrequency) {
	strcpy(m_audioType, audioType);
}

AudioServerMediaSubsession::~AudioServerMediaSubsession() {
	if (m_fAudioReveiver) {
		delete m_fAudioReveiver;
		m_fAudioReveiver = NULL;
	}
}

FramedSource* AudioServerMediaSubsession::createNewStreamSource(
		unsigned clientSessionId, unsigned& estBitrate) {

	estBitrate = m_bitrateKbps; // kbps, estimate
	if (!m_fAudioReveiver)
		m_fAudioReveiver = new FramesManager(10*1024);

	FramedAudioSource* pAudioSource = FramedAudioSource::createNew(envir(),
			m_fAudioReveiver);

	if (NULL == pAudioSource) {
		return NULL;
	}
	return pAudioSource;
}

RTPSink* AudioServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
		unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
/*

0       PCMU       A              8000       1     [RFC3551]
1       Reserved
2       Reserved
3          GSM            A              8000       1     [RFC3551]
4       G723          A              8000       1     [Kumar]
5       DVI4       A              8000       1     [RFC3551]
6       DVI4       A              16000       1     [RFC3551]
7       LPC           A              8000       1     [RFC3551]
8       PCMA          A              8000       1     [RFC3551]
9       G722       A              8000       1     [RFC3551]
10       L16           A              44100       2     [RFC3551]
11       L16           A              44100       1     [RFC3551]
12       QCELP        A              8000       1
13       CN          A              8000       1     [RFC3389]
14       MPA           A              90000     [RFC3551][RFC2250]
15       G728       A              8000       1     [RFC3551]
16       DVI4       A              11025       1     [DiPol]
17       DVI4       A              22050       1     [DiPol]
18       G729       A              8000       1
19       reserved    A
20       unassigned A
21       unassigned A
22       unassigned A
23       unassigned A
24       unassigned V
25       CelB       V              90000             [RFC2029]
26       JPEG       V              90000             [RFC2435]
27       unassigned V
28       nv             V              90000             [RFC3551]
29       unassigned V
30       unassigned V
31       H261       V              90000             [RFC2032]
32       MPV           V              90000             [RFC2250]
33       MP2T       AV          90000             [RFC2250]
34       H263          V              90000             [Zhu]
35--71     unassigned ?
72--76     reserved for RTCP conflict avoidance          [RFC3550]
77--95     unassigned ?
96--127 dynamic    ?                               [RFC3551]
 */
	return SimpleRTPSink::createNew(envir(), rtpGroupsock, 8,
			m_timeStampFrequency, "audio", m_audioType, 1, False, False);
}

int AudioServerMediaSubsession::WriteFrame(char *data, int len)
{
	if (m_fAudioReveiver)
		return m_fAudioReveiver->WriteFrame((unsigned char *)data, len);
	else
		return -1;
}

