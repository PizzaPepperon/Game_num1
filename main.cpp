#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <string>

#include "source/music.h"
#include "source/eat_sound.h"
#include "source/game_over.h"
#include "source/font.h"
#include "source/blue_fruit.h"
#include "source/yellow_fruit.h"
#include "source/green_fruit.h"
#include "source/player.h"
#include "source/icon.h"

const int SCREEN_WIDTH = 920;
const int SCREEN_HEIGHT = 700;
const int BLOCK_SIZE = 50;
const int PLAYER_SIZE = BLOCK_SIZE;

const float EASY_SPEED = 0.2f;
const float MEDIUM_SPEED = 0.15f;
const float HARD_SPEED = 0.1f;

const int EASY_GROWTH_RATE = 1;
const int MEDIUM_GROWTH_RATE = 2;
const int HARD_GROWTH_RATE = 3;

struct Fruit {
    sf::Sprite sprite;
    int growthRate;
    int points;
};

void restartGame(sf::RectangleShape& player, std::vector<sf::RectangleShape>& snakeBody, Fruit& fruit, int& score, std::string& direction, bool& gameOver);
Fruit spawnFruit(sf::Texture& greenTexture, sf::Texture& yellowTexture, sf::Texture& blueTexture, int growthRate, const std::vector<sf::RectangleShape>& snakeBody);

enum GameState { MainMenu, InGame, GameOver };

enum Difficulty { Easy, Medium, Hard };

std::string getDifficultyText(Difficulty diff) {
    switch (diff) {
        case Easy: return "Easy";
        case Medium: return "Normal";
        case Hard: return "Hard";
        default: return "Unknown";
    }
};

