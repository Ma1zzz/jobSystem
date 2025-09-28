cmake_minimum_required(VERSION 4.0)
project(job)

set(CMAKE_CXX_STANDARD 20)

add_executable(job main.cpp jobs.cpp jobs.h testProgram.cpp)
