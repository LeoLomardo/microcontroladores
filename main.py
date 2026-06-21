from pathlib import Path
import os
import json
import pygame
import tkinter as tk
from tkinter import ttk

import controle_serial

GRID_SIZE = 8
CELL_SIZE = 80
WINDOW_SIZE = GRID_SIZE * CELL_SIZE
FPS = 60

LEVEL_FOLDER = "./levels"
LEVEL_NAME_JSON = "fases.json"
LEVEL_FILE = Path(__file__).parent / LEVEL_FOLDER / LEVEL_NAME_JSON

# Porta onde o Arduino aparece. No Linux costuma ser /dev/ttyUSB0 ou
# /dev/ttyACM0; no Windows seria algo como "COM3". Para testar sem o Arduino,
# da pra apontar para uma porta virtual: PORTA_SERIAL=/tmp/ttyGAME python main.py
PORTA_SERIAL = os.environ.get("PORTA_SERIAL", "/dev/ttyUSB0")

OCEAN_BLUE_COLOR = (37, 139, 207)
GRID_LINE_COLOR = (18, 86, 140)
ROCK_COLOR = (88, 88, 88)
SHIP_COLOR = (139, 69, 19) 
MINE_COLOR = (28, 28, 28)
COINS_COLOR = (255, 255, 0)
START_COLOR = (30, 160, 90)
FINISH_COLOR = (210, 60, 60)
TEXT_COLOR = (255, 255, 255)



OBSTACLES = set()
MINES = set()
COINS = set()

# WASD = (direcao q move em X, direcao q move em Y)
MOVE_BY_KEY = {
    pygame.K_w: (0, -1),
    pygame.K_s: (0, 1),
    pygame.K_a: (-1, 0),
    pygame.K_d: (1, 0),
}

colected_coins = 0


def abrir_menu(caminho):
    with caminho.open("r", encoding="utf-8") as arquivo:
        dados = json.load(arquivo)

    fases = dados["fases"]
    fase_escolhida = {"fase": None}

    janela = tk.Tk()
    janela.title("Escolha a fase")
    janela.geometry("300x150")

    nomes_fases = []

    for fase in fases:
        nomes_fases.append(fase["nome"])

    label = tk.Label(janela, text="Escolha uma fase:")
    label.pack()

    dropdown = ttk.Combobox(janela, values=nomes_fases)
    dropdown.pack()

    def iniciar():
        nome_selecionado = dropdown.get()

        for fase in fases:
            if fase["nome"] == nome_selecionado:
                fase_escolhida["fase"] = fase
                break

        janela.destroy()

    botao = tk.Button(janela, text="Iniciar", command=iniciar)
    botao.pack()

    janela.mainloop()

    return fase_escolhida["fase"]

def carregar_level(fase):

    linhas = fase["mapa"]

    if not linhas:
        raise ValueError(f"Fase com esse nome nao foi encontrada")

    if len(linhas) != GRID_SIZE:
        raise ValueError(f"O level precisa ter {GRID_SIZE} linhas.")



    obstacles = set()
    mines = set()
    coins = set()
    start_position = None
    finish_position = None

    for row, linha in enumerate(linhas):
        if len(linha) != GRID_SIZE:
            raise ValueError(f"Tamanho das linhas invalido")

        for col, simbolo in enumerate(linha):
            position = (col, row)

            if simbolo == ".":
                continue

            if simbolo == "#":
                obstacles.add(position)

            elif simbolo == "M":
                mines.add(position)

            elif simbolo == "C":
                coins.add(position)

            elif simbolo == "S":
                if start_position is not None:
                    raise ValueError("Ponto de partida S ja defindo")

                start_position = position

            elif simbolo == "F":
                if finish_position is not None:
                    raise ValueError("Destino F já definido")

                finish_position = position
                
            else:
                raise ValueError(f"Simbolo invalido no level: {simbolo}")

    if start_position is None:
        raise ValueError("Faltou definir ponto de partida S")

    if finish_position is None:
        raise ValueError("Faltou definir ponto de chegada F")

    return start_position, finish_position, obstacles, mines, coins, len(coins)


def valid_position(col, row):
    if col < 0 or col >= GRID_SIZE:
        return False

    if row < 0 or row >= GRID_SIZE:
        return False

    if (col, row) in OBSTACLES:
        return False

    return True

def desenha_ret(col, row, padding=0):
    return pygame.Rect(
        col * CELL_SIZE + padding,
        row * CELL_SIZE + padding,
        CELL_SIZE - padding * 2,
        CELL_SIZE - padding * 2,
    )

def obter_dados_circulo(col, row, padding=0):
    tamanho_util = CELL_SIZE - padding * 2
    raio = tamanho_util // 2
    
    centro_x = col * CELL_SIZE + padding + raio
    centro_y = row * CELL_SIZE + padding + raio
    
    return (centro_x, centro_y), raio

def desenha_tab(screen):
    for row in range(GRID_SIZE):
        for col in range(GRID_SIZE):
            color = OCEAN_BLUE_COLOR
            pygame.draw.rect(screen, color, desenha_ret(col, row))
            pygame.draw.rect(screen, GRID_LINE_COLOR, desenha_ret(col, row), width=1)

def desenha_pedra(screen):
    for col, row in OBSTACLES:
        rect = desenha_ret(col, row, padding=12)
        pygame.draw.rect(screen, ROCK_COLOR, rect, border_radius=8)

def desenha_moeda(screen):
    for col, row in COINS:
        centro, raio = obter_dados_circulo(col, row, padding=12)
        pygame.draw.circle(screen, COINS_COLOR, centro, raio)

