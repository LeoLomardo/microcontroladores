# Jogo de Programação Tangível com Arduino e Pygame

Projeto desenvolvido na disciplina de **Projeto de Microcontroladores (PUC-Rio)**. O sistema é uma **Tangible User Interface (TUI)**: em vez de teclado ou joystick, o jogador programa os movimentos de um navio encaixando **blocos físicos de lógica** (setas, loops, condicionais e números) em um tabuleiro eletrônico. Dois Arduinos leem, validam e executam esse "programa físico", enquanto um jogo em **Pygame** no computador renderiza o mundo marítimo e aplica as regras.

O objetivo final é pedagógico: ensinar conceitos de programação (sequência, repetição, condicional) de forma concreta e manipulável, integrando **eletrônica analógica, firmware embarcado (C/C++), comunicação serial e software de jogo (Python)** em um único ecossistema.

---

## 🎯 Objetivo do Jogo

Guiar um navio da posição inicial (**S**) até o destino (**F**) em uma matriz **8×8**, coletando **todas as moedas** da fase, sem colidir com pedras nem pisar em minas marítimas. A vitória só é reconhecida se o navio chegar em **F** com todas as moedas coletadas. Se os comandos do tabuleiro acabarem antes disso, o jogo exibe "FIM DOS COMANDOS"; se o navio atingir uma mina, é game over.

---

## 🧩 Visão Geral da Arquitetura

```
[ Tabuleiro de Blocos Físicos ]
      (divisores resistivos)
              │  analogRead A0–A15
              ▼
[ Arduino Leitor ]  ── Serial1 ──►  [ Arduino Display (Mega) ]
   interpretacomandos.cpp               codigo_arduino_Walter.ino
   • lê e valida a sintaxe              • display LCD TFT (matriz 2×8)
   • executa o programa físico          • fita NeoPixel (16 LEDs)
              │                         • buzzer (feedback sonoro)
              │ Serial (USB, 9600 baud)
              ▼
[ PC — Python ]
   main.py (Pygame)  +  controle_serial.py (pySerial não-bloqueante)
   • renderiza o mapa 8×8, colisões, moedas, minas
   • envia START / recebe movimentos, ACAO, FIM, ERRO
```

O fluxo completo de uma partida:

1. **Montagem:** o jogador insere blocos em uma matriz física **2×8** — a **Linha 1** define o tipo de comando (setas, LOOP, IF/VEZES, AÇÃO) e a **Linha 2** define parâmetros numéricos (quantidade de passos ou índice de retorno do loop).
2. **Leitura analógica:** cada casa do tabuleiro é um divisor resistivo. O Arduino Leitor faz 5 leituras por casa (`analogRead` em A0–A15), tira a média e classifica o valor em uma das 14 janelas de tensão do vetor `intervalosAnalog`, identificando qual bloco está encaixado.
3. **Espelhamento em tempo real:** a cada 600 ms o Leitor detecta mudanças no tabuleiro e envia deltas via `Serial1` para o Arduino Display, que redesenha a matriz 2×8 no display TFT, acende o LED correspondente na fita NeoPixel e toca frequências distintas no buzzer para cada tipo de bloco.
4. **Início:** o jogador escolhe a fase no menu (Tkinter) e o Pygame envia **`START`** pela porta serial. O Leitor congela a leitura, **valida a sintaxe** do programa físico (loops sem alvo, números órfãos e blocos em linha errada geram `ERRO`) e começa a execução.
5. **Execução:** a cada 1 segundo o Leitor consome um comando e envia ao PC strings como `3 CIMA` ou `1 ACAO`. O Arduino Display acompanha a execução com uma **bolinha azul de rastreio** sob a coluna ativa no display. O Pygame move o navio célula a célula, aplicando colisão, coleta de moedas e explosão de minas.
6. **Desfecho:** quando a pilha de comandos termina, o Leitor envia **`FIM`**. O Pygame decide entre vitória, derrota ou falta de comandos, e pode disparar `vitoria`/`derrota` de volta ao hardware — o Arduino Display pisca a fita em verde (com ✓ na tela) ou vermelho (com ✗), acompanhado de tons no buzzer.

---

## 🛠️ Hardware

