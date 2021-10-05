cmake_minimum_required(VERSION 3.10)
project(WebServer)
add_definitions(-std=c++11)

include_directories(log net)
aux_source_directory(log  DIR_SRCS1)
aux_source_directory(net DIR_SRCS2)
add_executable(WebServer ${DIR_SRCS1} ${DIR_SRCS2})

find_package(Threads)
target_link_libraries(WebServer ${CMAKE_THREAD_LIBS_INIT})

