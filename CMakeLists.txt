cmake_minimum_required(VERSION 4.0)

project(FoxFire_Engine VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

set(FOXFIRE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Instance)

add_subdirectory(Engine)
add_subdirectory(Sandbox)