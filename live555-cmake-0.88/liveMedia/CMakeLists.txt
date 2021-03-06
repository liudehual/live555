project(LiveMedia)
include_directories(include)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
     AC3AudioFileServerMediaSubsession.cpp
AC3AudioRTPSink.cpp
AC3AudioRTPSource.cpp
AC3AudioStreamFramer.cpp
ADTSAudioFileServerMediaSubsession.cpp
ADTSAudioFileSource.cpp
AMRAudioFileServerMediaSubsession.cpp
AMRAudioFileSink.cpp
AMRAudioFileSource.cpp
AMRAudioRTPSink.cpp
AMRAudioRTPSource.cpp
AMRAudioSource.cpp
AudioInputDevice.cpp
AudioRTPSink.cpp
AVIFileSink.cpp
Base64.cpp
BasicUDPSink.cpp
BasicUDPSource.cpp
BitVector.cpp
ByteStreamFileSource.cpp
ByteStreamMemoryBufferSource.cpp
ByteStreamMultiFileSource.cpp
DeviceSource.cpp
DigestAuthentication.cpp
DVVideoFileServerMediaSubsession.cpp
DVVideoRTPSink.cpp
DVVideoRTPSource.cpp
DVVideoStreamFramer.cpp
EBMLNumber.cpp
FileServerMediaSubsession.cpp
FileSink.cpp
FramedFileSource.cpp
FramedFilter.cpp
FramedSource.cpp
GenericMediaServer.cpp
GSMAudioRTPSink.cpp
H261VideoRTPSource.cpp
H263plusVideoFileServerMediaSubsession.cpp
H263plusVideoRTPSink.cpp
H263plusVideoRTPSource.cpp
H263plusVideoStreamFramer.cpp
H263plusVideoStreamParser.cpp
H264or5VideoFileSink.cpp
H264or5VideoRTPSink.cpp
H264or5VideoStreamDiscreteFramer.cpp
H264or5VideoStreamFramer.cpp
H264VideoFileServerMediaSubsession.cpp
H264VideoFileSink.cpp
H264VideoRTPSink.cpp
H264VideoRTPSource.cpp
H264VideoStreamDiscreteFramer.cpp
H264VideoStreamFramer.cpp
H265VideoFileServerMediaSubsession.cpp
H265VideoFileSink.cpp
H265VideoRTPSink.cpp
H265VideoRTPSource.cpp
H265VideoStreamDiscreteFramer.cpp
H265VideoStreamFramer.cpp
InputFile.cpp
JPEGVideoRTPSink.cpp
JPEGVideoRTPSource.cpp
JPEGVideoSource.cpp
Locale.cpp
MatroskaDemuxedTrack.cpp
MatroskaFile.cpp
MatroskaFileParser.cpp
MatroskaFileServerDemux.cpp
MatroskaFileServerMediaSubsession.cpp
Media.cpp
MediaSession.cpp
MediaSink.cpp
MediaSource.cpp
MP3ADU.cpp
MP3ADUdescriptor.cpp
MP3ADUinterleaving.cpp
MP3ADURTPSink.cpp
MP3ADURTPSource.cpp
MP3ADUTranscoder.cpp
MP3AudioFileServerMediaSubsession.cpp
MP3AudioMatroskaFileServerMediaSubsession.cpp
MP3FileSource.cpp
MP3Internals.cpp
MP3InternalsHuffman.cpp
MP3InternalsHuffmanTable.cpp
MP3StreamState.cpp
MP3Transcoder.cpp
MPEG1or2AudioRTPSink.cpp
MPEG1or2AudioRTPSource.cpp
MPEG1or2AudioStreamFramer.cpp
MPEG1or2Demux.cpp
MPEG1or2DemuxedElementaryStream.cpp
MPEG1or2DemuxedServerMediaSubsession.cpp
MPEG1or2FileServerDemux.cpp
MPEG1or2VideoFileServerMediaSubsession.cpp
MPEG1or2VideoRTPSink.cpp
MPEG1or2VideoRTPSource.cpp
MPEG1or2VideoStreamDiscreteFramer.cpp
MPEG1or2VideoStreamFramer.cpp
MPEG2IndexFromTransportStream.cpp
MPEG2TransportFileServerMediaSubsession.cpp
MPEG2TransportStreamFramer.cpp
MPEG2TransportStreamFromESSource.cpp
MPEG2TransportStreamFromPESSource.cpp
MPEG2TransportStreamIndexFile.cpp
MPEG2TransportStreamMultiplexor.cpp
MPEG2TransportStreamTrickModeFilter.cpp
MPEG2TransportUDPServerMediaSubsession.cpp
MPEG4ESVideoRTPSink.cpp
MPEG4ESVideoRTPSource.cpp
MPEG4GenericRTPSink.cpp
MPEG4GenericRTPSource.cpp
MPEG4LATMAudioRTPSink.cpp
MPEG4LATMAudioRTPSource.cpp
MPEG4VideoFileServerMediaSubsession.cpp
MPEG4VideoStreamDiscreteFramer.cpp
MPEG4VideoStreamFramer.cpp
MPEGVideoStreamFramer.cpp
MPEGVideoStreamParser.cpp
MultiFramedRTPSink.cpp
MultiFramedRTPSource.cpp
OggDemuxedTrack.cpp
OggFile.cpp
OggFileParser.cpp
OggFileServerDemux.cpp
OggFileServerMediaSubsession.cpp
OggFileSink.cpp
OnDemandServerMediaSubsession.cpp
ourMD5.cpp
OutputFile.cpp
PassiveServerMediaSubsession.cpp
ProxyServerMediaSession.cpp
QCELPAudioRTPSource.cpp
QuickTimeFileSink.cpp
QuickTimeGenericRTPSource.cpp
RTCP.cpp
RTPInterface.cpp
RTPSink.cpp
RTPSource.cpp
RTSPClient.cpp
RTSPCommon.cpp
RTSPRegisterSender.cpp
RTSPServer.cpp
RTSPServerSupportingHTTPStreaming.cpp
ServerMediaSession.cpp
SimpleRTPSink.cpp
SimpleRTPSource.cpp
SIPClient.cpp
StreamParser.cpp
StreamReplicator.cpp
T140TextRTPSink.cpp
TCPStreamSink.cpp
TextRTPSink.cpp
TheoraVideoRTPSink.cpp
TheoraVideoRTPSource.cpp
uLawAudioFilter.cpp
VideoRTPSink.cpp
VorbisAudioRTPSink.cpp
VorbisAudioRTPSource.cpp
VP8VideoRTPSink.cpp
VP8VideoRTPSource.cpp
VP9VideoRTPSink.cpp
VP9VideoRTPSource.cpp
WAVAudioFileServerMediaSubsession.cpp
WAVAudioFileSource.cpp
H264LiveMediaServerFrameSource.cpp
H264LiveMediaServerSubSession.cpp
rtcp_from_spec.c
)
add_library(LiveMedia STATIC ${SRC_LIST})
