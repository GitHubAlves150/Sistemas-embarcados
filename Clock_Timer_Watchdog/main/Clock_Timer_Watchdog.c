// Consultar a frequẽncia da CPU
// Lucas Lorenço Alves
// Engenharia da Computação
// Ano 2024


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_clk_tree.h" // API atual da árvore de clocks (substitue esp_clk.h em versões recentes do IDF) 





void app_main(void)
{

        uint32_t cpu_freq_hz = 0;
        esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_APPROX, &cpu_freq_hz);

        printf("Frequẽncia da CPU: %lu HZ (%.1f MHZ)\n", (unsigned long)cpu_freq_hz, cpu_freq_hz/1e6);

        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
                

}
