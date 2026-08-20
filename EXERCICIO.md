![alt text](img/Temporizador.png)

## Clocks, Timers e Watchdog

### Exercício 2.3:
- Habilite o Task Watchdog Timer (TWDT) na task principal e force, de propósito, um travamento (`while(1);` sem yield) para observar o reset e o log de pânico. Depois, corrija o código para alimentar o watchdog corretamente.

---

# Documentação: configurando o menuconfig e analisando o funcionamento do watchdog

## Introdução
Em computação e sistemas embarcados, o watchdog é um temporizador que reinicia automaticamente um sistema quando o software ou o hardware trava.

Esse mecanismo previne o travamento de dispositivos, evitando que fiquem congelados indefinidamente sem resposta humana. É vital também em equipamentos remotos ou de difícil acesso (como satélites, sondas espaciais ou dispositivos IoT), onde o reset manual é impossível. Por fim, este sistema de reset restaura a normalização do sistema em milissegundos após detectar falhas de processamento.

O programa abaixo mostra um exemplo simples do funcionamento do watchdog configurado no `menuconfig`. Devido à falta de comandos que cedam tempo de CPU dentro do loop infinito, o ESP32 trava. Porém, com o watchdog habilitado, o sistema reinicia automaticamente toda vez que a falha ocorre. Analise o código abaixo:

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_clk_tree.h" // API atual da árvore de clocks (substitui esp_clk.h em versões recentes do IDF)

void app_main(void)
{
    printf("Iniciando.... O watchdog padrão (idle task) está ativo por padrão.\n");
    printf("Vou travar de propósito em 3 segundos.\n");

    vTaskDelay(pdMS_TO_TICKS(3000));

    printf("Travando agora (loop infinito sem yield)...\n");

    // ATENÇÃO: isso nunca cede o processador -> idle task nunca roda -> watchdog dispara
    while (1)
    {
        // Travamento intencional
    }
}
```

## Conclusão
Para habilitar o watchdog no ESP32, deve-se entrar no menu de configuração digitando `idf.py menuconfig`, navegar até **Component config -> ESP System Settings** e marcar a opção correspondente que consta na imagem abaixo.

![alt text](img/menuconfig.png)

---

# Para esta brach.

Imagine que o ESP32 é um operário trabalhando dentro de um laboratório perigoso e o **Watchdog** é um guarda de segurança que fica do lado de fora monitorando por uma janela. 

* **O Acordo:** O operário promete que, a cada **10 segundos**, ele vai até a janela e acena para o guarda (`esp_task_wdt_reset`), provando que está acordado e bem.
* **A Regra:** Se o operário desmaiar, ficar preso em uma máquina ou esquecer de acenar por mais de 10 segundos, o guarda assume o pior. Ele não tenta entrar: ele simplesmente **desliga o disjuntor geral e reinicia o laboratório inteiro** (`Reboot`) para garantir a segurança.

No código abaixo, o operário tenta tirar uma soneca de **19 segundos** (`vTaskDelay(19000)`). Como 19 segundos é maior que o combinado de 10 segundos, o guarda vai reiniciar o sistema antes mesmo de o operário acordar para acenar!

# Documentação de Projeto: Gerenciamento de Task Watchdog (TWDT)

**Autor:** Lucas Lorenço Alves  
**Curso:** Engenharia da Computação  
**Ano:** 2024  
**Plataforma:** ESP32 / ESP-IDF  

---

## 🪵 A Analogia do Leigo: O Guarda do Labirinto

Imagine que o ESP32 é um operário trabalhando dentro de um laboratório perigoso e o **Watchdog** é um guarda de segurança que fica do lado de fora monitorando por uma janela. 

* **O Acordo:** O operário promete que, a cada **10 segundos**, ele vai até a janela e acena para o guarda (`esp_task_wdt_reset`), provando que está acordado e bem.
* **A Regra:** Se o operário desmaiar, ficar preso em uma máquina ou esquecer de acenar por mais de 10 segundos, o guarda assume o pior. Ele não tenta entrar: ele simplesmente **desliga o disjuntor geral e reinicia o laboratório inteiro** (`Reboot`) para garantir a segurança.

No código abaixo, o operário tenta tirar uma soneca de **19 segundos** (`vTaskDelay(19000)`). Como 19 segundos é maior que o combinado de 10 segundos, o guarda vai reiniciar o sistema antes mesmo de o operário acordar para acenar!

---

## 💻 Código Comentado Linha por Linha

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_clk_tree.h" 
#include "esp_task_wdt.h" // <--- Biblioteca que traz os comandos do Watchdog
```
* **Importância:** Importa as ferramentas necessárias. A `esp_task_wdt.h` ativa as funções de controle do "Cão de Guarda" de tarefas do ESP-IDF.

