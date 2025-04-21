#include <SDL.h>
#include <vector>
#include <deque>               // Bao gồm deque
#include <cmath>
#include "graphics.h"
#include "defs.h"
#include "logic.h"
#include "gameover.h"
#include <SDL_ttf.h>
using namespace std;

struct SnakeSegment {
    float x, y;
};

struct SnakeGame {
    bool isRunning = true;
    int direction = DIR_RIGHT;
    int nextDirection = DIR_RIGHT;
    deque<SnakeSegment> snake;  // Sử dụng deque thay cho vector
    SnakeSegment food;
    Uint32 lastUpdateTime = 0;

    int foodEatenCount = 0;
    bool isSpecialFood = false;
    double pendingGrowth = 0;
    Uint8 foodAlpha = 255;
    bool isFading = false;
    Uint32 fadeStartTime = 0;

    bool isBoosting = false;
    float boostTimer = 0.0f;
    const float boostDuration = 30.0f; // 3 giây tăng tốc
    const float boostSpeed = 130.0f;   // tốc độ tăng tốc
    float normalSpeed = 100.0f;        // tốc độ di chuyển bình thường

    vector<SnakeFragment> fragments;
    bool isGameOverEffect = false;
    Uint32 gameOverStartTime = 0;

    int score = 0;

    SnakeGame() {
        // Khởi tạo đầu rắn ở giữa màn hình
        snake.push_back({SCREEN_WIDTH / 2.0f, 100 + GRID_SIZE * 3});

        spawnFood();
        lastUpdateTime = SDL_GetTicks();
    }

