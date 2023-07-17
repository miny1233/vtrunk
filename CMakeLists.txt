cmake_minimum_required(VERSION 3.25)
project(vtrunk)

set(CMAKE_CXX_STANDARD 20)

INCLUDE_DIRECTORIES(include)

add_executable(vtrunk main.cpp include/kcp.h kcp/kcp.cpp kcp/ikcp.c)
