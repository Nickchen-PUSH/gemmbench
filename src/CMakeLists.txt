include_directories(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(cli)
add_subdirectory(sample)
add_subdirectory(ops)
add_subdirectory(benchmark)
add_subdirectory(output)

add_executable(gemmbench main.cpp cli/cli.cpp)

target_link_libraries(gemmbench
    PRIVATE
    cli
    sample
    $<TARGET_OBJECTS:ops>
    benchmark
    output
)