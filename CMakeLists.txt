cmake_minimum_required(VERSION 3.7)
project(raytracer_in_one_weekend CXX CSharp)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(engine)
add_subdirectory(engineMono)
add_subdirectory(game)