* **Arduino Leitor** (`interpretacomandos.cpp`)
  * 16 entradas analógicas (**A0–A7** para comandos, **A8–A15** para quantidades).
  * Botão físico no pino digital **2**, gerenciado pela biblioteca **GFButton** — usado como condição do bloco **IF** (se pressionado, segue em frente; senão, salta para o índice indicado na Linha 2).
  * Comunica-se com o PC pela **Serial USB** e com o Walter pela **Serial1**, ambas a **9600 baud**.
* **Arduino Mega "Walter"** (`codigo_arduino_Walter.ino`)
  * **Display LCD TFT** via `MCUFRIEND_kbv` + `Adafruit_GFX`: desenha a matriz 2×8 de blocos com mini-símbolos (setas, círculo de loop, "X" do multiplicador, "!" da ação) e a bolinha de rastreio da coluna em execução.
  * **Fita NeoPixel de 16 LEDs** (`Adafruit_NeoPixel`) no pino **24** — cada coluna física do tabuleiro mapeia para o LED `coluna × 2`, com a cor do comando ativo.
  * **Buzzer passivo** no pino **26** — frequências de ~110 Hz a ~880 Hz distinguem movimentos, loops, erros, vitória e derrota.
  * Mantém uma **fila de execução** (`MAX_FILA = 100`): ao receber `iniciar`, expande loops (bloco VEZES clona o trecho desde o índice do LOOP) e movimentos multiplicados em instruções unitárias, exibidas uma a uma a cada 1 s.
* **Tabuleiro e blocos impressos em 3D** com resistores internos: cada bloco produz uma tensão característica lida pelo ADC.

<img width="3441" height="1757" alt="Diagrama_blocos_programacao_bb" src="https://github.com/user-attachments/assets/043370ae-b938-4e0d-9e92-e419c06888a1" />

---

## 🎛️ Blocos e IDs Lógicos

| ID | Bloco | Linha | Cor (LED/Display) | Função |
| :---: | :--- | :---: | :--- | :--- |
| `0` | VAZIO | 1 ou 2 | Apagado / contorno cinza | Casa sem bloco |
| `1–6` | UM…SEIS | 2 | Branco | Quantidade de passos, multiplicador do loop ou índice de retorno |
| `7` | ACAO | 1 | Roxo ("!") | Desarma minas no raio de 1 célula ao redor do navio |
| `8` | IF | 1 | — | Condicional: avança se o botão físico estiver pressionado, senão salta |
| `9` | LOOP | 1 | Laranja (círculo) | Marca o início de um trecho repetível; Linha 2 indica o índice de retorno |
| `4`* | VEZES | 1 | Branco ("X") | Multiplicador do loop anterior (na expansão de fila do Walter) |
| `10` | DIREITA | 1 | Azul (seta →) | Move o navio para leste |
| `11` | ESQUERDA | 1 | Azul (seta ←) | Move para oeste |
| `12` | TRAS / BAIXO | 1 | Verde (seta ↓) | Move para o sul |
| `13` | FRENTE / CIMA | 1 | Verde (seta ↑) | Move para o norte |

\* No protocolo de exibição do Walter, os tipos são remapeados para 1–7 (`frente`, `direita`, `loop`, `vezes`, `tras`, `esquerda`, `acao`).

**Validação de sintaxe** (feita no Leitor antes de executar): números não podem aparecer na Linha 1; comandos/LOOP/IF não podem aparecer na Linha 2; todo LOOP exige um número de retorno válido (que aponte para trás) e a casa seguinte vazia na Linha 1; IF exige número na Linha 2. Qualquer violação envia `ERRO` ao PC e aborta a execução — impedindo, por exemplo, loops que voltariam para frente de si mesmos.

---

## 📨 Protocolo Serial (texto plano, `\n`, 9600 baud)

**PC → Arduino Leitor**

| Mensagem | Efeito |
| :--- | :--- |
| `START` | Congela o tabuleiro, valida a sintaxe e inicia a execução |

**Arduino Leitor → PC**

| Mensagem | Efeito no Pygame |
| :--- | :--- |
| `<N> <DIRECAO>` (ex.: `3 CIMA`) | Move o navio N casas, uma por vez (coletando moedas/pisando em minas no caminho) |
| `<N> ACAO` | Desarma minas nas 8 células adjacentes |
| `FIM` | Fim da pilha de comandos → tela de falha se não venceu |
| `ERRO` | Sintaxe física inválida |

**Leitor → Walter (Serial1)** e **PC → Walter**

