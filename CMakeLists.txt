cmake_minimum_required(VERSION 3.10)

project(yarc)
enable_language(C CXX)

add_subdirectory(src)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)