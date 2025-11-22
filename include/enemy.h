// enemy.h (VERSÃO ATUALIZADA COM CAMINHOS)

#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h> 
#include "raylib.h"

// Definição dos Tipos de Inimigos
typedef enum {
    INIMIGO_GOBLIN,
    INIMIGO_SPECTRO,
    INIMIGO_NECROMANTE,
    INIMIGO_DRAGAO,
    NUM_ENEMY_TYPES
} EnemyType;

// Definição dos Tipos de Heróis
typedef enum {
    HERO_GUERREIRO,
    HERO_BARDO,
    HERO_PALADINO,
    HERO_MAGO,
    NUM_HERO_TYPES
} HeroType;

// Estrutura para configurar as estatísticas base
typedef struct {
    int damage;
    float speed;
    int maxHealth;
    int recompensa;
    int range;
    int resistance;
    bool special_ability;
} EnemyConfig;

// Estrutura do Inimigo no Jogo (ATUALIZADA)
typedef struct {
    int active;
    EnemyType type;
    float x;
    float y;
    int health;
    int maxHealth;
    int damage;
    float speed;
    int currentWaypoint;
    int recompensa_moedas;
    int range;
    int resistance;
    
    // ⭐️ NOVOS CAMPOS PARA MULTIPLOS CAMINHOS
    int pathIndex;           // Índice do caminho (0 = inferior, 1 = superior)
    
    // Status e Habilidades Especiais
    bool is_burning;
    float burning_timer;
    float necromante_heal_timer;
} Enemy;

// Declaração de Inimigos
Enemy InitEnemy(float x, float y, EnemyType type);
void UpdateEnemy(Enemy *e);
void DrawEnemy(Enemy e);
int EnemyReachedTower(Enemy e);

// Declaração da tabela de estatísticas
extern const int NUM_WAYPOINTS;
extern Vector2 path[];

// Declaração de função
float Vector2Distance(Vector2 v1, Vector2 v2);
extern const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES];

#endif