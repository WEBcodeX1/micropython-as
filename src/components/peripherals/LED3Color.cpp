#include "LED3Color.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace std;


LED3Color::LED3Color()
{
}

LED3Color::~LED3Color()
{
}

void LED3Color::init(gpio_num_t GpioPIN1, gpio_num_t GpioPIN2, gpio_num_t GpioPIN3)
{
    LED1 = new LEDPwm();

    LED1->GpioPIN = GpioPIN1;
    LED1->HwTimer = LEDC_TIMER_0;
    LED1->HwChannel = LEDC_CHANNEL_0;
    LED1->setConfig();

    LED2 = new LEDPwm();

    LED2->GpioPIN = GpioPIN2;
    LED2->HwTimer = LEDC_TIMER_1;
    LED2->HwChannel = LEDC_CHANNEL_1;
    LED2->setConfig();

    LED3 = new LEDPwm();

    LED3->GpioPIN = GpioPIN3;
    LED3->HwTimer = LEDC_TIMER_2;
    LED3->HwChannel = LEDC_CHANNEL_2;
    LED3->setConfig();
}

void LED3Color::setColor(uint16_t Color1, uint16_t Color2, uint16_t Color3)
{
    LED1->Value = Color1;
    LED2->Value = Color2;
    LED3->Value = Color3;
    updateColor();
}

void LED3Color::updateColor()
{
    LED1->updateValue();
    LED2->updateValue();
    LED3->updateValue();
}

void LED3Color::fade(FadeConfig_t FadeConfig)
{
    setColor(
        FadeConfig.ColRedStart,
        FadeConfig.ColGreenStart,
        FadeConfig.ColBlueStart
    );

    for (uint8_t i = 0; i < FadeConfig.IterationCount; i++) {
        LED1->Value += FadeConfig.ColRedAdd;
        LED2->Value += FadeConfig.ColGreenAdd;
        LED3->Value += FadeConfig.ColBlueAdd;
        updateColor();
        vTaskDelay(FadeConfig.IterationDelay);
    }
}
