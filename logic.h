/*#ifndef _logic__H
#define _logic__H

#include <iostream>
#include <SDL.h>
#include <vector>
#include <cmath>
#include "graphics.h"
#include "defs.h"



struct SnakeSegment {
    float x, y;
};

struct SnakeGame {
    bool isRunning = true;
    int direction = DIR_RIGHT;
    int nextDirection = DIR_RIGHT;
    vector<SnakeSegment> snake;
    SnakeSegment food;
    Uint32 lastUpdateTime = 0;
    float speed = 2.0f;
    int foodEatenCount = 0;  // Đếm số lần rắn ăn thức ăn
    bool isSpecialFood = false;  // Biến kiểm tra thức ăn đặc biệt
    int pendingGrowth = 0;

    SnakeGame() {
        snake.push_back({SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f});
        spawnFood();
        lastUpdateTime = SDL_GetTicks();
    }


    void handleInput(SDL_Event &e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:    if (direction != DIR_DOWN)  nextDirection = DIR_UP;    break;
                case SDLK_DOWN:  if (direction != DIR_UP)    nextDirection = DIR_DOWN;  break;
                case SDLK_LEFT:  if (direction != DIR_RIGHT) nextDirection = DIR_LEFT;  break;
                case SDLK_RIGHT: if (direction != DIR_LEFT)  nextDirection = DIR_RIGHT; break;
            }
        }
    }

    void update() {
    Uint32 currentTime = SDL_GetTicks();
    lastUpdateTime = currentTime;

    SnakeSegment &head = snake.front();
    SnakeSegment newHead = head;

    if (fabs(fmod(head.x, GRID_SIZE)) < speed && fabs(fmod(head.y, GRID_SIZE)) < speed) {
        direction = nextDirection;
    }

    switch (direction) {
        case DIR_UP:    newHead.y -= speed; break;
        case DIR_DOWN:  newHead.y += speed; break;
        case DIR_LEFT:  newHead.x -= speed; break;
        case DIR_RIGHT: newHead.x += speed; break;
    }

    if (checkCollision(newHead)) {
        isRunning = false;
        return;
    }

    snake.insert(snake.begin(), newHead);

    if (checkFoodCollision(newHead)) {
        if (isSpecialFood) {
            pendingGrowth = snake.size() * 2;
        }
        spawnFood();
        if (speed < 6.0f) speed += 0.001f;
    } else {
        if (pendingGrowth > 0) {
            pendingGrowth--;
        } else {
            snake.pop_back();
        }
    }
}


        bool checkFoodCollision(const SnakeSegment &head) {
        SDL_Rect headRect = {static_cast<int>(head.x), static_cast<int>(head.y), GRID_SIZE, GRID_SIZE};
        SDL_Rect foodRect = {static_cast<int>(food.x), static_cast<int>(food.y), GRID_SIZE, GRID_SIZE};
        if (SDL_HasIntersection(&headRect, &foodRect)) {
            foodEatenCount++;
            return true;
        }
        return false;
    }

    bool checkCollision(const SnakeSegment &head) {
    if (head.x < 0 || head.y < 0 || head.x >= SCREEN_WIDTH || head.y >= SCREEN_HEIGHT)
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
    food.x = (rand() % (SCREEN_WIDTH / GRID_SIZE)) * GRID_SIZE;
    food.y = (rand() % (SCREEN_HEIGHT / GRID_SIZE)) * GRID_SIZE;

    // Chỉ đánh dấu special food cho lần tiếp theo
    if ((foodEatenCount+1) % 6 == 0) {
        isSpecialFood = true;
    } else {
        isSpecialFood = false;
    }
}


     void renderCircle(SDL_Renderer *renderer, int x, int y, int r, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int w = 0; w < r * 2; w++) {
            for (int h = 0; h < r * 2; h++) {
                int dx = r - w;
                int dy = r - h;
                if ((dx * dx + dy * dy) <= (r * r)) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
    void render(Graphics &graphics) {
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        SDL_RenderClear(graphics.renderer);

        for (size_t i = 0; i < snake.size(); i++) {
            int r = (i == 0) ? GRID_SIZE / 2 : GRID_SIZE / 2 - 2;
            renderCircle(graphics.renderer, (int)snake[i].x + GRID_SIZE / 2, (int)snake[i].y + GRID_SIZE / 2, r, {0, 255, 0, 255});
            if (i == 0) { // Vẽ mắt cho đầu rắn
                int eyeOffsetX = (direction == DIR_LEFT) ? -5 : (direction == DIR_RIGHT) ? 5 : 0;
                int eyeOffsetY = (direction == DIR_UP) ? -5 : (direction == DIR_DOWN) ? 5 : 0;

                renderCircle(graphics.renderer, (int)snake[i].x + GRID_SIZE / 2 + eyeOffsetX,
                             (int)snake[i].y + GRID_SIZE / 2 + eyeOffsetY - 3, 3, {255, 255, 255, 255});
                renderCircle(graphics.renderer, (int)snake[i].x + GRID_SIZE / 2 + eyeOffsetX,
                             (int)snake[i].y + GRID_SIZE / 2 + eyeOffsetY + 3, 3, {255, 255, 255, 255});
            }
        }

        SDL_Color foodColor = isSpecialFood ? SDL_Color{255, 165, 0, 255} : SDL_Color{255, 0, 0, 255};
        int foodSize = isSpecialFood ? GRID_SIZE / 2 : GRID_SIZE / 3;

        renderCircle(graphics.renderer, food.x + GRID_SIZE / 2, food.y + GRID_SIZE / 2, foodSize, foodColor);

        SDL_RenderPresent(graphics.renderer);
    }
};




#endif
*/
