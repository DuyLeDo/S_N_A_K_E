#include<bits/stdc++.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <vector>
#include <deque>               // Bao gồm deque
#include <cmath>
#include "graphics.h"
#include "defs.h"
#include "logic.h"
#include "highscore.h"

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
    const float boostDuration = 15.0f; // 3 giây tăng tốc
    const float boostSpeed = 200.0f;   // tốc độ tăng tốc
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

         // 1) tính t1, t2
    float t1 = 0.0f, t2 = 0.0f;
    if (isBoosting) {
        // phần thời gian còn boost
        t1 = min(deltaTime, boostTimer);
        boostTimer -= t1;
        if (boostTimer <= 0.0f) {
            isBoosting = false;
            boostTimer = 0.0f;
        }
        // phần dư (nếu boost kết thúc giữa chừng)
        t2 = deltaTime - t1;
    } else {
        // không boost: toàn bộ deltaTime là t2
        t2 = deltaTime;
    }

    // 2) di chuyển + growth cho t1 ở tốc độ boost
    if (t1 > 0.0f) {
        stepMovementAndGrowth(graphics, t1, boostSpeed);
    }
    // 3) di chuyển + growth cho t2 ở tốc độ bình thường
    if (t2 > 0.0f) {
        stepMovementAndGrowth(graphics, t2, normalSpeed);
    }
    }
    void stepMovementAndGrowth(Graphics &graphics, float dt, float speed) {
    float moveDist = speed * dt;
    float stepSize = GRID_SIZE;
    SnakeSegment head = snake.front();
    SnakeSegment newHead = head;
    int steps = 0;
    const int MAX_STEPS = 5;
    while (moveDist > 0.0f && steps < MAX_STEPS) {
        float step = min(moveDist, stepSize);

        // cập nhật direction khi head gần lưới
        if (nextDirection != (direction ^ 1)) {
           // chỉ đổi nếu không quay ngược trực tiếp
           direction = nextDirection;
       }
        // di chuyển
        switch (direction) {
            case DIR_UP:    newHead.y -= step; break;
            case DIR_DOWN:  newHead.y += step; break;
            case DIR_LEFT:  newHead.x -= step; break;
            case DIR_RIGHT: newHead.x += step; break;
        }

        // va chạm?
        if (checkCollision(newHead)) {
            graphics.playSound(graphics.gameOverSound);
            triggerGameOverEffect(graphics);
            return;
        }

        snake.push_front(newHead);

        // ăn food?
        if (moveDist <= stepSize && checkFoodCollision(newHead)) {
            graphics.playSound(graphics.eatSound);
            // pendingGrowth đã được set trong checkFoodCollision
        } else {
            // normal tail update
            updateGrowth(dt * (step / (speed * dt)), speed);
        }

        moveDist -= step;
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

    void reset() {
    // Khởi lại trạng thái chạy
    isRunning         = true;
    isGameOverEffect  = false;
    gameOverStartTime = 0;

    // Reset hướng đi
    direction     = DIR_RIGHT;
    nextDirection = DIR_RIGHT;

    // Xóa rắn và đặt đoạn đầu tại giữa màn
    snake.clear();
    snake.push_back({ SCREEN_WIDTH / 2.0f, 100 + GRID_SIZE * 3 });

    // Reset thức ăn
    foodEatenCount   = 0;
    isSpecialFood    = false;
    pendingGrowth    = 0;
    foodAlpha        = 255;
    isFading         = false;
    fadeStartTime    = 0;
    spawnFood();

    // Reset boost
    isBoosting   = false;
    boostTimer   = 0.0f;

    // Xóa mảnh vụn
    fragments.clear();

    // Reset thời gian cập nhật
    lastUpdateTime = SDL_GetTicks();

    // Reset điểm
    score = 0;
}

};
        void renderStartScreen(Graphics &graphics, TTF_Font* font, SDL_Rect &startButtonRect) {
    static SDL_Texture* bg = nullptr;
    if (!bg) bg = graphics.loadTexture(START_JPG);
    SDL_RenderCopy(graphics.renderer, bg, nullptr, nullptr);

    SDL_Texture* title  = graphics.renderText("SNAKE GAME", font, {128, 0, 255, 255});
    SDL_Texture* prompt = graphics.renderText("Click to Start", font, {0, 128, 255, 255});

    int w, h;
    SDL_QueryTexture(title, nullptr, nullptr, &w, &h);
    graphics.renderTexture(title, (SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT / 10);

    SDL_QueryTexture(prompt, nullptr, nullptr, &w, &h);
    int promptX = (SCREEN_WIDTH - w) / 2;
    int promptY = SCREEN_HEIGHT / 2;
    graphics.renderTexture(prompt, promptX, promptY);

    // Set vùng click của nút Start
    startButtonRect = {promptX, promptY, w, h};

    SDL_DestroyTexture(title);
    SDL_DestroyTexture(prompt);
}



void renderPauseScreen(Graphics &graphics, TTF_Font* font) {
    static SDL_Texture* bg = nullptr;
    if (!bg) bg = graphics.loadTexture(PAUSE_JPG);
    SDL_RenderCopy(graphics.renderer, bg, nullptr, nullptr);

    SDL_Texture* pausedText = graphics.renderText("PAUSED", font, {255, 255, 255, 255});
    SDL_Texture* resumeText = graphics.renderText("Press P to Resume", font, {255, 255, 0, 255});


    int w, h;
    SDL_QueryTexture(pausedText, NULL, NULL, &w, &h);
    graphics.renderTexture(pausedText, (SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT / 3);

    SDL_QueryTexture(resumeText, NULL, NULL, &w, &h);
    graphics.renderTexture(resumeText, (SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT / 2);

    SDL_DestroyTexture(pausedText);
    SDL_DestroyTexture(resumeText);
}
void renderGameOverScreen(Graphics &graphics, TTF_Font* font, int score, int highscore, SDL_Rect &restartButtonRect, SDL_Rect &backButtonRect) {
    static SDL_Texture* bg = nullptr;
    if (!bg) bg = graphics.loadTexture(GAMEOVER_JPG);
    SDL_RenderCopy(graphics.renderer, bg, nullptr, nullptr);

    SDL_Texture* go     = graphics.renderText("GAME OVER", font, {0, 0, 0, 255});
    SDL_Texture* scrTex = graphics.renderText(("Score: " + to_string(score)).c_str(), font, {0, 0, 0, 255});
    SDL_Texture* highscoreTex = graphics.renderText(("Highscore: " + to_string(highscore)).c_str(), font, {0, 0, 0, 255});
    SDL_Texture* restart = graphics.renderText("Click to Restart", font, {0, 0, 0, 255});
    SDL_Texture* back    = graphics.renderText("Click to Main Menu", font, {0, 0, 0, 255});

    int w, h;
    int y = 30;

    SDL_QueryTexture(go, NULL, NULL, &w, &h);
    graphics.renderTexture(go, (SCREEN_WIDTH - w) / 2-20, y);
    y += h + 10;

    SDL_QueryTexture(scrTex, NULL, NULL, &w, &h);
    graphics.renderTexture(scrTex, (SCREEN_WIDTH - w) / 2-20, y);
    y += h + 10;

    SDL_QueryTexture(highscoreTex, NULL, NULL, &w, &h);
    graphics.renderTexture(highscoreTex, (SCREEN_WIDTH - w) / 2-20, y);

    // Tính lại vị trí 2 nút dưới cùng
    int restartW, restartH, backW, backH;
    SDL_QueryTexture(restart, NULL, NULL, &restartW, &restartH);
    SDL_QueryTexture(back, NULL, NULL, &backW, &backH);

    int restartX = (SCREEN_WIDTH - restartW-20) / 2;
    int restartY = SCREEN_HEIGHT - restartH - backH - 40;

    int backX = (SCREEN_WIDTH - backW-20) / 2;
    int backY = restartY + restartH + 20;

    graphics.renderTexture(restart, restartX, restartY);
    graphics.renderTexture(back, backX, backY);

    restartButtonRect = {restartX, restartY, restartW, restartH};
    backButtonRect = {backX, backY, backW, backH};

    SDL_DestroyTexture(go);
    SDL_DestroyTexture(scrTex);
    SDL_DestroyTexture(highscoreTex);
    SDL_DestroyTexture(restart);
    SDL_DestroyTexture(back);
}



GameState currentState = STATE_START;

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

    // Đọc điểm cao nhất khi bắt đầu game
    int highscore = readHighScore();
    bool appRunning = true;
    SDL_Rect startButtonRect, restartButtonRect, backButtonRect;

while (appRunning) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            appRunning = false;
        }

        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            if (currentState == STATE_START) {
                SDL_Point p = {mouseX, mouseY};
                if (SDL_PointInRect(&p, &startButtonRect)) {
                    currentState = STATE_PLAYING;
                    snakeGame.reset();
                    lastFrameTime = SDL_GetTicks();
                }
            }
            else if (currentState == STATE_GAMEOVER) {
                SDL_Point p = {mouseX, mouseY};
                if (SDL_PointInRect(&p, &restartButtonRect)) {
                    snakeGame.reset();
                    currentState = STATE_PLAYING;
                    lastFrameTime = SDL_GetTicks();
                } else if (SDL_PointInRect(&p, &backButtonRect)) {
                    snakeGame.reset();
                    currentState = STATE_START;
                    lastFrameTime = SDL_GetTicks();
                }
            }
        }

        else if (currentState == STATE_PLAYING || currentState == STATE_PAUSED) {
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_p:  // Pause/Resume
                        if (currentState == STATE_PLAYING)
                            currentState = STATE_PAUSED;
                        else if (currentState == STATE_PAUSED)
                            currentState = STATE_PLAYING;
                        break;
                    case SDLK_m:  // Toggle sound
                        graphics.soundOn = !graphics.soundOn;
                        break;
                    case SDLK_n:  // Toggle music
                        graphics.toggleMusic();
                        break;
                }
            }

            if (currentState == STATE_PLAYING) {
                snakeGame.handleInput(e);
            }
        }
    }

    Uint32 currentFrameTime = SDL_GetTicks();
    float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
    lastFrameTime = currentFrameTime;

    // Cập nhật game khi đang chơi
    if (currentState == STATE_PLAYING) {
        snakeGame.update(graphics, deltaTime);
        if (snakeGame.isGameOverEffect) {
            if (SDL_GetTicks() - snakeGame.gameOverStartTime > 2000) {
                currentState = STATE_GAMEOVER;
            }
        }
    }

    // Render
    SDL_RenderClear(graphics.renderer);

    if (currentState == STATE_START) {
        renderStartScreen(graphics, font, startButtonRect);
    }

    else if (currentState == STATE_PLAYING) {
        background.scroll(1);
        graphics.render(background);
        snakeGame.render(graphics);

        string txt = "SCORE: " + to_string(snakeGame.score);
        scoreTex = graphics.renderText(txt.c_str(), font, scorecolor);

        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
        SDL_Rect separator = {0, 100, SCREEN_WIDTH, 2};
        SDL_RenderFillRect(graphics.renderer, &separator);
        graphics.renderTexture(scoreTex, 0, 0);

        if (snakeGame.isBoosting) {
            float remainingTime = snakeGame.boostTimer;
            string boostTxt = "BOOST: " + to_string(round(remainingTime * 10) / 10.0f) + "s";
            SDL_Texture* boostTex = graphics.renderText(boostTxt.c_str(), font, {255, 100, 100, 255});
            int textW, textH;
            SDL_QueryTexture(boostTex, NULL, NULL, &textW, &textH);
            graphics.renderTexture(boostTex, SCREEN_WIDTH - textW - 10, 0);
            SDL_DestroyTexture(boostTex);
        }
    }

    else if (currentState == STATE_PAUSED) {

    renderPauseScreen(graphics, font);


    }

    else if (currentState == STATE_GAMEOVER) {
        if (snakeGame.score > highscore) {
            highscore = snakeGame.score;
            writeHighScore(highscore);
        }
        renderGameOverScreen(graphics, font, snakeGame.score, highscore, restartButtonRect, backButtonRect);
    }

    SDL_RenderPresent(graphics.renderer);
    SDL_Delay(30);
}

SDL_DestroyTexture(scoreTex);
scoreTex = NULL;
TTF_CloseFont(font);
SDL_DestroyTexture(background.texture);
graphics.quit();
return 0;

}
