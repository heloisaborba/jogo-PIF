// game.h (VERSÃO CORRIGIDA)

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "recursos.h"

#define CASTLE_MAX_HEALTH 100
#define ENEMY_DAMAGE_TO_CASTLE 20

#define NUM_WAYPOINTS 84
#define MAX_WAYPOINTS 84
#define MAX_ENEMIES 20
#define MAX_HEROIS 4

// NO ARQUIVO .h (ex: include/game.h)
extern Texture2D background;
extern Texture2D backgroundFase2;
extern Texture2D backgroundFase3;
extern Texture2D towerTexture;

// Adicione estas declarações externas para que enemy.c possa acessar
extern Vector2 pathInferior[];
extern Vector2 pathSuperior[];
extern int currentWave;
extern int NUM_WAYPOINTS_BAIXO;
extern int NUM_WAYPOINTS_CIMA;

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

// Estrutura para upgrades de herói
typedef struct {
    int nivel_vida;      // Nível de upgrade de vida (0-5)
    int nivel_dano;      // Nível de upgrade de dano (0-5)
    int nivel_velocidade; // Nível de upgrade de velocidade (0-5)
    int vida_total;      // Vida total (vida base + upgrade)
    int dano_total;      // Dano total (dano base + upgrade)
    float velocidade_mult; // Multiplicador de velocidade (padrão 1.0)
} HeroUpgrade;

// Estrutura para heróis colocados no mapa
typedef struct {
    float x, y;
    int tipo;
    int dano;
    int alcance;
    int health;
    float lastAttackTime;
    Texture2D texture;
    bool is_burning;      // Para efeito de queima do dragão
    float burn_timer;     // Tempo restante de queima
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

// Nome do jogador atual (preenchido antes de iniciar o jogo)
extern char playerName[64];


#endif // GAME_H