void updateDifficultySettings(Difficulty& currentDifficulty, float& currentSpeed, int& currentGrowthRate) {
    switch (currentDifficulty) {
        case Easy:
            currentSpeed = EASY_SPEED;
            currentGrowthRate = EASY_GROWTH_RATE;
            break;
        case Medium:
            currentSpeed = MEDIUM_SPEED;
            currentGrowthRate = MEDIUM_GROWTH_RATE;
            break;
        case Hard:
            currentSpeed = HARD_SPEED;
            currentGrowthRate = HARD_GROWTH_RATE;
            break;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Daminem Snake Game");

    sf::Image icon;
    if (icon.loadFromMemory(icon_jpg, icon_jpg_len))
    {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromMemory(font_ttf, font_ttf_len)) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    sf::SoundBuffer eatBuffer, gameOverBuffer;
    if (!eatBuffer.loadFromMemory(eat_sound_wav, eat_sound_wav_len) || !gameOverBuffer.loadFromMemory(game_over_wav, game_over_wav_len)) {
        std::cerr << "Failed to load sound files!" << std::endl;
    return -1;
        }
        
        sf::Sound gameover; 
        gameover.setBuffer(gameOverBuffer);
        bool gameoverPlayed = false;

        sf::Sound eatSound(eatBuffer);
        sf::Sound gameOverSound(gameOverBuffer);

        sf::Music music;
        if (!music.openFromMemory(music_ogg, music_ogg_len)) {
            std::cerr << "Failed to load music file!" << std::endl;
            return -1;
        }
        music.setLoop(true);
        music.play();

        sf::Texture playerTexture, greenFruitTexture, yellowFruitTexture, blueFruitTexture;
        if (!playerTexture.loadFromMemory(player_png, player_png_len) ||
            !greenFruitTexture.loadFromMemory(green_fruit_png, green_fruit_png_len) ||
            !yellowFruitTexture.loadFromMemory(yellow_fruit_png, yellow_fruit_png_len) ||
            !blueFruitTexture.loadFromMemory(blue_fruit_png, blue_fruit_png_len)) {
            std::cerr << "Failed to load texture files!" << std::endl;
        return -1;
            }

            sf::RectangleShape player(sf::Vector2f(PLAYER_SIZE, PLAYER_SIZE));
            player.setPosition(100, 100);
            player.setTexture(&playerTexture);

            std::vector<sf::RectangleShape> snakeBody;
            snakeBody.push_back(player);

            Difficulty currentDifficulty = Easy;
            float currentSpeed = EASY_SPEED;
            int currentGrowthRate = EASY_GROWTH_RATE;

            Fruit fruit = spawnFruit(greenFruitTexture, yellowFruitTexture, blueFruitTexture, currentGrowthRate, snakeBody);

            int score = 0;
            std::string direction = "RIGHT";
            bool gameOver = false;
            sf::Clock clock;
            sf::Time moveDelay = sf::seconds(currentSpeed);

            GameState gameState = MainMenu;

            sf::Text volumeText("Volume: 100%", font, 24);
            volumeText.setFillColor(sf::Color::White);
            volumeText.setPosition(10, 40);
            int volume = 100;

            sf::RectangleShape sliderBar(sf::Vector2f(200, 10));
            sliderBar.setPosition(10, volumeText.getPosition().y + volumeText.getGlobalBounds().height + 10);
            sliderBar.setFillColor(sf::Color::White);

            sf::RectangleShape sliderKnob(sf::Vector2f(10, 20));
            sliderKnob.setPosition(sliderBar.getPosition().x + (volume / 100.0f) * (sliderBar.getSize().x - sliderKnob.getSize().x), sliderBar.getPosition().y - 5);
            sliderKnob.setFillColor(sf::Color::Red);

            bool isDraggingSlider = false;

            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        window.close();

                    if (event.type == sf::Event::MouseButtonPressed) 
                    {
                        if (event.mouseButton.button == sf::Mouse::Left) 
                        {
                            if (sliderKnob.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) 
                            {
                                isDraggingSlider = true;
                            }
                        }
                    }

                    if (event.type == sf::Event::MouseButtonReleased) 
                    {
                        if (event.mouseButton.button == sf::Mouse::Left) 
                            {
                                isDraggingSlider = false;
                            }
                    }

                    if (event.type == sf::Event::MouseMoved) 
                    {
                        if (isDraggingSlider) 
                        {
                            float newPos = event.mouseMove.x;
                            if (newPos < sliderBar.getPosition().x)
                                newPos = sliderBar.getPosition().x;
                            if (newPos > sliderBar.getPosition().x + sliderBar.getSize().x - sliderKnob.getSize().x)
                                newPos = sliderBar.getPosition().x + sliderBar.getSize().x - sliderKnob.getSize().x;
                            
                            sliderKnob.setPosition(newPos, sliderKnob.getPosition().y);
                            
                            // Рассчитываем новую громкость
                            volume = static_cast<int>((newPos - sliderBar.getPosition().x) / (sliderBar.getSize().x - sliderKnob.getSize().x) * 100);
                            music.setVolume(volume);
                            
                            // Обновляем текст громкости
                            volumeText.setString("Volume: " + std::to_string(volume) + "%");
                        }
                    }
                    
                    if (event.type == sf::Event::KeyPressed) {
                        if (gameState == MainMenu) {
                            if (event.key.code == sf::Keyboard::Enter) {
                                gameState = InGame;
                            } else if (event.key.code == sf::Keyboard::Left) {
                                // Cycle difficulty backwards
                                switch (currentDifficulty) {
                                    case Easy:
                                        currentDifficulty = Hard;
                                        break;
                                    case Medium:
                                        currentDifficulty = Easy;
                                        break;
                                    case Hard:
                                        currentDifficulty = Medium;
                                        break;
                                }
                                updateDifficultySettings(currentDifficulty, currentSpeed, currentGrowthRate);
                            } else if (event.key.code == sf::Keyboard::Right) {
                                // Cycle difficulty forwards
                                switch (currentDifficulty) {
                                    case Easy:
                                        currentDifficulty = Medium;
                                        break;
                                    case Medium:
                                        currentDifficulty = Hard;
                                        break;
                                    case Hard:
                                        currentDifficulty = Easy;
                                        break;
                                }
                                updateDifficultySettings(currentDifficulty, currentSpeed, currentGrowthRate);
                            }

                            if (event.key.code == sf::Keyboard::Enter) {
                                gameState = InGame;
                            } else if (event.key.code == sf::Keyboard::Up) {
                                volume = std::min(100, volume + 10);
                                music.setVolume(volume);
                                volumeText.setString("Volume: " + std::to_string(volume) + "%");
                            } else if (event.key.code == sf::Keyboard::Down) {
                                volume = std::max(0, volume - 10);
                                music.setVolume(volume);
                                volumeText.setString("Volume: " + std::to_string(volume) + "%");
                            }
                        } else if (gameState == InGame) {
                            gameoverPlayed = false;
                            if (event.key.code == sf::Keyboard::R)
                                restartGame(player, snakeBody, fruit, score, direction, gameOver);
                            if (event.key.code == sf::Keyboard::Up && direction != "DOWN")
                                direction = "UP";
                            if (event.key.code == sf::Keyboard::Down && direction != "UP")
                                direction = "DOWN";
                            if (event.key.code == sf::Keyboard::Left && direction != "RIGHT")
                                direction = "LEFT";
                            if (event.key.code == sf::Keyboard::Right && direction != "LEFT")
                                direction = "RIGHT";
                            if (event.key.code == sf::Keyboard::F11) {
                                if (window.getSize() == sf::Vector2u(SCREEN_WIDTH, SCREEN_HEIGHT)) {
                                    window.create(sf::VideoMode::getDesktopMode(), "Daminem Snake Game", sf::Style::Fullscreen);
                                } else {
                                    window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Daminem Snake Game");
                                }
                                window.setFramerateLimit(60);
                            }
                        } else if (gameState == GameOver) {
                            if (event.key.code == sf::Keyboard::R) {
                                restartGame(player, snakeBody, fruit, score, direction, gameOver);
                                gameState = InGame;
                            } else if (event.key.code == sf::Keyboard::Escape) {
                                restartGame(player, snakeBody, fruit, score, direction, gameOver);
                                gameState = MainMenu;
                            }
                        }
                    }
                }

                if (gameState == MainMenu) {
                    window.clear();
                    sf::Text title("Daminem Snake Game", font, 50);
                    title.setFillColor(sf::Color::Green);
                    title.setPosition(SCREEN_WIDTH / 2 - title.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 - 100);

                    sf::Text startPrompt("Press Enter to Start", font, 24);
                    startPrompt.setFillColor(sf::Color::White);
                    startPrompt.setPosition(SCREEN_WIDTH / 2 - startPrompt.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2);

                    sf::Text ourtext("Our Community: https://vk.com/club225497152", font, 24);
                    ourtext.setFillColor(sf::Color::White);
                    ourtext.setPosition(SCREEN_WIDTH - ourtext.getGlobalBounds().width - 20, SCREEN_HEIGHT - ourtext.getGlobalBounds().height * 2 );

                    sf::Text difficultyText("< Difficulty: " + getDifficultyText(currentDifficulty) + " >", font, 24);
                    difficultyText.setFillColor(sf::Color::White);
                    difficultyText.setPosition(SCREEN_WIDTH / 2 - difficultyText.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 + 50);

                    window.draw(title);
                    window.draw(startPrompt);
                    window.draw(ourtext);
                    window.draw(difficultyText);
                    window.draw(volumeText);
                    window.draw(sliderBar);
                    window.draw(sliderKnob);
                    window.display();
                    continue;
                }

                moveDelay = sf::seconds(currentSpeed);

                if (gameOver) {
                    if (gameOverSound.getStatus() != sf::Sound::Playing) {
                         if (!gameoverPlayed) { 
                            gameover.play(); 
                            gameoverPlayed = true;
                        }
                    }
                    window.clear();
                    sf::Text gameOverText("Game Over", font, 36);
                    gameOverText.setFillColor(sf::Color::White);
                    gameOverText.setPosition(SCREEN_WIDTH / 2 - gameOverText.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 - 50);
                    window.draw(gameOverText);

                    sf::Text scoreText("Score: " + std::to_string(score), font, 24);
                    scoreText.setFillColor(sf::Color::White);
                    scoreText.setPosition(SCREEN_WIDTH / 2 - scoreText.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2);

                    sf::Text restartPrompt("Press R to Restart or Esc for Menu", font, 18);
                    restartPrompt.setFillColor(sf::Color::White);
                    restartPrompt.setPosition(SCREEN_WIDTH / 2 - restartPrompt.getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 + 50);
                    window.draw(scoreText);
                    window.draw(restartPrompt);

                    window.display();
                    continue;
                }

                if (clock.getElapsedTime() >= moveDelay) {
                    clock.restart();
                    sf::Vector2f newHeadPosition = snakeBody.front().getPosition();

                    if (direction == "UP")
                        newHeadPosition.y -= BLOCK_SIZE;
                    else if (direction == "DOWN")
                        newHeadPosition.y += BLOCK_SIZE;
                    else if (direction == "LEFT")
                        newHeadPosition.x -= BLOCK_SIZE;
                    else if (direction == "RIGHT")
                        newHeadPosition.x += BLOCK_SIZE;

                    if (newHeadPosition.x < 0)
                        newHeadPosition.x = SCREEN_WIDTH - BLOCK_SIZE;
                    else if (newHeadPosition.x >= SCREEN_WIDTH)
                        newHeadPosition.x = 0;
                    if (newHeadPosition.y < 0)
                        newHeadPosition.y = SCREEN_HEIGHT - BLOCK_SIZE;
                    else if (newHeadPosition.y >= SCREEN_HEIGHT)
                        newHeadPosition.y = 0;

                    sf::RectangleShape newHead(sf::Vector2f(PLAYER_SIZE, PLAYER_SIZE));
                    newHead.setPosition(newHeadPosition);
                    newHead.setTexture(&playerTexture);

                    if (std::any_of(snakeBody.begin(), snakeBody.end(), [&](sf::RectangleShape& segment) { return segment.getPosition() == newHeadPosition; })) {
                        gameOver = true;
                        gameState = GameOver;
                    }

                    if (!gameOver) {
                        snakeBody.insert(snakeBody.begin(), newHead);
                        if (newHead.getGlobalBounds().intersects(fruit.sprite.getGlobalBounds())) {
                            eatSound.play();
                            score += fruit.points;
                            for (int i = 0; i < fruit.growthRate; ++i)
                                snakeBody.push_back(snakeBody.back());
                            fruit = spawnFruit(greenFruitTexture, yellowFruitTexture, blueFruitTexture, currentGrowthRate, snakeBody);
                        } else {
                            snakeBody.pop_back();
                        }
                    }
                }

                window.clear();

                window.draw(fruit.sprite);
                for (auto& segment : snakeBody)
                    window.draw(segment);

                sf::Text scoreText("Score: " + std::to_string(score), font, 24);
                scoreText.setFillColor(sf::Color::White);
                scoreText.setPosition(10, 10);
                window.draw(scoreText);

                window.display();
            }

            return 0;
}

