cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME FoxFire_Engine)
project(FoxFireEngineRewrite VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 26)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_link_options(${PROJECT_NAME}
    PRIVATE
        -static-libgcc
        -static-libstdc++
)

target_link_libraries(${PROJECT_NAME})


