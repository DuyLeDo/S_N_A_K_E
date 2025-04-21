#ifndef _more_effective_H
#define _more_effective_H
#include <unordered_set>
#include <functional>  // Cho std::hash
using namespace std;
// Cấu trúc GridPos để lưu tọa độ theo lưới
struct GridPos {
    int x, y;

    bool operator==(const GridPos &other) const {
        return x == other.x && y == other.y;
    }

    // 👇 Thêm dòng này vào:
    bool operator!=(const GridPos &other) const {
        return !(*this == other);
    }
};


// Hàm băm cho GridPos
struct GridPosHash {
    size_t operator()(const GridPos &pos) const {
        return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1);
    }
};
#endif
