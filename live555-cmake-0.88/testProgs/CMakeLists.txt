project(openRTSP)
include_directories(.)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)

set(openRTSP_LIST
	playCommon.cpp
	openRTSP.cpp
)
add_executable(openRTSP ${openRTSP_LIST})
TARGET_LINK_LIBRARIES(openRTSP
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )




set(H264VideStreamer_LIST
	testH264VideoStreamer.cpp
)
add_executable(H264VideoStreamer ${H264VideStreamer_LIST})
TARGET_LINK_LIBRARIES(H264VideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(rtspClient_LIST
	testRTSPClient.cpp
)
add_executable(testRtspClient ${rtspClient_LIST})
TARGET_LINK_LIBRARIES(testRtspClient
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(MPEG2TransportStreamIndexer_LIST
	MPEG2TransportStreamIndexer.cpp
)
add_executable(MPEG2TransportStreamIndexer ${MPEG2TransportStreamIndexer_LIST})
TARGET_LINK_LIBRARIES(MPEG2TransportStreamIndexer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(registerRTSPStream_LIST
	registerRTSPStream.cpp
)
add_executable(registerRTSPStream ${registerRTSPStream_LIST})
TARGET_LINK_LIBRARIES(registerRTSPStream
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(sapWatch_LIST
	sapWatch.cpp
)
add_executable(sapWatch ${sapWatch_LIST})
TARGET_LINK_LIBRARIES(sapWatch
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testAMRAudioStreamer_LIST
	testAMRAudioStreamer.cpp
)
add_executable(testAMRAudioStreamer ${testAMRAudioStreamer_LIST})
TARGET_LINK_LIBRARIES(testAMRAudioStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testDVVideoStreamer_LIST
	testDVVideoStreamer.cpp
)
add_executable(testDVVideoStreamer ${testDVVideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testDVVideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

# ≤ª±‡“Î testGSMStreamer
if(0)
set(testGSMStreamer_LIST
	testGSMStreamer.cpp
)
add_executable(testGSMStreamer ${testGSMStreamer_LIST})
TARGET_LINK_LIBRARIES(testGSMStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
endif()    
 set(testH264VideoStreamer_LIST
	testH264VideoStreamer.cpp
)
add_executable(testH264VideoStreamer ${testH264VideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testH264VideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
 set(testH264VideoToTransportStream_LIST
	testH264VideoToTransportStream.cpp
)
add_executable(testH264VideoToTransportStream ${testH264VideoToTransportStream_LIST})
TARGET_LINK_LIBRARIES(testH264VideoToTransportStream
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testH265VideoStreamer_LIST
	testH265VideoStreamer.cpp
)
add_executable(testH265VideoStreamer ${testH265VideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testH265VideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testH265VideoToTransportStream_LIST
	testH265VideoToTransportStream.cpp
)
add_executable(testH265VideoToTransportStream ${testH265VideoToTransportStream_LIST})
TARGET_LINK_LIBRARIES(testH265VideoToTransportStream
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testMKVStreamer_LIST
	testMKVStreamer.cpp
)
add_executable(testMKVStreamer ${testMKVStreamer_LIST})
TARGET_LINK_LIBRARIES(testMKVStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testMP3Receiver_LIST
	testMP3Receiver.cpp
)
add_executable(testMP3Receiver ${testMP3Receiver_LIST})
TARGET_LINK_LIBRARIES(testMP3Receiver
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testMP3Streamer_LIST
	testMP3Streamer.cpp
)
add_executable(testMP3Streamer ${testMP3Streamer_LIST})
TARGET_LINK_LIBRARIES(testMP3Streamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
 
 set(testMPEG1or2AudioVideoStreamer_LIST
	testMPEG1or2AudioVideoStreamer.cpp
)
add_executable(testMPEG1or2AudioVideoStreamer ${testMPEG1or2AudioVideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testMPEG1or2AudioVideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testMPEG1or2ProgramToTransportStream_LIST
	testMPEG1or2ProgramToTransportStream.cpp
)
add_executable(testMPEG1or2ProgramToTransportStream ${testMPEG1or2ProgramToTransportStream_LIST})
TARGET_LINK_LIBRARIES(testMPEG1or2ProgramToTransportStream
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testMPEG1or2Splitter_LIST
	testMPEG1or2Splitter.cpp
)
add_executable(testMPEG1or2Splitter ${testMPEG1or2Splitter_LIST})
TARGET_LINK_LIBRARIES(testMPEG1or2Splitter
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        

set(testMPEG1or2VideoReceiver_LIST
	testMPEG1or2VideoReceiver.cpp
)
add_executable(testMPEG1or2VideoReceiver ${testMPEG1or2VideoReceiver_LIST})
TARGET_LINK_LIBRARIES(testMPEG1or2VideoReceiver
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testMPEG1or2VideoStreamer_LIST
	testMPEG1or2VideoStreamer.cpp
)
add_executable(testMPEG1or2VideoStreamer ${testMPEG1or2VideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testMPEG1or2VideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        

set(testMPEG2TransportReceiver_LIST
	testMPEG2TransportReceiver.cpp
)
add_executable(testMPEG2TransportReceiver ${testMPEG2TransportReceiver_LIST})
TARGET_LINK_LIBRARIES(testMPEG2TransportReceiver
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        
set(testMPEG2TransportStreamer_LIST
	testMPEG2TransportStreamer.cpp
)
add_executable(testMPEG2TransportStreamer ${testMPEG2TransportStreamer_LIST})
TARGET_LINK_LIBRARIES(testMPEG2TransportStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        

set(testMPEG2TransportStreamTrickPlay_LIST
	testMPEG2TransportStreamTrickPlay.cpp
)
add_executable(testMPEG2TransportStreamTrickPlay ${testMPEG2TransportStreamTrickPlay_LIST})
TARGET_LINK_LIBRARIES(testMPEG2TransportStreamTrickPlay
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        
set(testMPEG4VideoStreamer_LIST
	testMPEG4VideoStreamer.cpp
)
add_executable(testMPEG4VideoStreamer ${testMPEG4VideoStreamer_LIST})
TARGET_LINK_LIBRARIES(testMPEG4VideoStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testOggStreamer_LIST
	testOggStreamer.cpp
)
add_executable(testOggStreamer ${testOggStreamer_LIST})
TARGET_LINK_LIBRARIES(testOggStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

        

set(testOnDemandRTSPServer_LIST
	testOnDemandRTSPServer.cpp
)
add_executable(testOnDemandRTSPServer ${testOnDemandRTSPServer_LIST})
TARGET_LINK_LIBRARIES(testOnDemandRTSPServer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )
        
set(testRelay_LIST
	testRelay.cpp
)
add_executable(testRelay ${testRelay_LIST})
TARGET_LINK_LIBRARIES(testRelay
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )

set(testReplicator_LIST
	testReplicator.cpp
)
add_executable(testReplicator ${testReplicator_LIST})
TARGET_LINK_LIBRARIES(testReplicator
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )  
   

set(testWAVAudioStreamer_LIST
	testWAVAudioStreamer.cpp
)
add_executable(testWAVAudioStreamer ${testWAVAudioStreamer_LIST})
TARGET_LINK_LIBRARIES(testWAVAudioStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )  
        
 set(vobStreamer_LIST
	vobStreamer.cpp
)
add_executable(vobStreamer ${vobStreamer_LIST})
TARGET_LINK_LIBRARIES(vobStreamer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )  
set(testSimpleDemo_LIST
	testSimpleDemo.cpp
)