    void handleInput(SDL_Event &e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    if (direction != DIR_DOWN) nextDirection = DIR_UP;
                    break;
                case SDLK_DOWN:
                    if (direction != DIR_UP) nextDirection = DIR_DOWN;
                    break;
                case SDLK_LEFT:
                    if (direction != DIR_RIGHT) nextDirection = DIR_LEFT;
                    break;
                case SDLK_RIGHT:
                    if (direction != DIR_LEFT) nextDirection = DIR_RIGHT;
                    break;
                case SDLK_LSHIFT: // Kích hoạt tăng tốc khi nhấn Shift
                case SDLK_RSHIFT:
                    if (!isBoosting) {
                        isBoosting = true;
                        boostTimer = boostDuration;
                    }
                    break;
            }
        }
    }

    void update(Graphics &graphics, float deltaTime) {
        if (isGameOverEffect) {
            updateGameOverEffect(deltaTime);
            // Kiểm tra nếu hiệu ứng chạy quá 2 giây, game dừng
            if (SDL_GetTicks() - gameOverStartTime > 2000) {
                isRunning = false;
            }
            return;
        }

        Uint32 currentTime = SDL_GetTicks();

        // Cập nhật hiệu ứng fading của thức ăn
        if (isFading) {
            Uint32 elapsedTime = currentTime - fadeStartTime;
            if (elapsedTime >= 200) {
                spawnFood();
                foodAlpha = 255;
                isFading = false;
            } else {
                foodAlpha = 255 - (elapsedTime * 255 / 200);
            }
        }

        // Cập nhật trạng thái tăng tốc
        if (isBoosting) {
            boostTimer -= deltaTime;
            if (boostTimer <= 0.0f) {
                isBoosting = false;
                boostTimer = 0.0f;
            }
        }

        // Tính tốc độ hiện tại và quãng đường cần di chuyển
        float currentSpeed = isBoosting ? boostSpeed : normalSpeed;
        float moveDistance = currentSpeed * deltaTime; // Tổng quãng đường cần di chuyển
        float stepSize = GRID_SIZE; // Mỗi bước di chuyển bằng kích thước của ô lưới

        SnakeSegment &head = snake.front();
        SnakeSegment newHead = head;

        // Chia nhỏ quãng đường thành các bước
        const int MAX_STEPS_PER_FRAME = 5;
        int steps = 0;
        while (moveDistance > 0 && steps < MAX_STEPS_PER_FRAME) {
            float step = min(moveDistance, stepSize); // Lấy bước nhỏ nhất

            // Cập nhật hướng nếu đầu gần khớp lưới
            if (fabs(fmod(newHead.x, GRID_SIZE)) < step &&
                fabs(fmod(newHead.y, GRID_SIZE)) < step) {
                direction = nextDirection;
            }

            // Di chuyển theo hướng
            switch (direction) {
                case DIR_UP:    newHead.y -= step; break;
                case DIR_DOWN:  newHead.y += step; break;
                case DIR_LEFT:  newHead.x -= step; break;
                case DIR_RIGHT: newHead.x += step; break;
            }

            // Kiểm tra va chạm sau mỗi bước
            if (checkCollision(newHead)) {
                graphics.playSound(graphics.gameOverSound);
                // Gọi hiệu ứng nổ
                triggerGameOverEffect(graphics);
                return;
            }

            // Thêm phần tử mới vào đầu rắn sử dụng deque (push_front nhanh)
            snake.push_front(newHead);

            // Kiểm tra ăn thức ăn chỉ ở bước cuối cùng
            if (moveDistance <= stepSize && checkFoodCollision(newHead)) {
                graphics.playSound(graphics.eatSound);
                if (isSpecialFood) {
                    pendingGrowth = snake.size() * 2; // Tăng trưởng lớn hơn cho thức ăn đặc biệt
                } else {
                    pendingGrowth += 1; // Tăng trưởng bình thường
                }
            } else {
                updateGrowth(deltaTime / (moveDistance / step), currentSpeed); // Điều chỉnh deltaTime cho từng bước
            }

            moveDistance -= step;
            steps++;
        }
    }

    // Sửa hàm checkFoodCollision: sử dụng làm tròn tọa độ để so sánh
    bool checkFoodCollision(const SnakeSegment &head) {
        if (isFading)
            return false;

        int headX = round(head.x / GRID_SIZE) * GRID_SIZE;
        int headY = round(head.y / GRID_SIZE) * GRID_SIZE;
        int foodX = static_cast<int>(food.x);
        int foodY = static_cast<int>(food.y);

        if (headX == foodX && headY == foodY) {
            score += isSpecialFood ? 50 : 10;
            foodEatenCount++;
            if (isSpecialFood) {
                pendingGrowth = snake.size() * 1.5;
            }
            isFading = true;
            fadeStartTime = SDL_GetTicks();
            return true;
        }
        return false;
    }

    bool checkCollision(const SnakeSegment &head) {
        if (head.x < 0 || head.y < 100 || head.x >= SCREEN_WIDTH || head.y >= SCREEN_HEIGHT)
            return true;

        for (size_t i = 1; i < snake.size(); i++) {
            if ((int)round(head.x) == (int)round(snake[i].x) &&
                (int)round(head.y) == (int)round(snake[i].y)) {
                return true;
            }
        }
        return false;
    }

    void spawnFood() {
        bool validPosition = false;
        while (!validPosition) {
            // Sinh tọa độ thức ăn theo lưới
            food.x = (rand() % (SCREEN_WIDTH / GRID_SIZE)) * GRID_SIZE;
            food.y = (rand() % ((SCREEN_HEIGHT - 100) / GRID_SIZE)) * GRID_SIZE + 100;

            validPosition = true;
            // Duyệt qua từng segment của rắn để kiểm tra
            for (const auto &segment : snake) {
                int segX = static_cast<int>(round(segment.x / GRID_SIZE)) * GRID_SIZE;
                int segY = static_cast<int>(round(segment.y / GRID_SIZE)) * GRID_SIZE;
                if (segX == static_cast<int>(food.x) && segY == static_cast<int>(food.y)) {
                    validPosition = false;
                    break;
                }
            }
        }

        // Kiểm tra số lượng thức ăn đã ăn để xác định special food
        if (foodEatenCount == 6) {
            isSpecialFood = true;
            foodEatenCount = 0;
        } else {
            isSpecialFood = false;
        }
    }

    void updateGrowth(float deltaTime, float currentSpeed) {
        float growthFactor = isBoosting ? 0.5f : 1.0f;
        float growthConsumption = currentSpeed * deltaTime * growthFactor;
        if (pendingGrowth > growthConsumption) {
            pendingGrowth -= growthConsumption;
        } else {
            pendingGrowth = 0;
            // Xóa đuôi rắn bằng pop_back của deque
            snake.pop_back();
        }
    }

    // Khi game over, chuyển sang hiệu ứng vỡ vụn
    void triggerGameOverEffect(Graphics &graphics) {
        fragments.clear();
        for (auto &segment : snake) {
            SnakeFragment frag;
            frag.x = segment.x;
            frag.y = segment.y;
            float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
            float speed = 100 + rand() % 100; // vận tốc ngẫu nhiên
            frag.vx = cos(angle) * speed;
            frag.vy = sin(angle) * speed;
            fragments.push_back(frag);
        }
        // Xóa toàn bộ rắn
        snake.clear();
        isGameOverEffect = true;
        gameOverStartTime = SDL_GetTicks();
        graphics.playSound(graphics.gameOverSound);
    }

    // Update các fragment hiệu ứng game over
    void updateGameOverEffect(float deltaTime) {
        for (auto &frag : fragments) {
            frag.x += frag.vx * deltaTime;
            frag.y += frag.vy * deltaTime;
            // Giảm alpha theo thời gian
            frag.alpha = max(0.0f, frag.alpha - 200 * deltaTime);
        }
    }

    // Render các fragment hiệu ứng game over
    void renderGameOverEffect(Graphics &graphics) {
        for (auto &frag : fragments) {
            SDL_Rect rect;
            rect.x = static_cast<int>(frag.x);
            rect.y = static_cast<int>(frag.y);
            rect.w = GRID_SIZE;
            rect.h = GRID_SIZE;
            SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, static_cast<Uint8>(frag.alpha));
            SDL_RenderFillRect(graphics.renderer, &rect);
        }
    }

    // Hàm vẽ hình vuông, với center, halfSize và màu cho trước.
    void renderSquare(SDL_Renderer *renderer, int centerX, int centerY, int halfSize, SDL_Color color) {
        SDL_Rect rect;
        rect.x = centerX - halfSize;
        rect.y = centerY - halfSize;
        rect.w = halfSize * 2;
        rect.h = halfSize * 2;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    void render(Graphics &graphics) {
        if (isGameOverEffect) {
            renderGameOverEffect(graphics);
            return;
        }
        const int shadowOffset = 4;
        SDL_Color shadowColor = {0, 0, 0, 100};

        // --- Vẽ bóng cho các segment của rắn ---
        for (size_t i = 0; i < snake.size(); i++) {
            int halfSize = (i == 0) ? GRID_SIZE / 2 : (GRID_SIZE / 2 - 2);
            int centerX = static_cast<int>(round(snake[i].x + GRID_SIZE / 2));
            int centerY = static_cast<int>(round(snake[i].y + GRID_SIZE / 2));
            renderSquare(graphics.renderer, centerX + shadowOffset, centerY + shadowOffset, halfSize, shadowColor);
        }

        // --- Vẽ thân rắn và mắt của đầu rắn ---
        for (size_t i = 0; i < snake.size(); i++) {
            int halfSize = (i == 0) ? GRID_SIZE / 2 : (GRID_SIZE / 2 - 2);
            int centerX = static_cast<int>(round(snake[i].x + GRID_SIZE / 2));
            int centerY = static_cast<int>(round(snake[i].y + GRID_SIZE / 2));
            renderSquare(graphics.renderer, centerX, centerY, halfSize, {0, 255, 0, 255});

            if (i == 0) {
                int eyeOffsetX = (direction == DIR_LEFT) ? -5 : (direction == DIR_RIGHT) ? 5 : 0;
                int eyeOffsetY = (direction == DIR_UP) ? -5 : (direction == DIR_DOWN) ? 5 : 0;
                int eyeHalfSize = 2;
                renderSquare(graphics.renderer, centerX + eyeOffsetX, centerY + eyeOffsetY - 3, eyeHalfSize, {255, 255, 255, 255});
                renderSquare(graphics.renderer, centerX + eyeOffsetX, centerY + eyeOffsetY + 3, eyeHalfSize, {255, 255, 255, 255});
            }
        }

        // --- Vẽ thức ăn và bóng của nó ---
        int foodHalfSize = isSpecialFood ? GRID_SIZE / 4 : GRID_SIZE / 5;
        int foodCenterX = static_cast<int>(round(food.x + GRID_SIZE / 2));
        int foodCenterY = static_cast<int>(round(food.y + GRID_SIZE / 2));
        renderSquare(graphics.renderer, foodCenterX + shadowOffset, foodCenterY + shadowOffset, foodHalfSize, shadowColor);
        SDL_Color foodColor = isSpecialFood ? SDL_Color{255, 165, 0, foodAlpha} : SDL_Color{255, 0, 0, foodAlpha};
        renderSquare(graphics.renderer, foodCenterX, foodCenterY, foodHalfSize, foodColor);
    }
};

