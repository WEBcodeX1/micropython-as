#pragma once

// Stub FreeRTOS task delay for Linux builds.
// vTaskDelay(x) maps to x ticks at the default ESP-IDF 100 Hz tick rate (1 tick = 10 ms).

#include <unistd.h>

static inline void vTaskDelay(int ticks)
{
    usleep(ticks * 10000);
}
