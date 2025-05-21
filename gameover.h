#ifndef _GAME_Over_H
#define _Game_Over_H
struct FloatingScore {
    float x, y;
    int value;
    float alpha = 255;
    Uint32 startTime;
};

struct SnakeFragment {
    float x, y;
    float vx, vy;        // vận tốc bay
    float alpha = 255;   // độ trong suốt
};
#endif // _GAME_Over