```c
void app_main(void)
{
    // 1. Criando a ficha de configuração do Watchdog
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 10000,    // Tempo limite de 10 segundos antes do reset
        .idle_core_mask = 0,    // Não monitora as tarefas ociosas padrões do sistema
        .trigger_panic = true   // Se estourar o tempo, gera um erro crítico e reinicia
    };
```
* **Importância:** Define as regras do jogo. Criamos uma estrutura que diz ao hardware qual será o tempo máximo de tolerância antes de resetar o chip.

```c
    // 2. Aplica a nova configuração no chip
    esp_task_wdt_reconfigure(&twdt_config);
```
* **Importância:** O ESP32 já vem com um Watchdog padrão de fábrica. Esta linha diz: *"Esqueça o padrão, use a minha nova regra de 10 segundos"*.

```c
    // 3. Registra esta tarefa específica (app_main) no sistema de monitoramento
    esp_task_wdt_add(NULL);
```
* **Importância:** Passar `NULL` significa que a própria função atual (`app_main`) está entrando na lista de tarefas vigiadas pelo guarda. A partir daqui, ela **obrigatoriamente** precisa se identificar no tempo correto.

```c
    printf("Task registrada no watchdog (time 10s). Alimentando a cada 1s.\n");

    int ciclo = 0;
    while (1)
    {
        printf("Ciclo %d de trabalho .....\n", ciclo++);
        
        // 4. Pausa/Simulação de processamento pesado
        vTaskDelay(pdMS_TO_TICKS(19000)); 
```
* **Análise Técnica:** Esta linha coloca a tarefa para dormir por 19 segundos. Como o limite do Watchdog é de 10 segundos, o chip sofrerá um **Reset por Hardware** bem no meio desta linha, impedindo que o código abaixo dela seja executado. *(Nota: Para o código rodar para sempre sem resetar, esse valor deveria ser menor que 10000, como 1000 ms).*

```c
        // 5. Alimenta o Watchdog (Aviso de "Estou vivo!")
        esp_err_t err =  esp_task_wdt_reset();
        
        // 6. Validação de segurança
        if(err != ESP_OK){
            printf("ATENÇÃO: Falha ao resetar watchdog! err=%d\n", err);
        }
    }
}
```
* **Importância:** O `esp_task_wdt_reset()` zera o cronômetro do Watchdog. Guardamos o resultado na variável `err` para verificar se o comando funcionou com sucesso, uma prática essencial em softwares industriais de alta confiabilidade.

---

## 🛠️ Importância Prática na Engenharia da Computação

Em sistemas comerciais (como roteadores, centrais automotivas ou dispositivos médicos), os códigos rodam por meses ou anos seguidos sem supervisão humana. 

Se ocorrer um **ruído elétrico**, uma **falha de memória** ou um **travamento de rede**, o software pode travar. O Watchdog garante que o dispositivo se recupere sozinho em segundos, eliminando a necessidade de um operador técnico ir até o local para desligar e ligar o aparelho na tomada.

---

# Arquitetura e Aplicação de Watchdog Timers (WDT) no ESP32. A diferença do watchdog via firmware e via menuconfig

---

## 1. Introdução

No desenvolvimento de sistemas embarcados de alta confiabilidade (*mission-critical* ou aplicações industriais), a resiliência do hardware e do software é um requisito fundamental. Dispositivos baseados no microcontrolador ESP32 frequentemente operam de forma isolada em campo, sujeitos a intempéries, flutuações de energia e ruídos eletromagnéticos. 

Para garantir que o sistema se recupere autonomamente de travamentos sem a necessidade de intervenção humana física (como pressionar um botão de *Reset*), o ecossistema do ESP32 implementa múltiplos mecanismos de **Watchdog Timer (WDT)**. 

Este documento analisa a divisão arquitetural desses temporizadores, diferenciando a atuação em nível de hardware/sistema e em nível de firmware/tarefa.

