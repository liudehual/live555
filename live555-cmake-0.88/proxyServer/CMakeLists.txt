project(ProxyServer)

include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${LiveMedia_SOURCE_DIR}/include)
include_directories(${BasicUsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)

set(SRC_LIST
	live555ProxyServer.cpp
)
add_executable(ProxyServer ${SRC_LIST})
TARGET_LINK_LIBRARIES(ProxyServer LiveMedia GroupSock BasicUsageEnvironment UsageEnvironment)   

