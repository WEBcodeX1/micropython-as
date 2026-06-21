#include "Display.hpp"

using namespace std;


Display::Display(bool init)
{
    if (init == true) {
        Display::init();
    }
}

Display::~Display()
{
}

void Display::init()
{
    i2c_master_init(&DisplayDev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&DisplayDev, 128, 64);
    ssd1306_clear_screen(&DisplayDev, false);
    ssd1306_contrast(&DisplayDev, 0xff);
}

void Display::renderText(uint8_t LineIndex, std::string DisplayText)
{
    ssd1306_display_text(&DisplayDev, LineIndex, DisplayText.c_str(), DisplayText.length(), false);
}

void Display::drawLine(LineCoords_t Coord, bool invert)
{
    _ssd1306_line(&DisplayDev, Coord.xPosStart, Coord.yPosStart, Coord.xPosEnd, Coord.yPosEnd, invert);
}

void Display::showBuffer()
{
    ssd1306_show_buffer(&DisplayDev);
}

SSD1306_t Display::getDisplayDev()
{
    return DisplayDev;
}

void Display::setDisplayDev(SSD1306_t SetDisplayDev)
{
    DisplayDev = SetDisplayDev;
}
