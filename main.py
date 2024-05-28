import pygame
import sys
import random
import time

# Инициализируем pygame
pygame.init()

# Константы
SCREEN_WIDTH = 920
SCREEN_HEIGHT = 700
BLOCK_SIZE = 50
PLAYER_SIZE = BLOCK_SIZE
BASE_GROWTH_RATE = 3

# Цвета
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Загружаем музыку и звуки
pygame.mixer.music.load('source/music.mp3')
pygame.mixer.music.play(-1)
eat_sound = pygame.mixer.Sound('source/eat_sound.wav')
game_over_sound = pygame.mixer.Sound('source/game_over.wav')

# Получаем информацию о дисплее
display_info = pygame.display.Info()

# Создаем окно игры
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Змейка")

# Шрифт для отображения текста
font = pygame.font.SysFont(None, 36)
small_font = pygame.font.SysFont(None, 24)

# Загружаем изображения
player_image = pygame.image.load('source/player.png').convert_alpha()
green_fruit_image = pygame.image.load('source/green_fruit.png').convert_alpha()
yellow_fruit_image = pygame.image.load('source/yellow_fruit.png').convert_alpha()
blue_fruit_image = pygame.image.load('source/blue_fruit.png').convert_alpha()

# Масштабируем изображения до нужного размера
player_image = pygame.transform.scale(player_image, (PLAYER_SIZE, PLAYER_SIZE))
green_fruit_image = pygame.transform.scale(green_fruit_image, (PLAYER_SIZE, PLAYER_SIZE))
yellow_fruit_image = pygame.transform.scale(yellow_fruit_image, (PLAYER_SIZE, PLAYER_SIZE))
blue_fruit_image = pygame.transform.scale(blue_fruit_image, (PLAYER_SIZE, PLAYER_SIZE))

