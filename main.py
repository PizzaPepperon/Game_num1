import pygame

# Инициализируем pygame
pygame.init()

# Создаем окно игры
screen = pygame.display.set_mode((800, 600))

# Создаем персонажа игрока
player = pygame.Rect(100, 100, 50, 50)

# Создаем список блоков
blocks = []
for x in range(0, 800, 50):
    blocks.append(pygame.Rect(x, 400, 50, 50))

# Цикл игры
running = True
while running:
    # Обновляем события
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Обновляем состояние игрока
    if pygame.key.get_pressed()[pygame.K_UP]:
        player.y -= 5
    if pygame.key.get_pressed()[pygame.K_DOWN]:
        player.y += 5

    # Проверяем столкновения игрока с блоками
    for block in blocks:
        if player.colliderect(block):
            running = False

    # Рисуем экран
    screen.fill((0, 0, 0))
    for block in blocks:
        pygame.draw.rect(screen, (255, 255, 255), block)
    pygame.draw.rect(screen, (255, 0, 0), player)
    pygame.display.flip()

# Завершаем pygame
pygame.quit()
