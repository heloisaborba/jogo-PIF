#include "enemy.h"
#include "game.h"    // ⭐️ Para acessar NUM_WAYPOINTS e currentPath
#include <stdlib.h>
#include <math.h>    // ⭐️ Para sqrtf

// Tabela de estatísticas dos inimigos (deve corresponder ao enum EnemyType)
const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES] = {
    // INIMIGO_GOBLIN - Pode ser atingido por TODOS os heróis
    {
        .damage = 10,
        .speed = 2.0f,
        .maxHealth = 100,
        .recompensa = 5,
        .range = 50,
        .resistance = 0, // Nenhuma resistência (todos podem atacar)
        .special_ability = false
    },
    // INIMIGO_SPECTRO - Só pode ser atingido pelo MAGO
    {
        .damage = 8,
        .speed = 2.5f,
        .maxHealth = 80,
        .recompensa = 8,
        .range = 60,
        .resistance = 7, // 1 (Guerreiro) + 2 (Bardo) + 4 (Paladino) = 7
        .special_ability = false
    },
    // INIMIGO_NECROMANTE - Só pode ser atingido pelo MAGO e PALADINO
    {
        .damage = 15,
        .speed = 1.5f,
        .maxHealth = 150,
        .recompensa = 15,
        .range = 70,
        .resistance = 3, // 1 (Guerreiro) + 2 (Bardo) = 3
        .special_ability = true
    },
    // INIMIGO_DRAGAO - Pode ser atingido por TODOS os heróis
    {
        .damage = 25,
        .speed = 1.8f,
        .maxHealth = 300,
        .recompensa = 30,
        .range = 80,
        .resistance = 0, // Nenhuma resistência (todos podem atacar)
        .special_ability = true
    }
};

Enemy InitEnemy(float x, float y, EnemyType type) {
    Enemy enemy;
    
    // Inicialização básica
    enemy.active = true;
    enemy.type = type;
    enemy.x = x;
    enemy.y = y;
    enemy.currentWaypoint = 0;
    
    // Configura estatísticas baseadas na tabela
    enemy.damage = ENEMY_STATS[type].damage;
    enemy.speed = ENEMY_STATS[type].speed;
    enemy.maxHealth = ENEMY_STATS[type].maxHealth;
    enemy.health = enemy.maxHealth;
    enemy.recompensa_moedas = ENEMY_STATS[type].recompensa;
    enemy.range = ENEMY_STATS[type].range;
    enemy.resistance = ENEMY_STATS[type].resistance;
    
    // Inicializa status especiais
    enemy.is_burning = false;
    enemy.burning_timer = 0.0f;
    enemy.necromante_heal_timer = 0.0f;
    
    return enemy;
}

void UpdateEnemy(Enemy *e) {
    if (!e->active) return;
    
    // Movimento pelos waypoints
    if (e->currentWaypoint < NUM_WAYPOINTS - 1) {
        Vector2 target = currentPath[e->currentWaypoint + 1];
        Vector2 direction = {
            target.x - e->x,
            target.y - e->y
        };
        
        float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
        
        if (distance < e->speed) {
            // Chegou no waypoint
            e->currentWaypoint++;
            e->x = target.x;
            e->y = target.y;
        } else {
            // Move em direção ao waypoint
            direction.x /= distance;
            direction.y /= distance;
            
            e->x += direction.x * e->speed;
            e->y += direction.y * e->speed;
        }
    }
    
    // Atualiza status especiais
    if (e->is_burning) {
        e->burning_timer -= GetFrameTime();
        if (e->burning_timer <= 0) {
            e->is_burning = false;
        }
    }
    
    if (e->type == INIMIGO_NECROMANTE) {
        e->necromante_heal_timer -= GetFrameTime();
    }
}

void DrawEnemy(Enemy e) {
    if (!e.active) return;
    
    Color color;
    switch(e.type) {
        case INIMIGO_GOBLIN: color = GREEN; break;
        case INIMIGO_SPECTRO: color = GRAY; break;
        case INIMIGO_NECROMANTE: color = PURPLE; break;
        case INIMIGO_DRAGAO: color = RED; break;
        default: color = WHITE;
    }
    
    // Desenha inimigo como círculo (substitua por sprites depois)
    DrawCircle(e.x, e.y, 15, color);
    
    // Desenha barra de vida
    float healthPercent = (float)e.health / (float)e.maxHealth;
    DrawRectangle(e.x - 20, e.y - 25, 40, 5, GRAY);
    DrawRectangle(e.x - 20, e.y - 25, 40 * healthPercent, 5, GREEN);
    
    // Indicador de queima
    if (e.is_burning) {
        DrawCircle(e.x + 15, e.y - 15, 5, ORANGE);
    }
}

int EnemyReachedTower(Enemy e) {
    // Verifica se chegou no último waypoint (próximo à torre)
    // Ou se está muito próximo das coordenadas finais da torre
    return (e.currentWaypoint >= NUM_WAYPOINTS - 1) || 
           (Vector2Distance((Vector2){e.x, e.y}, (Vector2){650, 100}) < 50.0f);
}