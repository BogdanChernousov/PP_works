cmake_minimum_required(VERSION 3.15)
project(HeatEquationSolver LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "pgc++")
set(CMAKE_CXX_FLAGS "-fast -O3 -Minfo=all -acc=gpu")

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(heat_solver main.cpp)
target_link_libraries(heat_solver ${Boost_LIBRARIES} cublas)

# Для удобства сборки разных режимов
add_custom_target(build_host
    COMMAND ${CMAKE_CXX_COMPILER} -acc=host -fast -O3 -Minfo=all main.cpp -o heat_solver_host -lboost_program_options -lcublas
    COMMENT "Building for CPU (host)")

add_custom_target(build_multicore
    COMMAND ${CMAKE_CXX_COMPILER} -acc=multicore -fast -O3 -Minfo=all main.cpp -o heat_solver_multicore -lboost_program_options -lcublas
    COMMENT "Building for multicore CPU")

add_custom_target(build_gpu
    COMMAND ${CMAKE_CXX_COMPILER} -acc=gpu -fast -O3 -Minfo=all main.cpp -o heat_solver_gpu -lboost_program_options -lcublas
    COMMENT "Building for GPU")