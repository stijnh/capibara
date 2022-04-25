cmake_minimum_required(VERSION 3.0)
project(experiment)

set(CMAKE_CXX_FLAGS "-O0 -Wall -Wextra -g")

set(CMAKE_CXX_STANDARD 14)


add_executable(experiment test.cpp)

target_include_directories(experiment PRIVATE include)
