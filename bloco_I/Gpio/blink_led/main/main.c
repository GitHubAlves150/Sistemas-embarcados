#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define LED_PIN GPIO_NUM_12

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(LED_PIN, 1);
        printf("\nled on");
        vTaskDelay(pdMS_TO_TICKS(2000));

        gpio_set_level(LED_PIN, 0);
        printf("\nled off");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}