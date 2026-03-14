Escrita
Aulimentador

Projeto de alimentador automático para pets utilizando ESP32.

A ideia do projeto foi criar um sistema simples onde é possível definir a quantidade de ração que será liberada remotamente através de um aplicativo. A comunicação entre o app e o dispositivo é feita via MQTT.

O ESP32 recebe a quantidade de gramas enviada pelo aplicativo e calcula o tempo necessário para abrir o servo motor, liberando a ração.

Tecnologias usadas

ESP32

MQTT

WiFi

Bluetooth (configuração do WiFi)

MIT App Inventor

Servo Motor

Como funciona

O usuário envia a quantidade de ração pelo aplicativo

O app publica essa informação em um tópico MQTT

O ESP32 recebe o valor

O dispositivo calcula o tempo de abertura do servo com base em uma taxa de gramas por segundo

A ração é dispensada

Estrutura do projeto

esp32/ → código do ESP32
app/ → arquivo do aplicativo (.aia)
docs/ → documentação do projeto
imagens/ → fotos do protótipo

MQTT

Tópicos utilizados no projeto:

Esp32/GramasEnvio
Esp32/HorarioEnvio
Esp32/PesoAtual
Esp32/UltimoPeso
Esp32/UltimoHorario
Esp32/Status

Observação

Esse projeto foi desenvolvido como estudo do projeto da faculcade com IoT e sistemas embarcados, focando em comunicação entre dispositivos e controle remoto de hardware.
