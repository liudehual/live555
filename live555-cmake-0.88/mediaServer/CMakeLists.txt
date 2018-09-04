project(MediaServer)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
message("hello world")

set(SRC_LIST
	DynamicRTSPServer.cpp 
	live555MediaServer.cpp
)
add_executable(live555MediaServer ${SRC_LIST})
TARGET_LINK_LIBRARIES(live555MediaServer LiveMedia GroupSock BasicUsageEnvironment UsageEnvironment)   
