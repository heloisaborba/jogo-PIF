#include <raylib.h>
#include "hero.h"
#include "enemy.h"   // se o herói ataca inimigos
#include "tower.h"   // se o herói interage com torres

Hero InitHero(int x, int y) {
    Hero h = { x, y, 100 };
    return h;
}

void UpdateHero(Hero *h) {
    if (IsKeyDown(KEY_RIGHT)) h->x += 2;
    if (IsKeyDown(KEY_LEFT))  h->x -= 2;
    if (IsKeyDown(KEY_UP))    h->y -= 2;
    if (IsKeyDown(KEY_DOWN))  h->y += 2;
}

void DrawHero(Hero h) {
    DrawCircle(h.x, h.y, 20, BLUE);
}
