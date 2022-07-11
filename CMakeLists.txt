# CMake最低版本号要求
cmake_minimum_required(VERSION 3.10)
# 项目信息
project(SignalsAndSlots)

set(CMAKE_CXX_STANDARD 20)

add_executable(SignalsAndSlots  main.cpp)

#判断 系统类型
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    #linux 需要自己链接线程库
    find_package(Threads)
    target_link_libraries(LiveShare ${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")