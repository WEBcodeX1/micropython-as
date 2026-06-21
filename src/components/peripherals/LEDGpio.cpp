#include "LEDGpio.hpp"

using namespace std;


LEDGpio::LEDGpio()
{
}

LEDGpio::~LEDGpio()
{
}

void LEDGpio::setConfig()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = GpioBitmask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
}

void LEDGpio::updateValue()
{
    gpio_set_level(GpioPIN, Value);
}
