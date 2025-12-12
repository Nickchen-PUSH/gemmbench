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

get_property(_OPS_OBJECT_TARGETS GLOBAL PROPERTY OPS_OBJECT_TARGETS)
if(_OPS_OBJECT_TARGETS)
	foreach(_ops_target IN LISTS _OPS_OBJECT_TARGETS)
		target_sources(gemmbench PRIVATE $<TARGET_OBJECTS:${_ops_target}>)
	endforeach()
endif()


# target_link_libraries(gemmbench PRIVATE gcc_s)

# clang -march=armv8.6-a+sme+sve2+sve -lgcc_s \
# -L/home/share/pcdemo/home/guest1_test/HPCKit_25.1.0/HPCKit/25.1.0/compiler/bisheng/lib/aarch64-unknown-linux-gnu/ -rtlib=compiler-rt \
# -O3
