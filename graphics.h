#ifndef _GRAPHIC__H
#define _GRAPHIC__H
#include <SDL_ttf.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "defs.h"

struct ScrollingBackground {
    SDL_Texture* texture;
    int scrollingOffset = 0;
    int width, height;

    void setTexture(SDL_Texture* _texture) {
        texture = _texture;
        if (texture) {
            SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        }
    }

    void scroll(int distance) {
        scrollingOffset -= distance;
        if (scrollingOffset < -height) { scrollingOffset = 0; }
    }
};

struct Graphics {
    SDL_Renderer *renderer;
    SDL_Window *window;

    // Tài nguyên âm thanh
    Mix_Music *backgroundMusic = nullptr;
    Mix_Chunk *eatSound = nullptr;
    Mix_Chunk *gameOverSound = nullptr;

    // Biến cho hàm box
    int X = 200, Y = 400;
    int size = 10;

    void logErrorAndExit(const char* msg, const char* error) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "%s: %s", msg, error);
        SDL_Quit();
    }

    void init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            logErrorAndExit("SDL_Init", SDL_GetError());
        }

        window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == nullptr) logErrorAndExit("CreateWindow", SDL_GetError());

        if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG)) {
            logErrorAndExit("SDL_image error:", IMG_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) logErrorAndExit("CreateRenderer", SDL_GetError());

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Khởi tạo SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            logErrorAndExit("Mix_OpenAudio", Mix_GetError());
        }
        Mix_AllocateChannels(4);

        // Load tài nguyên âm thanh
        backgroundMusic = loadMusic("assets\\RunningAway.mp3");
        eatSound = loadSound("assets\\jump.wav");
        gameOverSound = loadSound("assets\\jump.wav");
        playMusic(backgroundMusic); // Phát nhạc nền ngay khi khởi tạo
        if (TTF_Init() == -1) {
            logErrorAndExit("SDL_ttf could not initialize! SDL_ttf Error: ",
                             TTF_GetError());
        }

    }

    // Destructor để giải phóng tài nguyên âm thanh
    ~Graphics() {
        if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
        if (eatSound) Mix_FreeChunk(eatSound);
        if (gameOverSound) Mix_FreeChunk(gameOverSound);
    }

    // Hàm frame
    void frame() {
        SDL_Rect rect = {0, 150, 400, 561};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 10; i++) {
            SDL_RenderDrawRect(renderer, &rect);
            rect.x++;
            rect.y++;
            rect.w -= 2;
            rect.h -= 2;
        }
    }

    // Hàm box
    void box() {
        SDL_Rect filled_rect;
        filled_rect.x = X;
        filled_rect.y = Y;
        filled_rect.w = size;
        filled_rect.h = size;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &filled_rect);
    }

    // Hàm load và phát nhạc nền
    Mix_Music *loadMusic(const char* path) {
        Mix_Music *gMusic = Mix_LoadMUS(path);
        if (gMusic == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                           "Could not load music: %s, SDL_mixer Error: %s", path, Mix_GetError());
        }
        return gMusic;
    }

    void playMusic(Mix_Music *gMusic) {
        if (gMusic == nullptr) return;
        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(gMusic, -1); // Lặp vô hạn
        } else if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
        }
    }

    // Hàm load và phát hiệu ứng âm thanh
    Mix_Chunk *loadSound(const char* path) {
        Mix_Chunk *gSound = Mix_LoadWAV(path);
        if (gSound == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                           "Could not load sound: %s, SDL_mixer Error: %s", path, Mix_GetError());
        }
        return gSound;
    }

    void playSound(Mix_Chunk *gSound) {
        if (gSound != nullptr) {
            Mix_PlayChannel(-1, gSound, 0); // Phát trên kênh tự do, không lặp
        }
    }

    // Hàm load và render texture
    SDL_Texture *loadTexture(const char *filename) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);
        SDL_Texture *texture = IMG_LoadTexture(renderer, filename);
        if (texture == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                           "Load texture %s: %s", filename, IMG_GetError());
        }
        return texture;
    }

    void renderTexture(SDL_Texture *texture, int x, int y) {
        if (!texture) return;
        SDL_Rect dest;
        dest.x = x;
        dest.y = y;
        SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
        SDL_RenderCopy(renderer, texture, NULL, &dest);
    }

    void blitRect(SDL_Texture *texture, SDL_Rect *src, int x, int y) {
        if (!texture || !src) return;
        SDL_Rect dest;
        dest.x = x;
        dest.y = y;
        dest.w = src->w;
        dest.h = src->h;
        SDL_RenderCopy(renderer, texture, src, &dest);
    }

    void render(const ScrollingBackground& background) {
        renderTexture(background.texture, 0, background.scrollingOffset);
        renderTexture(background.texture, 0, background.scrollingOffset + background.height);
    }

    void prepareScene(SDL_Texture *background) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);
    }

    void presentScene() {
        SDL_RenderPresent(renderer);
    }

    void quit() {
        TTF_Quit();
        Mix_CloseAudio();
        Mix_Quit();
        IMG_Quit();
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }





    TTF_Font* loadFont(const char* path, int size)
    {
        TTF_Font* gFont = TTF_OpenFont( path, size );
        if (gFont == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                           SDL_LOG_PRIORITY_ERROR,
                           "Load font %s", TTF_GetError());
        }
    }
    SDL_Texture* renderText(const char* text,
                            TTF_Font* font, SDL_Color textColor)
    {
        SDL_Surface* textSurface =
                TTF_RenderText_Solid( font, text, textColor );
        if( textSurface == nullptr ) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                           SDL_LOG_PRIORITY_ERROR,
                           "Render text surface %s", TTF_GetError());
            return nullptr;
        }

        SDL_Texture* texture =
                SDL_CreateTextureFromSurface( renderer, textSurface );
        if( texture == nullptr ) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                           SDL_LOG_PRIORITY_ERROR,
                           "Create texture from text %s", SDL_GetError());
        }
        SDL_FreeSurface( textSurface );
        return texture;
    }


};

#endif
