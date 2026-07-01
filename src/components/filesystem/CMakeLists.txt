idf_component_register(
	SRCS Filesystem.cpp
    PRIV_REQUIRES main
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++11)
