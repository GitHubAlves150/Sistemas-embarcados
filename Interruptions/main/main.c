// Interrupção de borda com fila
// Engenharia da Computação
// Lucas Lorenço Alves
// Ano 2026



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"


#define BUTTON_PIN GPIO_NUM_32  

static QueueHandle_t gpio_evt_queue = NULL;   // Crio o berço- A fila pra receber os dados na RAM 


// Aqui é definido o que fazer quando a interrupção for disparada
void IRAM_ATTR gpio_isr_handler(void *arg){
    int64_t now_us = esp_timer_get_time();
    xQueueSendFromISR(gpio_evt_queue, &now_us, NULL);
}

void button_task_consumidora(void *arg){
    int64_t event_time_us;
    int64_t last_event_us = 0;

    int64_t delta_ms = 0;
    while (1)
    {
        if(xQueueReceive(gpio_evt_queue, &event_time_us, portMAX_DELAY)){
            
            delta_ms = (event_time_us - last_event_us);
            last_event_us = event_time_us;
            printf("Evento detectada em %lld us\n", delta_ms);
        }
    }
    
}

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    gpio_evt_queue = xQueueCreate(10, sizeof(int64_t));         // Fila para receber os timestamp da interrupção

    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);          // Aqui eu aviso o hardware do pino pegar um sinal de interrupção para quando estiver na borda de descida.
                                                                // Só configura o "tipo de evento" que o pino deve monitorar(borad de descida) - mas não diz o que fazer quando isso acontecer
    
    gpio_install_isr_service(0);                                // Liga a central telefonica -  inicializa o sistema geral de interrupções de GPIO do esp-idf (precisa rodar uma vez só antes de resgistrar qualquer handler)
    
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, NULL);   // Registra quem atende o telefone do pino X tocar - Aqui é onde a função "IRAM_ATTR gpio_isr_handler()" finalmente é conectada ao pino específico

    // OBS! 
    // São 3 coisas juntas (o trio indispensável)
    // Escrever a função - "IRAM_ATTR gpio_isr_handler()"
    // Ligar o serviço   - "gpio_install_isr_handler_add(botao, regra de negócio para o botão)", inicializa o sistema geral de interrupçoes, 
    // Registrar a função específica para o pino específico - "gpio_isr_handler_add(pino, regra de negocio, arg)" -> conecta a função ao pino específico

    // Criar a task que irá consumir o que está na fila
    xTaskCreate(button_task_consumidora, "button_task_consumer", 2048, NULL, 10, NULL);

    while (1)
    {
        printf("estou aqui\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

}