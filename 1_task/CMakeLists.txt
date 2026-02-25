cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(task_1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_FLOAT "use float instead of double" ON)

if(USE_FLOAT)
    message(STATUS "building with float")
    add_compile_definitions(USE_FLOAT)
else()
    message(STATUS "building with double")
endif()

add_executable(task_1 task_1.cpp)