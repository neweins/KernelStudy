cmake_minimum_required(VERSION 2.8.7)
project(algorithm CXX)

include_directories(
    /usr/include
    src/
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") #-Wall -Wfatal-errors -std=c++11 -g
file(GLOB_RECURSE SOURCES
    src/lru_algorithm.cpp # Input filename to compile
)

add_executable(lru "${SOURCES}")

SET_TARGET_PROPERTIES (
    lru
    PROPERTIES LINKER_LANGUAGE CXX
)

