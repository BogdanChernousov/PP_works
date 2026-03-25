cmake_minimum_required(VERSION 3.20)
project(ServerClientDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Основная программа
add_executable(server_client 3_2_task.cpp)
target_link_libraries(server_client PRIVATE pthread)

# Тестовая программа
add_executable(test_result test_result.cpp)
target_link_libraries(test_result PRIVATE pthread)