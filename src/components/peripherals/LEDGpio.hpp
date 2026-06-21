#pragma once

#include "LED.hpp"


using namespace std;


class LEDGpio : public LED
{

public:

    LEDGpio();
    ~LEDGpio();

    // (1ULL<<1) | (1ULL<<2) | (1ULL<<3) )
    uint64_t GpioBitmask;

    void setConfig() override;
    void updateValue() override;

};
