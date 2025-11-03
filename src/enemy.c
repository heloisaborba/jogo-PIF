#include <raylib.h>
#include "enemy.h"
#include "hero.h"    // se os inimigos interagem com o herói

Enemy InitEnemy(int x, int y) {
    Enemy e = { x, y, 50 };
    return e;
}

void UpdateEnemy(Enemy *e) {
    e->x += 1; // inimigo se move para direita
}

void DrawEnemy(Enemy e) {
    DrawRectangle(e.x, e.y - 15, 30, 30, RED);
}