# Класс для фруктов
class Fruit:
    def __init__(self, image, growth_rate, points):
        self.image = image
        self.growth_rate = growth_rate
        self.points = points
        self.rect = self.spawn_fruit()

    def spawn_fruit(self):
        x = random.randint(0, (SCREEN_WIDTH - PLAYER_SIZE) // BLOCK_SIZE) * BLOCK_SIZE
        y = random.randint(0, (SCREEN_HEIGHT - PLAYER_SIZE) // BLOCK_SIZE) * BLOCK_SIZE
        return pygame.Rect(x, y, PLAYER_SIZE, PLAYER_SIZE)

# Функция для перезапуска игры
def restart_game():
    global player, snake_body, fruit, score, direction, last_move_time, game_over
    player.topleft = (100, 100)
    snake_body = [player.copy()]
    fruit = random.choice(fruit_types)()
    score = 0
    direction = 'RIGHT'
    last_move_time = time.time()
    game_over = False

# Счетчик собранных фруктов
score = 0

# Начальное направление движения змейки
direction = 'RIGHT'

# Функция для выбора сложности
def choose_settings():
    choosing = True
    volume = 0.5
    while choosing:
        screen.fill(BLACK)
        easy_text = font.render('Press E for Easy', True, WHITE)
        medium_text = font.render('Press M for Medium', True, WHITE)
        hard_text = font.render('Press H for Hard', True, WHITE)
        volume_text = font.render(f'Volume: {int(volume * 100)}%', True, WHITE)
        community_text = small_font.render('Our Community: link', True, WHITE)
        screen.blit(easy_text, (SCREEN_WIDTH // 2 - easy_text.get_width() // 2, SCREEN_HEIGHT // 2 - 90))
        screen.blit(medium_text, (SCREEN_WIDTH // 2 - medium_text.get_width() // 2, SCREEN_HEIGHT // 2 - 30))
        screen.blit(hard_text, (SCREEN_WIDTH // 2 - hard_text.get_width() // 2, SCREEN_HEIGHT // 2 + 30))
        screen.blit(volume_text, (SCREEN_WIDTH // 2 - volume_text.get_width() // 2, SCREEN_HEIGHT // 2 + 90))
        screen.blit(community_text, (SCREEN_WIDTH - community_text.get_width() - 10, SCREEN_HEIGHT - community_text.get_height() - 10))

        # Отрисовка ползунка громкости
        volume_slider_rect = pygame.Rect(SCREEN_WIDTH // 2 - 100, SCREEN_HEIGHT // 2 + 120, 200, 10)
        pygame.draw.rect(screen, WHITE, volume_slider_rect)
        handle_x = SCREEN_WIDTH // 2 - 100 + int(volume * 200)
        handle_rect = pygame.Rect(handle_x - 5, SCREEN_HEIGHT // 2 + 115, 10, 20)
        pygame.draw.rect(screen, WHITE, handle_rect)

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key in [pygame.K_e, ord('E')]:
                    return 0.2, volume
                if event.key in [pygame.K_m, ord('M')]:
                    return 0.1, volume
                if event.key in [pygame.K_h, ord('H')]:
                    return 0.05, volume
            if event.type == pygame.MOUSEBUTTONDOWN:
                if volume_slider_rect.collidepoint(event.pos):
                    volume = (event.pos[0] - (SCREEN_WIDTH // 2 - 100)) / 200
                    volume = min(max(volume, 0.0), 1.0)
                    pygame.mixer.music.set_volume(volume)

# Выбираем сложность игры и громкость музыки
move_delay, volume = choose_settings()
pygame.mixer.music.set_volume(volume)

# Создаем персонажа игрока
player = pygame.Rect(100, 100, PLAYER_SIZE, PLAYER_SIZE)

# Список для частей тела змейки
snake_body = [player.copy()]

# Время последнего движения змейки
last_move_time = time.time()

# Ограничение FPS
clock = pygame.time.Clock()
FPS_LIMIT = 144

# Список типов фруктов
fruit_types = [
    lambda: Fruit(green_fruit_image, BASE_GROWTH_RATE, 1),
    lambda: Fruit(yellow_fruit_image, BASE_GROWTH_RATE * 2, 3),
    lambda: Fruit(blue_fruit_image, BASE_GROWTH_RATE * 3, 5)
]

# Создаем первый фрукт
fruit = random.choice(fruit_types)()

# Функция для переключения полноэкранного режима
fullscreen = False
def toggle_fullscreen():
    global fullscreen, screen, SCREEN_WIDTH, SCREEN_HEIGHT
    fullscreen = not fullscreen
    if fullscreen:
        SCREEN_WIDTH, SCREEN_HEIGHT = display_info.current_w, display_info.current_h
        screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.FULLSCREEN)
    else:
        SCREEN_WIDTH, SCREEN_HEIGHT = 920, 700
        screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))

# Функция для отображения экрана Game Over
def game_over_screen():
    screen.fill(BLACK)
    game_over_text = font.render('Game Over', True, WHITE)
    score_text = font.render(f'Score: {score}', True, WHITE)
    play_again_text = font.render('Press M to Play Again', True, WHITE)
    main_menu_text = font.render('Press R for Main Menu', True, WHITE)
    screen.blit(game_over_text, (SCREEN_WIDTH // 2 - game_over_text.get_width() // 2, SCREEN_HEIGHT // 2 - 60))
    screen.blit(score_text, (SCREEN_WIDTH // 2 - score_text.get_width() // 2, SCREEN_HEIGHT // 2))
    screen.blit(play_again_text, (SCREEN_WIDTH // 2 - play_again_text.get_width() // 2, SCREEN_HEIGHT // 2 + 60))
    screen.blit(main_menu_text, (SCREEN_WIDTH // 2 - main_menu_text.get_width() // 2, SCREEN_HEIGHT // 2 + 120))
    pygame.display.flip()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    restart_game()
                    return
                if event.key == pygame.K_m:
                    main()
                    return

# Основная функция игры
def main():
    global game_over, player, snake_body, fruit, score, direction, last_move_time, move_delay, volume
    restart_game()

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
                if event.key in [pygame.K_UP, pygame.K_w] and direction != 'DOWN':
                    direction = 'UP'
                if event.key in [pygame.K_DOWN, pygame.K_s] and direction != 'UP':
                    direction = 'DOWN'
                if event.key in [pygame.K_LEFT, pygame.K_a] and direction != 'RIGHT':
                    direction = 'LEFT'
                if event.key in [pygame.K_RIGHT, pygame.K_d] and direction != 'LEFT':
                    direction = 'RIGHT'
                if event.key == pygame.K_f:
                    toggle_fullscreen()

        if game_over:
            game_over_sound.play()
            game_over_screen()
            move_delay, volume = choose_settings()
            pygame.mixer.music.set_volume(volume)
            continue

        # Двигаем змейку, если прошло достаточно времени
        current_time = time.time()
        if current_time - last_move_time >= move_delay:
            last_move_time = current_time

            # Обновляем части тела змейки
            new_head = snake_body[0].copy()
            if direction == 'UP':
                new_head.y -= BLOCK_SIZE
            if direction == 'DOWN':
                new_head.y += BLOCK_SIZE
            if direction == 'LEFT':
                new_head.x -= BLOCK_SIZE
            if direction == 'RIGHT':
                new_head.x += BLOCK_SIZE

            # Проверяем телепортацию головы
            if new_head.left < 0:
                new_head.right = SCREEN_WIDTH
            if new_head.right > SCREEN_WIDTH:
                new_head.left = 0
            if new_head.top < 0:
                new_head.bottom = SCREEN_HEIGHT
            if new_head.bottom > SCREEN_HEIGHT:
                new_head.top = 0

            # Проверяем столкновение головы с телом
            if new_head in snake_body:
                game_over = True

            # Добавляем новую голову к змейке
            if not game_over:
                snake_body = [new_head] + snake_body[:-1]

                # Проверяем столкновение с фруктом
                if snake_body[0].colliderect(fruit.rect):
                    eat_sound.play()
                    score += fruit.points
                    for _ in range(fruit.growth_rate):
                        snake_body.append(snake_body[-1].copy())
                    fruit = random.choice(fruit_types)()

        # Рисуем экран
        screen.fill(BLACK)
        screen.blit(fruit.image, fruit.rect.topleft)
        for i, segment in enumerate(snake_body):
            transparency = max(0, 255 - (i * 10))
            segment_image = player_image.copy()
            segment_image.fill((255, 255, 255, transparency), special_flags=pygame.BLEND_RGBA_MULT)
            screen.blit(segment_image, segment.topleft)

        # Отображение счета
        score_text = font.render(f'Score: {score}', True, WHITE)
        screen.blit(score_text, (SCREEN_WIDTH - score_text.get_width() - 10, 10))

        pygame.display.flip()

        # Ограничиваем FPS
        clock.tick(FPS_LIMIT)

# Запуск игры
main()
pygame.quit()
sys.exit()
