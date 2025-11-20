cmake_minimum_required(VERSION 3.16)

project(STSSaveEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build Qt GUI" ON)
option(BUILD_CLI "Build CLI tool" ON)

add_subdirectory(src)

if(BUILD_GUI)
    add_subdirectory(src/gui)
endif()
