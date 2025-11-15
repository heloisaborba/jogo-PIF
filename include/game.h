// game.h
#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "recursos.h"

#define MAX_WAYPOINTS 84  // Mude de 9 para 10
#define MAX_ENEMIES 5
#define MAX_HEROIS 4

// Estrutura para heróis
typedef struct {
    char nome[20];
    int custo;
    int dano;
    int alcance;
    Texture2D texture;
} Heroi;

// Estrutura para heróis colocados no mapa
typedef struct {
    int x, y;
    int tipo; // 0: Guerreiro, 1: Bardo, 2: Paladino, 3: Mago
    int dano;
    int alcance;
    float lastAttackTime;
    Texture2D texture;
} PlacedHero;

// Declaração do caminho com o novo tamanho
extern Vector2 path[MAX_WAYPOINTS];

// Declarações das funções do jogo
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);
void InicializarHerois(void);
int ComprarHeroiEspecifico(recursos *r, int tipoHeroi);
void DrawMenuHerois(void);
void VerificarCliqueMenu(void);
void DrawGameUI(void);

#endif