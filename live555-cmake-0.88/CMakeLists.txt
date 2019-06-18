project(live555)

# 编译显示报警信息
#add_definitions(-Wall)

if(LINUX)
 add_definitions(-O3 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64)
endif(LINUX)
if(WIN32)
endif(WIN32)
#添加调试信息(gdb)
#add_definitions(-g)

ADD_SUBDIRECTORY(UsageEnvironment)
ADD_SUBDIRECTORY(groupsock)
ADD_SUBDIRECTORY(BasicUsageEnvironment)
ADD_SUBDIRECTORY(liveMedia)
ADD_SUBDIRECTORY(liveH264)
ADD_SUBDIRECTORY(liveTS)
ADD_SUBDIRECTORY(mediaServer)
ADD_SUBDIRECTORY(testThread)
ADD_SUBDIRECTORY(testProgs)
ADD_SUBDIRECTORY(proxyServer)

