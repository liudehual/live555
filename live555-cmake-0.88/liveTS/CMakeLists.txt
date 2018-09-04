project(LiveTS)
include_directories(.)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
	FrameCache.cpp
	LiveTsFrameSource.cpp
	LiveTsServerMediaSubSession.cpp
	OSMutex.cpp
	LiveTs.cpp
)
add_definitions(-O2 -g) 
add_library(LiveTs STATIC ${SRC_LIST})
TARGET_LINK_LIBRARIES(LiveTs
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        pthread
        )
add_executable(testTsLiveServer testTsLiveServer.cpp)

TARGET_LINK_LIBRARIES(testTsLiveServer
        LiveMedia 
        GroupSock 
        BasicUsageEnvironment 
        UsageEnvironment
        LiveTs
        pthread
        )