cmake_minimum_required(VERSION 3.0)
project(experiment)

set(CMAKE_CXX_FLAGS "-O0 -Wall -Wextra -g")
set(CMAKE_CXX_STANDARD 14)

add_library(capibara INTERFACE)
target_include_directories(capibara INTERFACE include)

add_subdirectory(tests)
