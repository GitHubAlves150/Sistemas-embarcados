## Clocks, Timers e Watchdog.

## Exercício 2.1:
- Consulte a frequência atual da CPU via esp_clk_cpu_freq() e imprima no boot.

```bash
⚠️ Nota de versão: o ESP-IDF mudou a API de clocks algumas vezes entre versões. Se seu IDF for mais antigo (4.x), a forma clássica é #include "esp_clk.h" e chamar esp_clk_cpu_freq(), que retorna o Hz diretamente. Se der erro de "função não encontrada", me manda a versão (idf.py --version) que eu ajusto o código para sua versão exata.
``` 