Fruit spawnFruit(sf::Texture& greenTexture, sf::Texture& yellowTexture, sf::Texture& blueTexture, int growthRate, const std::vector<sf::RectangleShape>& snakeBody) {
    Fruit fruit;
    
    // Случайный выбор текстуры
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> textureDist(0, 2);
    int textureChoice = textureDist(gen);
    
    switch(textureChoice) {
        case 0:
            fruit.sprite.setTexture(greenTexture);
            fruit.points = 1;
            fruit.growthRate = growthRate;
            break;
        case 1:
            fruit.sprite.setTexture(yellowTexture);
            fruit.points = 2;
            fruit.growthRate = growthRate;
            break;
        case 2:
            fruit.sprite.setTexture(blueTexture);
            fruit.points = 3;
            fruit.growthRate = growthRate;
            break;
    }
    
    fruit.sprite.setScale(
        static_cast<float>(PLAYER_SIZE) / fruit.sprite.getTexture()->getSize().x,
                          static_cast<float>(PLAYER_SIZE) / fruit.sprite.getTexture()->getSize().y
    );
    
    // Генерация позиции фрукта
    std::uniform_int_distribution<> xDist(0, (SCREEN_WIDTH - PLAYER_SIZE) / BLOCK_SIZE - 1);
    std::uniform_int_distribution<> yDist(0, (SCREEN_HEIGHT - PLAYER_SIZE) / BLOCK_SIZE - 1);
    
    bool validPosition = false;
    sf::Vector2f position;
    
    // Пытаемся найти валидную позицию
    while (!validPosition) {
        position = sf::Vector2f(xDist(gen) * BLOCK_SIZE, yDist(gen) * BLOCK_SIZE);
        validPosition = true;
        
        // Проверяем пересечение с каждым сегментом змеи
        for (const auto& segment : snakeBody) {
            if (segment.getPosition() == position) {
                validPosition = false;
                break;
            }
        }
    }
    
    fruit.sprite.setPosition(position);
    return fruit;
}

void restartGame(sf::RectangleShape& player, std::vector<sf::RectangleShape>& snakeBody, Fruit& fruit, int& score, std::string& direction, bool& gameOver) {
    player.setPosition(100, 100);
    snakeBody.clear();
    snakeBody.push_back(player);
    score = 0;
    direction = "RIGHT";
    gameOver = false;
}
