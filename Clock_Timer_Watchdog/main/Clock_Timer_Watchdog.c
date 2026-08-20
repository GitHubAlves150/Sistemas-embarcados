// projeto que registra a task no watchdog corretamente
// e o alimenta (esp_task_wdt_reset()) dentro de um loop de "trabalho simulado",
// provando que dá pra evitar o reset mesmo com o watchdog ativo.

// Lucas Lorenço Alves
// Engenharia da Computação
// Ano 2024

// =========================================================================================================================================================
// Importa as ferramentas necessárias. A `esp_task_wdt.h` ativa as funções de controle do "Cão de Guarda" de tarefas do ESP-IDF.
// =========================================================================================================================================================

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_clk_tree.h" // API atual da árvore de clocks (substitue esp_clk.h em versões recentes do IDF)
#include "esp_task_wdt.h"



void app_main(void)
{
    // 1. Configurando o watchdog
    // Define as regras do jogo. Criamos uma estrutura que diz ao hardware qual será o tempo máximo de tolerância antes de resetar o chip.
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 10000,                 // Tempo limite de 10 segundos antes do reset
        .idle_core_mask = 0,                 // Não monitora as tarefas ociosas padrões do sistema
        .trigger_panic = true                // Se estourar o tempo, gera um erro crítico e reinicia
    };

    // 2. Aplica a nova configuração no chip
    esp_task_wdt_reconfigure(&twdt_config);
    // Importância: O ESP32 já vem com um Watchdog padrão de fábrica. Esta linha diz: *"Esqueça o padrão, use a minha nova regra de 10 segundos"*.
    
    // 3. Registra esta tarefa específica (app_main) no sistema de monitoramento
    esp_task_wdt_add(NULL);

    // Importância: Passar `NULL` significa que a própria função atual (`app_main`) está entrando na lista de tarefas vigiadas pelo guarda. A partir daqui, ela **obrigatoriamente** precisa se identificar no tempo correto.

    printf("Task resgitrada no watchdog (time 10s). Alimentando a cada 1s.\n");

    int ciclo = 0;
    while (1)
    {
        printf("Ciclo %d de trabalho .....\n", ciclo++);
        // 4. Pausa/Simulação de processamento pesado
        
        vTaskDelay(pdMS_TO_TICKS(19000));        
        // Análise Técnica: Esta linha coloca a tarefa para dormir por 19 segundos. Como o limite do Watchdog é de 10 segundos, o chip sofrerá um **Reset por Hardware** bem no meio desta linha, impedindo que o código abaixo dela seja executado. *(Nota: Para o código rodar para sempre sem resetar, esse valor deveria ser menor que 10000, como 1000 ms).*
        
        // 5. Alimenta o Watchdog (Aviso de "Estou vivo!")
        esp_err_t err =  esp_task_wdt_reset();

        // 6. Validação de segurança
        if(err != ESP_OK){
            printf("ATENÇAO: Falha ao resetar watchdog! err=%d\n", err);
        }
    // Importância: O `esp_task_wdt_reset()` zera o cronômetro do Watchdog. Guardamos o resultado na variável `err` para verificar se o comando funcionou com sucesso, uma prática essencial em softwares industriais de alta confiabilidade.
    }

}
