cmake_minimum_required(VERSION 3.15)

project(gemmbench LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rtlib=compiler-rt")

add_subdirectory(src)