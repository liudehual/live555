project(GroupSock)
include_directories(include)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
	GroupEId.cpp
	Groupsock.cpp
	GroupsockHelper.cpp
	IOHandlers.cpp
	NetAddress.cpp
	NetInterface.cpp
	inet.c
)
add_library(GroupSock STATIC ${SRC_LIST})
