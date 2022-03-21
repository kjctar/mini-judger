cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 11)

add_executable(untitled judger_server.cpp judger_core.cpp safe_queue.h msg_cache.h msg.cpp msg.h language.cpp language.h submit.cpp)
