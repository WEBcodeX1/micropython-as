idf_component_register(
    SRCS Server.cpp DNSServer.cpp ClientHandler.cpp Client.cpp ASRequestHandler.cpp
    PRIV_REQUIRES main esp_netif filesystem
    INCLUDE_DIRS "."
)

add_library(httpparser_lib STATIC IMPORTED)
set_property(TARGET httpparser_lib PROPERTY IMPORTED_LOCATION /usr/local/lib/esp32s3/libhttpparser.a)

target_link_libraries(${COMPONENT_LIB} PRIVATE httpparser_lib)
