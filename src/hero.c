#include <raylib.h>
#include "hero.h"
#include "enemy.h"
#include "tower.h"

// Variável global do herói principal
Hero hero = { 400, 300, 100 };

Hero InitHero(int x, int y, int initialHealth) {
    Hero h = { x, y, initialHealth };  // Mude 100 para initialHealth
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