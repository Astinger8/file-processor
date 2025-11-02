cmake_minimum_required(VERSION 3.28)
project(untitled2 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled2
        file_processor_svr.c
        file_processor_clnt.c)
