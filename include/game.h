// game.h (VERSÃO CORRIGIDA)

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "recursos.h"

#define CASTLE_MAX_HEALTH 100
#define ENEMY_DAMAGE_TO_CASTLE 20

#define MAX_WAYPOINTS 84
#define MAX_ENEMIES 20
#define MAX_HEROIS 4

// 1. Definição do ENUM GameState (DEVE VIR PRIMEIRO)
typedef enum {
    MENU,
    PLAYING,
    PAUSED, 
    GAME_OVER,
    WAVE_WON
} GameState;

// 2. Declaração da variável global (DEPOIS da definição do tipo)
// REMOVEMOS A DECLARAÇÃO DUPLICADA DO FINAL
extern GameState current_game_state;

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
    int health;
    float lastAttackTime;
    Texture2D texture;
} PlacedHero;

// Declaração do caminho
extern Vector2 path[MAX_WAYPOINTS];

// Variáveis globais de estado
extern int enemies_defeated_count; // Contador para a condição de vitória
extern int towerHealth;

// Declarações das funções do jogo
void IniciarFase2(void);
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);
void InicializarHerois(void);
int ComprarHeroiEspecifico(recursos *r, int tipoHeroi);
void DrawMenuHerois(void);
void VerificarCliqueMenu(void);
void DrawGameUI(void);
void ReiniciarFase(void);
void VoltarMenuPrincipal(void);
void DrawPause(void);


#endif // GAME_H