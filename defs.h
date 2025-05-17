#ifndef _DEFS__H /// tránh trường hợp thư viện được khai báo 2 lần
#define _DEFS__H


#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define GRID_SIZE 20
const char* WINDOW_TITLE = "JALLYDOO";
const int DIR_UP = 0;
const int DIR_DOWN = 1;
const int DIR_LEFT = 2;
const int DIR_RIGHT = 3;
#define FOOD_IMG  "img\\Hear_t.png"
#define BACKGROUND_IMG "img\\backgroundimages(2).jpg"
#define START_JPG "img\\snakestart.jpg"
#define GAMEOVER_JPG "img\\snakecry.jpg"
#define PAUSE_JPG "img\\snakepause.jpg"
enum GameState {
    STATE_START,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_PAUSED
};


#endif
