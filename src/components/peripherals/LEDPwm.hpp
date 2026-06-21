#pragma once

#include "LED.hpp"


using namespace std;


class LEDPwm : public LED
{

public:

    LEDPwm();
    ~LEDPwm();

    ledc_timer_t HwTimer;
    ledc_channel_t HwChannel;

    void setConfig() override;
    void updateValue() override;

};
