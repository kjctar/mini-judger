cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)
add_executable(untitled judger_server.cpp judger_core.cpp safe_queue.h msg_cache.h msg.h language.cpp language.h submit.cpp)
