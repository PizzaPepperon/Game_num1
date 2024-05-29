#ifdef _WIN32
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

// Константы
const int SCREEN_WIDTH = 920;
const int SCREEN_HEIGHT = 700;
const int BLOCK_SIZE = 50;
const int PLAYER_SIZE = BLOCK_SIZE;
const int BASE_GROWTH_RATE = 3;

// Цвета
SDL_Color BLACK = {0, 0, 0, 255};
SDL_Color WHITE = {255, 255, 255, 255};

// Структура для фруктов
struct Fruit {
    SDL_Texture* image;
    int growth_rate;
    int points;
    SDL_Rect rect;

    Fruit(SDL_Texture* img, int growth, int pts) : image(img), growth_rate(growth), points(pts) {
        spawnFruit();
    }

    void spawnFruit() {
        rect.x = (rand() % ((SCREEN_WIDTH - PLAYER_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE;
        rect.y = (rand() % ((SCREEN_HEIGHT - PLAYER_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE;
        rect.w = PLAYER_SIZE;
        rect.h = PLAYER_SIZE;
    }
};

// Глобальные переменные
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Mix_Music* backgroundMusic = nullptr;
Mix_Chunk* eatSound = nullptr;
Mix_Chunk* gameOverSound = nullptr;
SDL_Texture* playerTexture = nullptr;
SDL_Texture* greenFruitTexture = nullptr;
SDL_Texture* yellowFruitTexture = nullptr;
SDL_Texture* blueFruitTexture = nullptr;
SDL_Rect player;
std::vector<SDL_Rect> snake_body;
Fruit* fruit = nullptr;
int score = 0;
std::string direction = "RIGHT";
bool game_over = false;
bool fullscreen = false;
Uint32 last_move_time = 0;
int move_delay = 200;
bool game_over_sound_played = false;

// Функция для загрузки текстуры
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer* ren) {
    SDL_Texture* texture = IMG_LoadTexture(ren, file.c_str());
    if (texture == nullptr) {
        std::cerr << "LoadTexture Error: " << IMG_GetError() << std::endl;
    }
    return texture;
}

// Функция для отображения текста
void renderText(const std::string &message, SDL_Color color, int x, int y, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, message.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = surf->w;
    dst.h = surf->h;
    SDL_FreeSurface(surf);
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

// Функция для перезапуска игры
void restartGame() {
    player.x = 100;
    player.y = 100;
    snake_body.clear();
    snake_body.push_back(player);
    delete fruit;
    fruit = new Fruit(greenFruitTexture, BASE_GROWTH_RATE, 1);
    score = 0;
    direction = "RIGHT";
    last_move_time = SDL_GetTicks();
    game_over = false;
    game_over_sound_played = false;
}

// Функция для переключения полноэкранного режима
void toggleFullscreen() {
    fullscreen = !fullscreen;
    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
}

// Функция для отображения экрана Game Over
void gameOverScreen(TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);

    renderText("Game Over", WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 60, font, renderer);
    renderText("Press R to Play Again", WHITE, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, font, renderer);
    renderText("Press M for Main Menu", WHITE, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 60, font, renderer);

    SDL_RenderPresent(renderer);
}

// Функция для выбора сложности и громкости
std::pair<int, int> chooseSettings(TTF_Font* font) {
    int volume = 50;
    bool choosing = true;
    int difficulty = 200;

    while (choosing) {
        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
        SDL_RenderClear(renderer);

        renderText("Press E for Easy", WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 60, font, renderer);
        renderText("Press M for Medium", WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, font, renderer);
        renderText("Press H for Hard", WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 60, font, renderer);
        renderText("Volume: " + std::to_string(volume) + "%", WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 120, font, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                choosing = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_e:
                        difficulty = 200;
                        choosing = false;
                        break;
                    case SDLK_m:
                        difficulty = 100;
                        choosing = false;
                        break;
                    case SDLK_h:
                        difficulty = 50;
                        choosing = false;
                        break;
                    case SDLK_UP:
                        if (volume < 100) volume += 10;
                        break;
                    case SDLK_DOWN:
                        if (volume > 0) volume -= 10;
                        break;
                }
                Mix_VolumeMusic(MIX_MAX_VOLUME * volume / 100);
            }
        }
    }
    return std::make_pair(difficulty, volume);
}

// Основная функция игры
void mainGame(TTF_Font* font) {
    auto settings = chooseSettings(font);
    move_delay = settings.first;
    Mix_VolumeMusic(MIX_MAX_VOLUME * settings.second / 100);

    restartGame();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_r:
                        restartGame();
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                        if (direction != "DOWN") direction = "UP";
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        if (direction != "UP") direction = "DOWN";
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        if (direction != "RIGHT") direction = "LEFT";
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        if (direction != "LEFT") direction = "RIGHT";
                        break;
                    case SDLK_f:
                        toggleFullscreen();
                        break;
                }
            }
        }

