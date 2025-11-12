#pragma once
#include <stdint.h>

typedef struct { int x, y; } Coord;

static inline int coord_valid(int x, int y) {
    return (x >= 0 && x < 9 && y >= 0 && y < 10);
}
