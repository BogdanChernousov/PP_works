cmake_minimum_required(VERSION 3.20)
project(3_1_task)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(3_1_task 3_1_task.cpp)
target_link_libraries(3_1_task PRIVATE pthread)