project(mediaServerThread)
include_directories(.)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)

if(LINUX)
else(LINUX)
include_directories(${live555_SOURCE_DIR}/win-pthread/include)
link_directories(${live555_SOURCE_DIR}/win-pthread/lib/x64)
endif(LINUX)

if(LINUX)
set(my_pthread_lib "pthread")
else(LINUX)
set(my_pthread_lib "pthreadVC2")
endif(LINUX)

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
	${my_pthread_lib}
	)   