---

## 2. Desenvolvimento: A Dupla Camada de Proteção

A arquitetura do ESP32 não depende de um único "Cão de Guarda", mas sim de uma estratégia de proteção em camadas. Eles dividem-se principalmente em duas categorias de atuação:

### 2.1. Watchdog do Sistema e Hardware Puro (MWDT e RTCWDT)
Esta camada opera no nível mais baixo do silício. São circuitos integrados diretamente no hardware do chip, projetados para serem robustos e independentes do estado do sistema operacional.

* **Main Watchdog Timer (MWDT):** Integrado aos módulos de temporizadores do sistema. Monitora falhas críticas globais, como interrupções travadas que impedem o escalonador do sistema operacional de funcionar.
* **RTC Watchdog Timer (RTCWDT):** Localizado no coprocessador de ultra baixo consumo (RTC). É a última linha de defesa do ESP32. Ele continua ativo mesmo se o clock principal da CPU parar ou se o chip entrar em modos de *Deep-sleep*.
* **Gerenciamento:** Sua ativação e parâmetros básicos (como tempo de timeout do boot) são configurados de forma global e estática através do **Menuconfig** (`idf.py menuconfig`), sendo inicializados automaticamente pelo próprio ecossistema do ESP-IDF antes da execução do código do usuário.
* **Foco de Atuação:** Falhas catastróficas de hardware, picos de tensão (brownouts), corrupção de memória por estática ou travamentos no processo de inicialização (*bootloader*).

### 2.2. Watchdog de Tarefas (TWDT - Task Watchdog Timer)
Esta camada opera em nível de software e gerenciamento de processos, trabalhando diretamente integrada ao sistema operacional de tempo real (**FreeRTOS**).

* **Funcionamento:** O TWDT monitora se tarefas específicas do FreeRTOS estão recebendo tempo de processamento ou se ficaram presas em loops de execução.
* **Flexibilidade via Firmware:** Diferente dos watchdogs de hardware puro, o TWDT permite que o desenvolvedor interaja dinamicamente via código. Através de funções como `esp_task_wdt_add()`, uma função criada pelo usuário (como a `app_main`) pode voluntariamente entrar na lista de vigilância do sistema.
* **Alimentação Dinâmica:** Uma vez registrada, a tarefa assume a obrigação de chamar a função `esp_task_wdt_reset()` dentro do seu prazo estipulado de *timeout*.
* **Foco de Atuação:** Falhas de lógica de programação, loops infinitos acidentais (`while(1)` sem liberação de CPU), ou funções de rede bloqueantes que congelam aguardando uma resposta de servidor que nunca chega.

---

### 2.3. Resumo Comparativo: Analogia Corporativa

Para fins didáticos, a interação entre essas duas camadas pode ser comparada à estrutura de segurança de uma grande empresa:

| Característica | TWDT (Task Watchdog) | RTCWDT / MWDT (Hardware) |
| :--- | :--- | :--- |
| **Analogia** | O Gerente de Projetos | O Segurança do Prédio |
| **Escopo** | Fiscaliza se os funcionários (*tasks*) estão produzindo e batendo o ponto no prazo. | Não olha a produção; fiscaliza se o prédio tem energia e se a estrutura física está de pé. |
| **Configuração** | Dinâmica (Via código/firmware). | Estática (Via Menuconfig/hardware). |
| **Causa do Reset** | Uma tarefa travou a CPU por muito tempo. | O processador principal desmaiou por um choque elétrico. |

---

## 3. Conclusão

A Engenharia de Computação exige o desenvolvimento de firmwares tolerantes a falhas. No ESP32, isso é alcançado combinando as duas filosofias de Watchdog:

Enquanto o **Menuconfig** nos permite estabelecer as defesas físicas básicas do chip contra distúrbios elétricos e falhas de inicialização, a manipulação do **TWDT via Firmware** confere ao desenvolvedor o poder de ditar as regras de saúde do seu próprio algoritmo. 

Projetar sistemas onde as tarefas críticas são registradas no Watchdog com uma margem de segurança adequada (com o tempo de *timeout* consideravelmente maior do que o tempo de ciclo de trabalho) é o diferencial que separa um protótipo de bancada de um produto comercial robusto de padrão industrial.
