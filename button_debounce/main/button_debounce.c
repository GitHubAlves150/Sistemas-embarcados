#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_32   // ajuste para o botão físico da sua T-A7670
#define DEBOUNCE_MS      30
#define LONG_PRESS_MS   1000
#define POLL_INTERVAL_MS  10

// Estado lógico "confirmado" do botão (depois do debounce)
typedef enum {
    BTN_RELEASED,
    BTN_PRESSED
} btn_state_t;

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY); // botão liga ao GND quando pressionado

    btn_state_t debounced_state = BTN_RELEASED;
    btn_state_t last_raw_state  = BTN_RELEASED;
    int64_t last_change_time_us = 0;
    int64_t press_start_us = 0;

    int clicks_curtos = 0;
    int clicks_longos = 0;

    while (1) {
        // nível 0 = pressionado (pull-up interno, botão ao GND)
        int level = gpio_get_level(BUTTON_PIN);
        btn_state_t raw_state = (level == 0) ? BTN_PRESSED : BTN_RELEASED;
        int64_t now_us = esp_timer_get_time();

        if (raw_state != last_raw_state) {
            // o nível bruto mudou agora -> reinicia a contagem de estabilidade
            last_change_time_us = now_us;
            last_raw_state = raw_state;
        }

        // só aceita a mudança como "real" se ficou estável tempo suficiente
        if (raw_state != debounced_state &&
            (now_us - last_change_time_us) >= (DEBOUNCE_MS * 1000)) {

            debounced_state = raw_state;

            if (debounced_state == BTN_PRESSED) {
                // transição confirmada: solto -> pressionado
                press_start_us = now_us;
                printf("Botao pressionado\n");
            } else {
                // transição confirmada: pressionado -> solto
                int64_t duration_ms = (now_us - press_start_us) / 1000;

                if (duration_ms >= LONG_PRESS_MS) {
                    clicks_longos++;
                    printf("Clique LONGO (%lld ms) | curtos=%d longos=%d\n",
                           duration_ms, clicks_curtos, clicks_longos);
                } else {
                    clicks_curtos++;
                    printf("Clique curto (%lld ms) | curtos=%d longos=%d\n",
                           duration_ms, clicks_curtos, clicks_longos);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}