project(LiveH264)
include_directories(.)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
	AudioServerMediaSubsession.cpp
	FramedAudioSource.cpp
	FramedLiveSource.cpp
	FramesManager.cpp
	Live.cpp
	LiveServerMediaSubsession.cpp
	LiveClient.cpp
)
#link_libraries(${UsageEnvironment_SOURCE_DIR})
#link_libraries(${GroupSock_SOURCE_DIR})
#link_libraries(${LiveMedia_SOURCE_DIR})
#link_libraries(${BasicUsageEnvironment_SOURCE_DIR})

add_library(LiveH264 STATIC ${SRC_LIST})
TARGET_LINK_LIBRARIES(LiveH264
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        )


add_executable(test_live test_live.cpp)
TARGET_LINK_LIBRARIES(test_live
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        LiveH264
        pthread
        )