        if (game_over) {
            if (!game_over_sound_played) {
                Mix_PlayChannel(-1, gameOverSound, 0);
                game_over_sound_played = true;
            }
            gameOverScreen(font);
            continue;
        }

        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_move_time >= move_delay) {
            last_move_time = current_time;

            SDL_Rect new_head = snake_body[0];
            if (direction == "UP") new_head.y -= BLOCK_SIZE;
            if (direction == "DOWN") new_head.y += BLOCK_SIZE;
            if (direction == "LEFT") new_head.x -= BLOCK_SIZE;
            if (direction == "RIGHT") new_head.x += BLOCK_SIZE;

            if (new_head.x < 0) new_head.x = SCREEN_WIDTH - BLOCK_SIZE;
            if (new_head.x >= SCREEN_WIDTH) new_head.x = 0;
            if (new_head.y < 0) new_head.y = SCREEN_HEIGHT - BLOCK_SIZE;
            if (new_head.y >= SCREEN_HEIGHT) new_head.y = 0;

            for (const auto& segment : snake_body) {
                if (new_head.x == segment.x && new_head.y == segment.y) {
                    game_over = true;
                }
            }

            if (!game_over) {
                snake_body.insert(snake_body.begin(), new_head);
                if (SDL_HasIntersection(&new_head, &fruit->rect)) {
                    Mix_PlayChannel(-1, eatSound, 0);
                    score += fruit->points;
                    for (int i = 0; i < fruit->growth_rate; ++i) {
                        snake_body.push_back(snake_body.back());
                    }
                    delete fruit;
                    int randFruit = rand() % 3;
                    if (randFruit == 0) {
                        fruit = new Fruit(greenFruitTexture, BASE_GROWTH_RATE, 1);
                    } else if (randFruit == 1) {
                        fruit = new Fruit(yellowFruitTexture, BASE_GROWTH_RATE * 2, 3);
                    } else {
                        fruit = new Fruit(blueFruitTexture, BASE_GROWTH_RATE * 3, 5);
                    }
                } else {
                    snake_body.pop_back();
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, fruit->image, nullptr, &fruit->rect);

        for (size_t i = 0; i < snake_body.size(); ++i) {
            int alpha = 255 - (i * 10);
            SDL_SetTextureAlphaMod(playerTexture, alpha < 0 ? 0 : alpha);
            SDL_RenderCopy(renderer, playerTexture, nullptr, &snake_body[i]);
        }

        renderText("Score: " + std::to_string(score), WHITE, SCREEN_WIDTH - 150, 10, font, renderer);

        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("DAMINEM DONBASS Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    backgroundMusic = Mix_LoadMUS("source/music.mp3");
    if (backgroundMusic == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "Mix_LoadMUS Error: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    Mix_PlayMusic(backgroundMusic, -1);

    eatSound = Mix_LoadWAV("source/eat_sound.wav");
    if (eatSound == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "Mix_LoadWAV Error: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    gameOverSound = Mix_LoadWAV("source/game_over.wav");
    if (gameOverSound == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "Mix_LoadWAV Error: " << Mix_GetError() << std::endl;
        Mix_FreeChunk(eatSound);
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    playerTexture = loadTexture("source/player.png", renderer);
    greenFruitTexture = loadTexture("source/green_fruit.png", renderer);
    yellowFruitTexture = loadTexture("source/yellow_fruit.png", renderer);
    blueFruitTexture = loadTexture("source/blue_fruit.png", renderer);

    if (!playerTexture || !greenFruitTexture || !yellowFruitTexture || !blueFruitTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_FreeChunk(eatSound);
        Mix_FreeChunk(gameOverSound);
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    player = {100, 100, PLAYER_SIZE, PLAYER_SIZE};

    TTF_Font* font = TTF_OpenFont("source/arial.ttf", 24);
    if (!font) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_FreeChunk(eatSound);
        Mix_FreeChunk(gameOverSound);
        Mix_FreeMusic(backgroundMusic);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    mainGame(font);

    TTF_CloseFont(font);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(greenFruitTexture);
    SDL_DestroyTexture(yellowFruitTexture);
    SDL_DestroyTexture(blueFruitTexture);
    Mix_FreeChunk(eatSound);
    Mix_FreeChunk(gameOverSound);
    Mix_FreeMusic(backgroundMusic);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
