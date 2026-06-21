#include "LEDPwm.hpp"

using namespace std;


LEDPwm::LEDPwm()
{
}

LEDPwm::~LEDPwm()
{
}

void LEDPwm::setConfig()
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = HwTimer,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_CLK_SRC,
        .deconfigure      = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = GpioPIN,
        .speed_mode     = LEDC_MODE,
        .channel        = HwChannel,
        .timer_sel      = HwTimer,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void LEDPwm::updateValue()
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, HwChannel, Value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, HwChannel));
}
