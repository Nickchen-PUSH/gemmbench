file(GLOB OPS_SOURCES "*.cpp")
file(GLOB OPS_HEADERS "*.h")
add_library(ops OBJECT ${OPS_SOURCES} ${OPS_HEADERS})

target_include_directories(ops PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})