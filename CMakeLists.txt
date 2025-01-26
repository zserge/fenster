cmake_minimum_required(VERSION 3.22.1)
project(fenster VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_library(fenster
        STATIC
        src/fenster.cpp
)