

#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h> 
#include "raylib.h"

typedef enum {
    INIMIGO_GOBLIN,
    INIMIGO_SPECTRO,
    INIMIGO_NECROMANTE,
    INIMIGO_DRAGAO,
    NUM_ENEMY_TYPES
} EnemyType;

typedef enum {
    HERO_GUERREIRO,
    HERO_BARDO,
    HERO_PALADINO,
    HERO_MAGO,
    NUM_HERO_TYPES
} HeroType;

typedef struct {
    int damage;
    float speed;
    int maxHealth;
    int recompensa;
    int range;
    int resistance;
    bool special_ability;
} EnemyConfig;

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
    
    int pathIndex;           
    
    bool is_burning;
    float burning_timer;
    float necromante_heal_timer;
} Enemy;

Enemy InitEnemy(float x, float y, EnemyType type);
void UpdateEnemy(Enemy *e);
void DrawEnemy(Enemy e);
int EnemyReachedTower(Enemy e);

extern const int NUM_WAYPOINTS;
extern Vector2 path[];

float Vector2Distance(Vector2 v1, Vector2 v2);
extern const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES];

#endif