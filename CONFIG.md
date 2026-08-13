## Configuração inicial.

- Sistema operacional Linux(fuja do windowns para desenvolvimento)
- Instalar o docker (não o desctop)
- Criar um repositório no github e clonar na sua máquina(Deixar ela sincronizada com seu gitHub)
- Dentro do seu repositório baixe a imagem oficial da espressif ```docker pull espressif/idf:latest``` 
- Identifica a porta em que o esp32 está conectada no seu linux
- Puxar a imagen oficial da espressif
```docker pull espressif/idf:latest```

- Criar o container(Atenção!!!)

```bash
docker run -dit \
  --name esp32-dev \
  -v ~/Documents/Sistemas-embarcados:/project \ # Aqui eu coloquei o caminho do meu repositório sincronizado com meu github - faça com o seu
  -w /project \
  --device=/dev/ttyACM0 \ # Mude para sua porta
  espressif/idf bash

  ```
- Dentro do container rode os comandos abaixo para poder usar o idf.py 
```. $IDF_PATH/export.sh```
- Ou dependendo da versão da imagem ```source /opt/esp/idf/export.sh```
- ```idf.py --version```
- Ou rode ``` echo 'source /opt/esp/idf/export.sh' >> ~/.bashrc``` para ficar permanente

## Vamos testar o hello word!
Entre no seu container e copie o exemplo que vem junto na imagem para dentro da sua pasta de projetos.
```cp -r /opt/esp/idf/examples/get-started/hello_world/* hello_world/```
```cd hello_world ``` 
- Defina o target do dsipositivo ```idf.py set-target esp32``` 
- Compile ``` idf.py build```
- Grave ``` idf.py -p /dev/ttyACM0 flash```
- Monitora ```idf.py -p /dev/ttyACM0 monitor```

