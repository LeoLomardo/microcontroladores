# microcontroladores
# Jogo de Programação com Arduino e Pygame

Projeto desenvolvido durante o curso de Projeto de Microcontroladores na PUC-Rio. O sistema integra hardware e software para criar um jogo interativo onde a interface de controle é feita através de blocos físicos de lógica.

O desenvolvimento envolveu scripts em Python para a interface gráfica (Pygame) e comunicação serial, além de lógica em C para o controle dos microcontroladores.

## Como funciona

O jogador precisa resolver fases de um jogo digital montando a lógica de movimento em um tabuleiro físico. 

1. O usuário insere blocos de comando no tabuleiro.
2. Um Arduino faz a leitura da posição de cada bloco e transmite essa configuração.
3. Um segundo Arduino recebe a configuração e desenha os blocos em um display. Ele também aciona uma fita de LEDs e um buzzer, emitindo cores e sons diferentes para cada tipo de comando lido.
4. O script em Pygame no computador processa esses movimentos, atualiza o personagem na tela e verifica se a fase foi vencida ou perdida.

## Hardware utilizado

- 2 placas Arduino
- Display para interface
- Fita de LEDs
- Buzzer passivo
- Fios e resistores para o tabuleiro e os blocos
- Blocos e tabuleiro impressos em 3D

## Estrutura do repositório

- codigo_arduino_Walter/: Código C do Arduino responsável pelo display, LEDs e som.
- levels/: Arquivo JSON com a configuração das fases do jogo.
- teorias/: Materiais de apoio e anotações do projeto.
- main.py: Arquivo principal do jogo em Pygame.
- controle_serial.py: Gerencia a comunicação via porta serial entre o PC e o hardware.
- simula_arduino.py: Script para simular os comandos e testar o jogo sem o hardware conectado.
- interpretacomandos.cpp: Lógica de processamento e interpretação dos blocos inseridos.
- requirements.txt: Dependências do projeto Python.

## Como rodar o projeto

Primeiro, certifique-se de que os códigos foram gravados nos Arduinos e que o hardware está conectado ao computador via USB.

Abra o terminal na pasta do projeto e instale as dependências do Python:
pip install -r requirements.txt

Caso necessário, ajuste a porta serial no arquivo de controle. Depois, inicie o jogo com o comando:
python main.py
