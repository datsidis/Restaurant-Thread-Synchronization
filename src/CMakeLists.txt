cmake_minimum_required(VERSION 4.0)
project(assignment2 C)

set(CMAKE_C_STANDARD 90)

add_executable(assignment2 main.c
        restaurant.c
        restaurant.h)
