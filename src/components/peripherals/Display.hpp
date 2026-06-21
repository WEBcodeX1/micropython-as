#pragma once

#include <string>
#include "ssd1306.h"

using namespace std;

//- GPIO

// ESP-C3
/*
static const uint8_t CONFIG_SDA_GPIO = 6;
static const uint8_t CONFIG_SCL_GPIO = 7;
static const uint8_t CONFIG_RESET_GPIO = -1;
*/

// ESP-S3
static const uint8_t CONFIG_SDA_GPIO = 5;
static const uint8_t CONFIG_SCL_GPIO = 6;
static const uint8_t CONFIG_RESET_GPIO = -1;

//- line coordinate container struct
struct LineCoords_t {
    uint8_t xPosStart;
    uint8_t yPosStart;
    uint8_t xPosEnd;
    uint8_t yPosEnd;
};


class Display
{

public:

    Display(bool init=true);
    ~Display();

    SSD1306_t getDisplayDev();
    void setDisplayDev(SSD1306_t);

    void renderText(uint8_t, std::string);
    void drawLine(LineCoords_t, bool invert=false);
    void showBuffer();

private:

    SSD1306_t DisplayDev;
    void init();

};
