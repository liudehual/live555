project(mediaServerThread)
include_directories(.)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
	mediaServerThread.cpp 
	liveThread.cpp
	DynamicRTSPServer.cpp
)

add_executable(mediaServerThread ${SRC_LIST})
TARGET_LINK_LIBRARIES(mediaServerThread
	LiveMedia 
	GroupSock 
	BasicUsageEnvironment 
	UsageEnvironment
	pthread
	)   
