#pragma once

// Stub ESP logging macros for Linux builds

#include <stdio.h>

#define ESP_LOGI(tag, fmt, ...) \
    printf("[INFO][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOGW(tag, fmt, ...) \
    printf("[WARN][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOGE(tag, fmt, ...) \
    fprintf(stderr, "[ERR][" tag "] " fmt "\n", ##__VA_ARGS__)

#define ESP_LOG_BUFFER_HEX_LEVEL(tag, buf, len, level) \
    do {} while(0)

#define ESP_LOG_BUFFER_CHAR_LEVEL(tag, buf, len, level) \
    do {} while(0)
