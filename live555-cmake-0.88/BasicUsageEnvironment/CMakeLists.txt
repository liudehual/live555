project(BasicUsageEnvironment)
include_directories(include)
include_directories(${UsageEnvironment_SOURCE_DIR}/include)
include_directories(${GroupSock_SOURCE_DIR}/include)
include_directories(${live555_SOURCE_DIR}/include)

set(SRC_LIST
	BasicHashTable.cpp
	BasicTaskScheduler0.cpp
	BasicTaskScheduler.cpp
	BasicUsageEnvironment0.cpp
	BasicUsageEnvironment.cpp
	DelayQueue.cpp
)
add_library(BasicUsageEnvironment STATIC ${SRC_LIST})
