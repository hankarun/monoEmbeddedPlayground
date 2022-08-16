cmake_minimum_required(VERSION 3.7)
project(monoPlayground CXX CSharp)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(engine)
add_subdirectory(engineMono)
add_subdirectory(game)

