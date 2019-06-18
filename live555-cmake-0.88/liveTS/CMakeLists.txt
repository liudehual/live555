project(LiveTS)
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
	FrameCache.cpp
	LiveTsFrameSource.cpp
	LiveTsServerMediaSubSession.cpp
	OSMutex.cpp
	LiveTs.cpp
)
#add_definitions(-O2 -g) 
add_library(LiveTs STATIC ${SRC_LIST})
TARGET_LINK_LIBRARIES(LiveTs
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        ${my_pthread_lib}
        )

add_executable(testTsLiveServer testTsLiveServer.cpp)

TARGET_LINK_LIBRARIES(testTsLiveServer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        LiveTs
		${my_pthread_lib}
		)
