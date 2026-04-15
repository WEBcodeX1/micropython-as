idf_component_register(
    SRCS "ClientHandler.cpp"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)