| Mensagem | Efeito |
| :--- | :--- |
| `ini <trios>` | Carga inicial completa do tabuleiro (trios `comando linha coluna`) |
| `<comando> <linha> <coluna>` (ex.: `frente 1 4`) | Insere/atualiza um bloco na matriz do display |
| `remover <linha> <coluna>` | Apaga um bloco (e o LED correspondente) |
| `iniciar` | Expande loops e monta a fila de execução visual |
| `limpar` | Zera a matriz, LEDs e tela |
| `vitoria` / `derrota` | Pisca verde+✓ / vermelho+✗ com tons no buzzer |

---

## 🐍 Software (Python)

### `main.py` — motor do jogo (Pygame)
* **Menu de fases em Tkinter** lendo `levels/fases.json`, com fluxo de reinício, próxima fase e retorno ao menu.
* **Mapa 8×8** carregado de strings de caracteres: `.` água, `#` pedra, `M` mina, `C` moeda, `S` início, `F` fim — com validação de dimensões e unicidade de S/F.
* **Colisão e coleta** com estruturas `set()` (`OBSTACLES`, `MINES`, `COINS`) para busca O(1): movimento contra pedra é anulado; mina aciona a tela de morte; a vitória exige todas as moedas + chegada em F.
* **Desarme (`ACAO`)**: varre o perímetro 3×3 ao redor do navio e remove qualquer mina encontrada.
* **Modo debug por teclado**: WASD move o navio e Espaço desarma minas, sem depender do hardware.
* Envia `START` automaticamente ~3 s após conectar à serial.

### `controle_serial.py` — camada serial não-bloqueante
Desenhada para que a comunicação com o hardware **não trave o FPS** do Pygame:
* `Serial(porta, baudrate, timeout=0)` → `read()` retorna imediatamente só o que já chegou.
* Buffer global acumula fragmentos (`decode("utf-8", errors="ignore")`) e extrai comandos completos por quebra de linha:

```python
while "\n" in _buffer:
    linha, _buffer = _buffer.split("\n", 1)
    instrucao = interpretar(linha)
```

* `interpretar()` converte o texto do Arduino em tuplas prontas para o loop do jogo:
  * `"FIM"` → `("FIM",)`
  * `"<N> ACAO"` → `("ACAO",)`
  * `"3 DIREITA"` → `("MOVER", 1, 0, 3)` — (ação, Δcol, Δrow, passos), via o dicionário `MOVIMENTO_POR_DIRECAO`.
  * Linhas malformadas são ignoradas com aviso, sem derrubar o jogo.

---

## 📂 Estrutura do Repositório

```
├── codigo_arduino_Walter/
│   └── codigo_arduino_Walter.ino   # Firmware do Mega: display TFT, NeoPixel, buzzer, fila de execução
├── interpretacomandos.cpp          # Firmware do Leitor: ADC, validação de sintaxe, execução do programa
├── levels/
│   └── fases.json                  # Fases (matrizes 8×8 com S, F, #, M, C, .)
├── main.py                         # Motor do jogo em Pygame + menu Tkinter
├── controle_serial.py              # Camada serial não-bloqueante (pySerial)
└── requirements.txt                # Dependências Python
```

---

## 🚀 Como Rodar

**Pré-requisitos:** os dois Arduinos gravados e conectados; identifique a porta (`COM*` no Windows, `/dev/ttyUSB*`/`/dev/ttyACM*` no Linux).

1. **Instale as dependências:**

```bash
pip install -r requirements.txt
```

2. **Configure a porta serial**dentro do main.py:


3. Escolha a fase no menu, monte seu programa no tabuleiro físico e acompanhe a execução no display, na fita de LEDs e na tela.

> **Sem hardware?** O jogo roda normalmente em modo debug: **WASD** movimenta o navio e **Espaço** desarma minas.

---

## 🔗 Integração entre as Áreas

| Área | Papel no projeto |
| :--- | :--- |
| **Eletrônica analógica** | Blocos com divisores resistivos → identificação por janelas de tensão no ADC |
| **Sistemas embarcados (C/C++)** | Máquinas de estado, temporização não-bloqueante (`millis()`), validação de linguagem, filas de execução |
| **Comunicação serial** | Protocolo textual próprio em três pontas (Leitor ↔ Walter ↔ PC) |
| **Computação gráfica / jogos (Python)** | Renderização, física de grade, estados de jogo e UX de fases |
| **Design/fabricação** | Blocos e tabuleiro impressos em 3D como interface tangível |

