add_library(ops OBJECT)

target_include_directories(ops PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(OPS_COMMON_COMPILE_OPTIONS
	# -march=armv8.6-a+sme+sve2+sve
	-O3
)

# keep track of every per-op object library so we can inject them later
set_property(GLOBAL PROPERTY OPS_OBJECT_TARGETS "")
set_property(GLOBAL PROPERTY OPS_LINK_LIBRARIES "")

# helper to build each operator with its own compiler flags
function(register_op name)
	cmake_parse_arguments(OP "" "" "SOURCES;HEADERS;COMPILE_OPTIONS;LINK_LIB;INCLUDE_DIR" ${ARGN})

	if(NOT OP_SOURCES)
		message(FATAL_ERROR "register_op(${name}) requires at least one source file.")
	endif()

	set(op_target "${name}_object")
	add_library(${op_target} OBJECT ${OP_SOURCES} ${OP_HEADERS})
	if(OP_LINK_LIB)
        set_property(GLOBAL APPEND PROPERTY OPS_LINK_LIBRARIES ${OP_LINK_LIB})
    endif()
	target_include_directories(${op_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(${op_target} PRIVATE ${OP_INCLUDE_DIR})
	target_compile_options(${op_target} PRIVATE ${OPS_COMMON_COMPILE_OPTIONS} ${OP_COMPILE_OPTIONS})
	set_property(TARGET ${op_target} PROPERTY POSITION_INDEPENDENT_CODE ON)
	set_property(GLOBAL APPEND PROPERTY OPS_OBJECT_TARGETS ${op_target})
endfunction()

# operator implementations
register_op(naive_op
	SOURCES naive_op.cpp
	HEADERS naive_op.h
)

register_op(naive_op_column_major
	SOURCES naive_op_column_major.cpp
	HEADERS naive_op_column_major.h
)


target_sources(ops PRIVATE registry.cpp registry.h)