cmake_minimum_required(VERSION 3.15)
project(moore C)

set(CMAKE_C_STANDARD 99)

add_executable(main.c main.h svg.c svg.h moore.c moore.h)