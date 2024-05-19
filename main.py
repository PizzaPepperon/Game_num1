import pygame
import sys

# Инициализируем pygame
pygame.init()

# Константы
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
BLOCK_SIZE = 50
PLAYER_SIZE = 50
SPEED = 5

# Создаем окно игры
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Змейка")

# Создаем персонажа игрока
player = pygame.Rect(100, 100, PLAYER_SIZE, PLAYER_SIZE)

# Создаем список блоков
blocks = [pygame.Rect(x, 400, BLOCK_SIZE, BLOCK_SIZE) for x in range(0, SCREEN_WIDTH, BLOCK_SIZE)]

# Функция для перезапуска игры
def restart_game():
    global player, blocks
    player.topleft = (100, 100)
    blocks = [pygame.Rect(x, 400, BLOCK_SIZE, BLOCK_SIZE) for x in range(0, SCREEN_WIDTH, BLOCK_SIZE)]

# Цикл игры
running = True
while running:
    # Обрабатываем события
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r:
                restart_game()

    # Обновляем состояние игрока
    keys = pygame.key.get_pressed()
    if keys[pygame.K_UP]:
        player.y -= SPEED
    if keys[pygame.K_DOWN]:
        player.y += SPEED
    if keys[pygame.K_LEFT]:
        player.x -= SPEED
    if keys[pygame.K_RIGHT]:
        player.x += SPEED

    # Проверяем границы экрана
    if player.left < 0:
        player.left = 0
    if player.right > SCREEN_WIDTH:
        player.right = SCREEN_WIDTH
    if player.top < 0:
        player.top = 0
    if player.bottom > SCREEN_HEIGHT:
        player.bottom = SCREEN_HEIGHT

    # Проверяем столкновения игрока с блоками
    for block in blocks:
        if player.colliderect(block):
            restart_game()

    # Рисуем экран
    screen.fill((0, 0, 0))
    for block in blocks:
        pygame.draw.rect(screen, (255, 255, 255), block)
    pygame.draw.rect(screen, (255, 0, 0), player)
    pygame.display.flip()

    # Ограничиваем FPS
    pygame.time.Clock().tick(30)

# Завершаем pygame
pygame.quit()
sys.exit()
