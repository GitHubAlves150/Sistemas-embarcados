// Watchdog
// Lucas Lorenço Alves
// Engenharia da Computação
// Ano 2024

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_clk_tree.h" // API atual da árvore de clocks (substitue esp_clk.h em versões recentes do IDF)

void app_main(void)
{
    printf("iniciando....watchdog padrão (idle task) esta ativo por default.\n");
    printf("Vou travar de propósito em 3 segundos.\n");

    vTaskDelay(pdMS_TO_TICKS(3000));

    printf("Travando agora (loop infinito sem yield)...");

    // ATENÇÃO: isso nunca cede o processador -> idle task nunca roda -> watchdog dispara
    while (1)
    {
        
    }
}
