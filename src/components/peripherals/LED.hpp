#pragma once

// include driver
#include "driver/ledc.h"
#include "driver/gpio.h"

// PWM constant defines
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES      LEDC_TIMER_13_BIT
#define LEDC_CLK_SRC       LEDC_AUTO_CLK
#define LEDC_FREQUENCY     (4000)


using namespace std;


class LED
{

public:

    virtual void setConfig() = 0;
    virtual void updateValue() = 0;

    gpio_num_t GpioPIN;
    uint16_t Value;
};
