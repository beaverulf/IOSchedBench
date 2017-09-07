cmake_minimum_required(VERSION 3.8)
project(IOSchedBench)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c lib/thpool.h lib/thpool.c)
add_executable(IOSchedBench ${SOURCE_FILES})