idf_component_register(
    SRCS Network.cpp NetworkWifi.cpp ClientHandler.cpp
    PRIV_REQUIRES main
    INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++11)

add_library(httpparser_lib STATIC IMPORTED)
set_property(TARGET httpparser_lib PROPERTY IMPORTED_LOCATION /usr/local/lib/libhttpparser.a)

target_link_libraries(${COMPONENT_LIB} PRIVATE httpparser_lib)
