#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_32 // Pino físico em que o botão está ligado
#define DEBOUNCE_MS 30         // Tempo de 30ms que o nível do pino precisa ficar estável antes de aceitarmos a mudança como real
                               // Isso existe por que um botão mecânico, ao ser pressionado ou solto, não faz uma transição limpa de 0 pra 1
                               // O contato "quica" fisicamente e gera várias transições falsas em poucos milisegundos.
#define LONG_PRESS_MS 10000    // O LONG_PRESS_MS tem um limite de 1000ms=1s que separa de um click "curto" de um "longo".
#define POLL_INTERVAL_MS 10    // A cada quantos milisegundos o programa vai checar o pino (10ms=100leituras por segundos)

// RELEASED - Liberado
// PRESSED  - Pressionado

typedef enum
{
    BTN_RELEASED, // enum=0
    BTN_PRESSED   // 1
} btn_state_pin;

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);                       // Reseta o estado do pino para padrão do esp32 - padrão zero
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);  // Define o Pino como entrada
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY); // Ativa o resilstor de pullup interno

    // Variaveis de estado que formam uma pequena memória de estado
    btn_state_pin debounce_state = BTN_RELEASED;
    btn_state_pin last_raw_state = BTN_RELEASED;
    int64_t last_cheange_time_us = 0;
    int64_t press_start_us = 0;

    int clicks_curtos = 0;
    int clicks_longos = 0;

    // O loop principal - A lógica de debounce
    while (1)
    {
        int level = gpio_get_level(BUTTON_PIN);
        btn_state_pin raw_state = (level == 0) ? BTN_PRESSED : BTN_RELEASED;
        int64_t now_us = esp_timer_get_time(); // Pega o timestamp atual em microssegundos

        // 1. Detectar se o nível bruto mudou desde a última leitura
        if (raw_state != last_raw_state)
        {
            last_cheange_time_us = now_us;
            last_raw_state = raw_state;
            printf("\nraw_state != last_raw_state confirmado......\n");
        }

        // 2. Só aceitar a mudança como real depois da estabilidade ser suficiente
        if (raw_state != debounce_state && (now_us - last_cheange_time_us) >= (DEBOUNCE_MS * 1000))
        {
            debounce_state = raw_state;
            // 3. Reagir à transição confirmada
            if (debounce_state == BTN_PRESSED)
            {
                press_start_us = now_us;
                printf("Botão pressionado\n");
            }
            else
            {
                int64_t duration_ms = (now_us - press_start_us) / 1000;

                if (duration_ms >= LONG_PRESS_MS)
                {
                    clicks_longos++;
                    printf("--Click LONGO (%lld ms) | Curto = %d Longo = %d\n", duration_ms, clicks_curtos, clicks_longos);
                }
                else
                {
                    clicks_curtos++;
                    printf("--Click Curto (%lld) | Curto = %d Longo = %d\n", duration_ms, clicks_curtos, clicks_longos);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}