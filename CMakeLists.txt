cmake_minimum_required(VERSION 4.1)
project(exercise_lib)

set(CMAKE_C_STANDARD 23)
add_library(exercise_lib STATIC functions.c)
