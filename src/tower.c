#include <raylib.h>
#include "tower.h"
#include "enemy.h"   // se as torres atacam inimigos

Tower InitTower(int x, int y) {
    Tower t = { x, y, 10 };
    return t;
}

void DrawTower(Tower t) {
    DrawRectangle(t.x - 15, t.y - 15, 30, 30, DARKGRAY);
}
