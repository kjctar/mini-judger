cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS -pthread)
add_executable(untitled JudgerCore/JudgerSever/judger_server.cpp JudgerCore/Judger/judger.cpp JudgerCore/Judger/safe_queue.h JudgerCore/Judger/msg_cache.h JudgerCore/Judger/msg.h JudgerCore/Judger/language.cpp JudgerCore/Judger/language.h JudgerCore/Judger/submit.cpp)
