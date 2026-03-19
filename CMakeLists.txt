cmake_minimum_required(VERSION 4.1)
project(steganography C)

set(CMAKE_C_STANDARD 11)

add_executable(steganography main.c
        PNG.c
        PNG.h)
target_link_libraries(steganography ws2_32)