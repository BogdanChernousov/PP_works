cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(task_1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти пакет OpenMP
find_package(OpenMP REQUIRED)

add_executable(task_2_1 task_2_1.cpp)

# Подключить OpenMP к компиляции и линковке
target_link_libraries(task_2_1 PRIVATE OpenMP::OpenMP_CXX)