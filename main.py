import pygame

GRID_SIZE = 8
CELL_SIZE = 80
WINDOW_SIZE = GRID_SIZE * CELL_SIZE
FPS = 60

OCEAN_BLUE_COLOR = (37, 139, 207)
GRID_LINE_COLOR = (18, 86, 140)
ROCK_COLOR = (88, 88, 88)
SHIP_COLOR = (139, 69, 19) 
MINE_COLOR = (28, 28, 28)
COINS_COLOR = (255, 255, 0)



OBSTACLES = {
    (1, 2),
    (2, 5),
    (3, 3),
    (4, 6),
    (5, 1),
    (6, 4),
}

MINES = {
    (5, 5),
    (1, 1),
    (1, 3),
    (6, 5)
}

COINS = {
    (2, 2),
    (4, 4)
}

# WASD = (direcao q move em X, direcao q move em Y)
MOVE_BY_KEY = {
    pygame.K_w: (0, -1),
    pygame.K_s: (0, 1),
    pygame.K_a: (-1, 0),
    pygame.K_d: (1, 0),
}

colected_coins = 0

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


#coleta moedas

#def coleta_moedas()

def main():
    global colected_coins
    pygame.init()
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Nacio Microcontroladores")


    fonte = pygame.font.SysFont(None, 36)

    clock = pygame.time.Clock()

    ship_col = 0
    ship_row = 0
    running = True

    while running:
        # display_surface.fill(white)
        # display_surface.blit(text, textRect)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            if event.type == pygame.KEYDOWN :
                
                if event.key in MOVE_BY_KEY:
                    delta_col, delta_row = MOVE_BY_KEY[event.key]
                    next_col = ship_col + delta_col
                    next_row = ship_row + delta_row

                    if valid_position(next_col, next_row):
                        ship_col = next_col
                        ship_row = next_row
                        
                        if (ship_col, ship_row) in MINES:
                            print("EXPLODIU!!!!")
                        elif(ship_col, ship_row) in COINS:
                            colected_coins +=1
                            print("COLETOU MOEDA!")
                            print(colected_coins)
                            COINS.remove((ship_col,ship_row))
                
                elif event.key == pygame.K_SPACE:
                    print('leu a barra de espaco\n')
                    desarma_bomba(ship_col, ship_row)

        
        desenha_tab(screen)
        desenha_pedra(screen)
        desenha_navio(screen, ship_col, ship_row)
        desenha_mina(screen)
        desenha_moeda(screen)

        texto = fonte.render(f"Moedas: {colected_coins}", True, (255, 255, 0))
        screen.blit(texto, (10, 10))
        
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == "__main__":
    main()
