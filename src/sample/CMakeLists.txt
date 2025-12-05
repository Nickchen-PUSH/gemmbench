add_library(sample sample_generator.cpp sample_io.cpp reference_gemm.cpp)
target_include_directories(sample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})