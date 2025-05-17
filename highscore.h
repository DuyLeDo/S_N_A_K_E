#ifndef HIGH_SCORE_
#define HIGH_SCORE_
#include <fstream>  // Để làm việc với tệp

int readHighScore() {
    ifstream file("highscore.txt");
    int highscore = 0;

    if (file.is_open()) {
        file >> highscore;

        if (file.fail()) {
            // Nếu đọc lỗi (ví dụ file không chứa số), reset về 0
            highscore = 0;
        }

        file.close();
    }

    return highscore;
}


void writeHighScore(int score) {
    if (score < 0) score = 0;  // Đảm bảo không ghi số âm

    ofstream file("highscore.txt");
    if (file.is_open()) {
        file << score;
        file.close();
    }
}


#endif // HIGH_SCORE_
