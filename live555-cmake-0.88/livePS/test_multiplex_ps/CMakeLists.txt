project(LivePS)
include_directories(".")
set(PS_SRC_LIST multiplexPs.cpp	utils.cpp test_multiplexPs.cpp ES2PS.cpp ES2TS.cpp pidint.c)
add_executable(test_multiplexPs ${PS_SRC_LIST})

