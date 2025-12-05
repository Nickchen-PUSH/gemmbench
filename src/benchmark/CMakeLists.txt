add_library(benchmark benchmark.cpp verify.cpp)
target_include_directories(benchmark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})