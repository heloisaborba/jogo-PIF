// hero.h

#ifndef HERO_H
#define HERO_H

// Definição da estrutura para o Herói Principal (Protagonista ou Castelo Reforçado)
typedef struct {
    int x, y;
    int health;
    int maxHealth; // Adicionado para rastrear vida máxima
    
    // Status de Queima (para interação com Dragão)
    bool is_burning;
    float burn_timer;
    
    // Se o herói principal lutar, adicione:
    // int damage;
    // int range; 
    
} Hero;

// Funções do Herói Principal
Hero InitHero(int x, int y, int initialHealth);
void UpdateHero(Hero *h);
void DrawHero(Hero h);

// Variável global do herói principal
extern Hero hero;

#endif