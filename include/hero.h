
#ifndef HERO_H
#define HERO_H

typedef struct {
    int x, y;
    int health;
    int maxHealth; 
    
    bool is_burning;
    float burn_timer;
    
   
} Hero;

Hero InitHero(int x, int y, int initialHealth);
void UpdateHero(Hero *h);
void DrawHero(Hero h);

extern Hero hero;

#endif