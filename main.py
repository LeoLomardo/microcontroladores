import pygame


GRID_SIZE = 8
CELL_SIZE = 80
WINDOW_SIZE = GRID_SIZE * CELL_SIZE
FPS = 60

OCEAN_BLUE = (37, 139, 207)
GRID_LINE = (18, 86, 140)
ROCK = (88, 88, 88)
SHIP = (192, 42, 42)
MINE = (28, 28, 28)

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
}

# WASD = (direcao q move em X, direcao q move em Y)
MOVE_BY_KEY = {
    pygame.K_w: (0, -1),
    pygame.K_s: (0, 1),
    pygame.K_a: (-1, 0),
    pygame.K_d: (1, 0),
}


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

def desenha_tab(screen):
    for row in range(GRID_SIZE):
        for col in range(GRID_SIZE):
            color = OCEAN_BLUE
            pygame.draw.rect(screen, color, desenha_ret(col, row))
            pygame.draw.rect(screen, GRID_LINE, desenha_ret(col, row), width=1)


def desenha_pedra(screen):
    for col, row in OBSTACLES:
        rect = desenha_ret(col, row, padding=12)
        pygame.draw.rect(screen, ROCK, rect, border_radius=8)


#isso aqui eu pedi pro gpt fazer
def desenha_mina(screen):
    for col, row in MINES:
        mine_rect = desenha_ret(col, row, padding=18)
        center_x, center_y = mine_rect.center
        radius = mine_rect.width // 2
        spike = 8

        pygame.draw.line(
            screen,
            MINE,
            (center_x - radius - spike, center_y),
            (center_x + radius + spike, center_y),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE,
            (center_x, center_y - radius - spike),
            (center_x, center_y + radius + spike),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE,
            (center_x - radius, center_y - radius),
            (center_x + radius, center_y + radius),
            width=4,
        )
        pygame.draw.line(
            screen,
            MINE,
            (center_x - radius, center_y + radius),
            (center_x + radius, center_y - radius),
            width=4,
        )
        pygame.draw.circle(screen, MINE, mine_rect.center, radius)


def desenha_navio(screen, col, row):
    ship_rect = desenha_ret(col, row, padding=16)
    pygame.draw.rect(screen, SHIP, ship_rect, border_radius=6)

    cabin_rect = pygame.Rect(0, 0, ship_rect.width // 2, ship_rect.height // 3)
    cabin_rect.center = ship_rect.center


def main():
    pygame.init()
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Nacio Microcontroladores")
    clock = pygame.time.Clock()

    ship_col = 0
    ship_row = 0
    running = True

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            if event.type == pygame.KEYDOWN and event.key in MOVE_BY_KEY:
                delta_col, delta_row = MOVE_BY_KEY[event.key]
                next_col = ship_col + delta_col
                next_row = ship_row + delta_row

                if valid_position(next_col, next_row):
                    ship_col = next_col
                    ship_row = next_row

        desenha_tab(screen)
        desenha_pedra(screen)
        desenha_navio(screen, ship_col, ship_row)
        desenha_mina(screen)
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()


if __name__ == "__main__":
    main()
