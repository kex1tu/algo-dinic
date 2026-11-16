cmake_minimum_required(VERSION 3.16)
project(DinicExperiment LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dinic_core STATIC src/dinic.cpp)
add_library(graph_generator STATIC src/generator.cpp)

add_executable(dinic src/dinic_main.cpp)
target_link_libraries(dinic PRIVATE dinic_core)

add_executable(generator src/generator_main.cpp)
target_link_libraries(generator PRIVATE graph_generator)

add_executable(experiment src/experiment.cpp)
target_link_libraries(experiment PRIVATE dinic_core graph_generator)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target dinic_core graph_generator dinic generator experiment)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()
