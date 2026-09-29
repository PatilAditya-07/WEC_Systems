cmake_minimum_required(VERSION 4.3)
project(WEC_Systems_OS C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(WEC_Systems_OS main.c
        structures.h
)
