#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#define BUZZER_GPIO  GPIO_NUM_27
#define BUZZ_CH      LEDC_CHANNEL_0
void buzzer_init(void)
{
    ledc_timer_config_t t = { .speed_mode=LEDC_LOW_SPEED_MODE,
        .timer_num=LEDC_TIMER_0, .duty_resolution=LEDC_TIMER_10_BIT, .freq_hz=2500 };
    ledc_timer_config(&t);
    ledc_channel_config_t c = { .gpio_num=BUZZER_GPIO, .speed_mode=LEDC_LOW_SPEED_MODE,
        .channel=BUZZ_CH, .timer_sel=LEDC_TIMER_0, .duty=0 };
    ledc_channel_config(&c);
}

void buzzer_on(void)  { ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZ_CH, 512); ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZ_CH); }
void buzzer_off(void) { ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZ_CH,   0); ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZ_CH); }