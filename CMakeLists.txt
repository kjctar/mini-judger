cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)
add_executable(untitled JudgerSever/judger_server.cpp Judger/judger.cpp  Judger/safe_queue.h  Judger/msg_cache.h  Judger/msg.h  Judger/language.cpp  Judger/language.h  Judger/submit.cpp)
