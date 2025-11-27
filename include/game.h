

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "recursos.h"

#define CASTLE_MAX_HEALTH 100
#define ENEMY_DAMAGE_TO_CASTLE 20

#define NUM_WAYPOINTS 84
#define MAX_WAYPOINTS 84
#define MAX_ENEMIES 40
#define HERO_TYPE_COUNT 4

extern Texture2D background;
extern Texture2D backgroundFase2;
extern Texture2D backgroundFase3;
extern Texture2D towerTexture;

extern Vector2 pathInferior[];
extern Vector2 pathSuperior[];
extern int currentWave;
extern int NUM_WAYPOINTS_BAIXO;
extern int NUM_WAYPOINTS_CIMA;

typedef enum {
    MENU,
    PLAYING,
    PAUSED, 
    GAME_OVER,
    WAVE_WON
} GameState;

extern GameState current_game_state;

typedef struct {
    char nome[20];
    int custo;
    int dano;
    int alcance;
    Texture2D texture;
} Heroi;

typedef struct {
    int nivel_vida;
    int nivel_dano;
    int nivel_velocidade;
    int vida_total;
    int dano_total;
    float velocidade_mult;
} HeroUpgrade;

typedef struct {
    float x, y;
    int tipo;
    int dano;
    int alcance;
    int health;
    float lastAttackTime;
    Texture2D texture;
    bool is_burning;
    float burn_timer;
} PlacedHero;

extern Heroi herois[HERO_TYPE_COUNT];
extern HeroUpgrade heroUpgrades[HERO_TYPE_COUNT];

extern PlacedHero *placedHeroes;
extern int placedHeroCount;
extern int placedHeroCapacity;

extern int enemies_defeated_count;
extern int towerHealth;

extern char playerName[64];

void IniciarFase2(void);
void IniciarFase3(void);
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);
void ResetGame(void);
void InicializarHerois(void);
int ComprarHeroiEspecifico(recursos *r, int tipoHeroi);
void DrawMenuHerois(void);
void VerificarCliqueMenu(void);
void DrawGameUI(void);
void ReiniciarFase(void);
void VoltarMenuPrincipal(void);
void DrawPause(void);

#endif 