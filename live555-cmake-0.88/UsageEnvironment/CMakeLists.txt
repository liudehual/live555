project(UsageEnvironment)

include_directories(include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)
set(SRC_LIST
	HashTable.cpp
	strDup.cpp
	UsageEnvironment.cpp
)
add_library(UsageEnvironment STATIC ${SRC_LIST})
