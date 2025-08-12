cmake_minimum_required(VERSION 3.10)

project(tsafeQ CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -lpthread")

add_executable(tsafeQ 
    main.cpp
    tsqueue.cpp
)