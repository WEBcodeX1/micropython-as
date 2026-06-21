#pragma once

#include "LEDPwm.hpp"


using namespace std;


//- 3 color fade configuration
struct FadeConfig_t {
    uint16_t ColRedStart;
    uint16_t ColGreenStart;
    uint16_t ColBlueStart;
    int8_t ColRedAdd;
    int8_t ColGreenAdd;
    int8_t ColBlueAdd;
    uint8_t IterationCount;
    uint16_t IterationDelay;
};


class LED3Color
{

public:

    LED3Color();
    ~LED3Color();

    LEDPwm* LED1;
    LEDPwm* LED2;
    LEDPwm* LED3;

    void init(gpio_num_t, gpio_num_t, gpio_num_t);
    void setColor(uint16_t, uint16_t, uint16_t);
    void updateColor();
    void fade(FadeConfig_t);

};
