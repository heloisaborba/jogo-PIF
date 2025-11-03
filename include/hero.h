#ifndef HERO_H
#define HERO_H

typedef struct {
    int x, y;
    int health;
} Hero;

Hero InitHero(int x, int y);
void UpdateHero(Hero *h);
void DrawHero(Hero h);

#endif
