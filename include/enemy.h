// enemy.h (VERSÃO ATUALIZADA)

#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h> 
#include "raylib.h" // Necessário para Vector2 (se já não estiver lá)

// Definição dos Tipos de Inimigos (Enum já fornecido)
typedef enum {
    INIMIGO_GOBLIN,
    INIMIGO_SPECTRO,
    INIMIGO_NECROMANTE,
    INIMIGO_DRAGAO,
    NUM_ENEMY_TYPES
} EnemyType;

// Definição dos Tipos de Heróis (Precisa existir no seu código para referenciar a resistência)
typedef enum {
    HERO_GUERREIRO,
    HERO_BARDO,
    HERO_PALADINO, // Usado na resistência do Necromante
    HERO_MAGO,     // Usado na resistência do Espectro e Necromante
    NUM_HERO_TYPES
} HeroType;

// Estrutura para configurar as estatísticas base
typedef struct {
    int damage;
    float speed;
    int maxHealth;
    int recompensa;
    int range; // Alcance de ataque
    int resistance; // Bitmask para heróis que podem atacá-lo
    bool special_ability; // Se possui habilidade especial (cura/queima)
} EnemyConfig;

// Estrutura do Inimigo no Jogo
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
    int resistance; // Bitmask: 1=Guerreiro, 2=Bardo, 4=Paladino, 8=Mago (Exemplo)
    
    // Novas variáveis para Status e Habilidades Especiais
    bool is_burning;
    float burning_timer; // Tempo restante para queimar
    float necromante_heal_timer; // Tempo até a próxima cura (Necromante)
} Enemy;

// Declaração de Inimigos
Enemy InitEnemy(float x, float y, EnemyType type);
void UpdateEnemy(Enemy *e);
void DrawEnemy(Enemy e);
int EnemyReachedTower(Enemy e);

// Declaração da tabela de estatísticas (implementada em enemy.c)
// ⭐️ ADICIONE ESTAS DECLARAÇÕES EXTERNAS   
extern const int NUM_WAYPOINTS;
extern Vector2 path[];

// ⭐️ ADICIONE ESTA DECLARAÇÃO DE FUNÇÃO
float Vector2Distance(Vector2 v1, Vector2 v2);
extern const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES];

#endif