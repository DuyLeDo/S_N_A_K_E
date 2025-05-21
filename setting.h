#ifndef _setting_game_H
#define _setting_game_H

void renderInstructionsScreen(Graphics &graphics, TTF_Font* font) {
    static SDL_Texture* bg = nullptr;
    if (!bg) bg = graphics.loadTexture(INSTRUCTION_JPG); // có thể dùng nền tùy ý
    SDL_RenderCopy(graphics.renderer, bg, nullptr, nullptr);

    vector<string> lines = {
        "INSTRUCTIONS",
        "",
        "M - TURN OFF VOLUMEMusic",
        "N - TURN OFF VOLUMEEFFECT",
        "P - PAUSE",
        "Arrow Keys - Move the snake",
        "Shift       - Boost speed",
        "",
        "Red Food    - +10 points",
        "Orange Food - +50 points & huge growth",
        "",
        "Avoid walls and your own body!",
        "",
        "Press any key to go back..."
    };

    int y = 100;
    for (const string& line : lines) {
            if (line.empty()) {
            y += 30; // thêm khoảng cách nếu dòng rỗng
            continue;
        }
        SDL_Color color = {255, 255, 255, 255};
        TTF_Font* textFont = graphics.loadFont("assets/Purisa-BoldOblique.ttf", line == "INSTRUCTIONS" ? 28 : 20);
        SDL_Texture* tex = graphics.renderText(line.c_str(), textFont, color);
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        graphics.renderTexture(tex, (SCREEN_WIDTH - w) / 2, y);
        SDL_DestroyTexture(tex);
        y += (line == "INSTRUCTIONS") ? 50 : 30;
    }
}




#endif