def desenha_ponto_level(screen, fonte, position, color, letra):
    col, row = position
    rect = desenha_ret(col, row, padding=12)
    pygame.draw.rect(screen, color, rect, border_radius=8)

    texto = fonte.render(letra, True, TEXT_COLOR)
    texto_rect = texto.get_rect(center=rect.center)
    screen.blit(texto, texto_rect)

#isso aqui eu pedi pro gpt fazer, falta conhecimento artistico
def desenha_mina(screen):
    for col, row in MINES:
        mine_COLOR_rect = desenha_ret(col, row, padding=18)
        center_x, center_y = mine_COLOR_rect.center
        radius = mine_COLOR_rect.width // 2
        spike = 16

        pygame.draw.line(
            screen,
            MINE_COLOR,
            (center_x - radius - spike, center_y),
            (center_x + radius + spike, center_y),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE_COLOR,
            (center_x, center_y - radius - spike),
            (center_x, center_y + radius + spike),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE_COLOR,
            (center_x - radius, center_y - radius),
            (center_x + radius, center_y + radius),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE_COLOR,
            (center_x - radius, center_y + radius),
            (center_x + radius, center_y - radius),
            width=4,
        )
        pygame.draw.circle(screen, MINE_COLOR, mine_COLOR_rect.center, radius)

def desenha_navio(screen, col, row):
    ship_COLOR_rect = desenha_ret(col, row, padding=16)
    pygame.draw.rect(screen, SHIP_COLOR, ship_COLOR_rect, border_radius=6)

def desarma_bomba(ship_col, ship_row):
    minas_para_remover = []
    
    for delta_col in (-1, 0, 1):
        for delta_row in (-1, 0, 1):
            check_col = ship_col + delta_col
            check_row = ship_row + delta_row
            
            #refatorar colocando LINHA/COLUNA
            if (check_col, check_row) in MINES:
                minas_para_remover.append((check_col, check_row))
                print(f"Mina desarmada na posição: ({check_col}, {check_row})")

    for mina in minas_para_remover:
        MINES.remove(mina)


def aplica_movimento(ship_col, ship_row, delta_col, delta_row, finish_position, total_coins, mensagem_atual):
    global colected_coins

    next_col = ship_col + delta_col
    next_row = ship_row + delta_row

    if not valid_position(next_col, next_row):
        return ship_col, ship_row, mensagem_atual

    ship_col = next_col
    ship_row = next_row
    mensagem = ""

    if (ship_col, ship_row) in MINES:
        mensagem = "EXPLODIU!!!!"
        print(mensagem)
    elif (ship_col, ship_row) in COINS:
        colected_coins += 1
        mensagem = "COLETOU MOEDA!"
        print(mensagem)
        print(colected_coins)
        COINS.remove((ship_col, ship_row))
    elif (ship_col, ship_row) == finish_position:
        if colected_coins == total_coins:
            mensagem = "CHEGOU AO DESTINO!"
        else:
            mensagem = "Falta coletar moedas"
        print(mensagem)

    return ship_col, ship_row, mensagem

def main(fase):
    global OBSTACLES, MINES, COINS, colected_coins

    start_position, finish_position, OBSTACLES, MINES, COINS, total_coins = carregar_level(fase)
    colected_coins = 0

    pygame.init()
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Nacio Microcontroladores")


    fonte = pygame.font.SysFont(None, 36)

    clock = pygame.time.Clock()

    ship_col, ship_row = start_position
    mensagem = ""
    running = True

    controle_serial.conectar(PORTA_SERIAL)

    while running:
        # display_surface.fill(white)
        # display_surface.blit(text, textRect)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            if event.type == pygame.KEYDOWN :
                
                if event.key in MOVE_BY_KEY:
                    delta_col, delta_row = MOVE_BY_KEY[event.key]
                    ship_col, ship_row, mensagem = aplica_movimento(
                        ship_col, ship_row, delta_col, delta_row,
                        finish_position, total_coins, mensagem)

                elif event.key == pygame.K_SPACE:
                    print('leu a barra de espaco\n')
                    desarma_bomba(ship_col, ship_row)

        # comandos vindos do Arduino pela serial (ex.: "2 ESQUERDA", "3 ABAIXO")
        for instrucao in controle_serial.ler_instrucoes():
            if instrucao[0] == "FIM":
                print("Arduino enviou 'Fim' - sequencia de comandos encerrada.")
            elif instrucao[0] == "MOVER":
                _, delta_col, delta_row, passos = instrucao
                # "2 ESQUERDA" = andar 2 casas, uma de cada vez, para passar por
                # cada celula (coletando moedas / pisando em minas no caminho).
                for _ in range(passos):
                    ship_col, ship_row, mensagem = aplica_movimento(
                        ship_col, ship_row, delta_col, delta_row,
                        finish_position, total_coins, mensagem)

        desenha_tab(screen)
        desenha_ponto_level(screen, fonte, start_position, START_COLOR, "S")
        desenha_ponto_level(screen, fonte, finish_position, FINISH_COLOR, "F")
        desenha_pedra(screen)
        desenha_navio(screen, ship_col, ship_row)
        desenha_mina(screen)
        desenha_moeda(screen)

        texto = fonte.render(f"Moedas: {colected_coins}", True, (255, 255, 0))
        screen.blit(texto, (10, 10))

        if mensagem:
            texto_mensagem = fonte.render(mensagem, True, TEXT_COLOR)
            screen.blit(texto_mensagem, (10, WINDOW_SIZE - 40))
        
        pygame.display.flip()
        clock.tick(FPS)

    controle_serial.fechar()
    pygame.quit()


if __name__ == "__main__":
    fase = abrir_menu(LEVEL_FILE)

    main(fase)