int main(int argc, char *argv[]) {
    Graphics graphics;
    graphics.init();

    ScrollingBackground background;
    background.setTexture(graphics.loadTexture(BACKGROUND_IMG));

    SnakeGame snakeGame;
    Uint32 lastFrameTime = SDL_GetTicks();

    TTF_Font* font = graphics.loadFont("assets/Purisa-BoldOblique.ttf", 20);
    SDL_Color scorecolor = {255, 255, 0, 0};
    SDL_Texture* scoreTex;




    while (snakeGame.isRunning) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                snakeGame.isRunning = false;
            snakeGame.handleInput(e);
        }

        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        snakeGame.update(graphics, deltaTime);
        background.scroll(1);
        graphics.render(background);
        snakeGame.render(graphics);

        string txt= "SCORE: " + to_string(snakeGame.score);
        scoreTex = graphics.renderText(txt.c_str(), font, scorecolor);


        // Vẽ thanh ngang phân cách
        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255); // màu trắng
        SDL_Rect separator = {0, 100, SCREEN_WIDTH, 2}; // thanh cao 2px tại y=100
        SDL_RenderFillRect(graphics.renderer, &separator);
        graphics.renderTexture(scoreTex, 0, 0);
        SDL_RenderPresent(graphics.renderer);

        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);
        SDL_Delay(30);
    }



    SDL_DestroyTexture( scoreTex );
    scoreTex = NULL;
    TTF_CloseFont( font );
    SDL_DestroyTexture(background.texture);
    graphics.quit();
    return 0;
}


