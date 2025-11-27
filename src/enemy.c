#include "enemy.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>

const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES] = {
    {
        .damage = 10,
        .speed = 2.0f,
        .maxHealth = 100,
        .recompensa = 10,
        .range = 50,
        .resistance = 0,
        .special_ability = false
    },
    {
        .damage = 8,
        .speed = 2.5f,
        .maxHealth = 80,
        .recompensa = 15,
        .range = 60,
        .resistance = 7,
        .special_ability = false
    },
    {
        .damage = 15,
        .speed = 1.5f,
        .maxHealth = 150,
        .recompensa = 25,
        .range = 70,
        .resistance = 3,
        .special_ability = true
    },
    {
        .damage = 25,
        .speed = 1.8f,
        .maxHealth = 300,
        .recompensa = 50,
        .range = 80,
        .resistance = 0,
        .special_ability = true
    }
};

Vector2* GetEnemyCurrentPath(Enemy *e, int *pathLength) {
    extern Vector2 pathInferior[];
    extern Vector2 pathSuperior[];
    extern int currentWave;
    if (currentWave == 1) {
        *pathLength = 84;
        return pathInferior;
    } else {
        if (e->pathIndex == 0) {
            *pathLength = 84;
            return pathInferior;
        } else {
            *pathLength = 96;
            return pathSuperior;
        }
    }
}

Vector2 GetEnemyFinalWaypoint(Enemy *e) {
    int pathLength;
    Vector2* path = GetEnemyCurrentPath(e, &pathLength);
    return path[pathLength - 1];
}

Enemy InitEnemy(float x, float y, EnemyType type) {
    Enemy enemy;
    enemy.active = true;
    enemy.type = type;
    enemy.x = x;
    enemy.y = y;
    enemy.currentWaypoint = 0;
    enemy.pathIndex = 0;
    enemy.damage = ENEMY_STATS[type].damage;
    enemy.speed = ENEMY_STATS[type].speed;
    enemy.maxHealth = ENEMY_STATS[type].maxHealth;
    enemy.health = enemy.maxHealth;
    enemy.recompensa_moedas = ENEMY_STATS[type].recompensa;
    enemy.range = ENEMY_STATS[type].range;
    enemy.resistance = ENEMY_STATS[type].resistance;
    enemy.is_burning = false;
    enemy.burning_timer = 0.0f;
    enemy.necromante_heal_timer = 0.0f;
    return enemy;
}

void UpdateEnemy(Enemy *e) {
    if (!e->active) return;
    int pathLength;
    Vector2* currentPath = GetEnemyCurrentPath(e, &pathLength);
    if (e->currentWaypoint < pathLength - 1) {
        Vector2 target = currentPath[e->currentWaypoint + 1];
        Vector2 direction = { target.x - e->x, target.y - e->y };
        float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
        if (distance < e->speed) {
            e->currentWaypoint++;
            e->x = target.x;
            e->y = target.y;
        } else {
            direction.x /= distance;
            direction.y /= distance;
            e->x += direction.x * e->speed;
            e->y += direction.y * e->speed;
        }
    }
    if (e->is_burning) {
        e->burning_timer -= GetFrameTime();
        if (e->burning_timer <= 0) e->is_burning = false;
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
    if (currentWave > 1) {
        Color pathColor = (e.pathIndex == 0) ? BLUE : SKYBLUE;
        DrawCircle(e.x, e.y - 20, 3, pathColor);
    }
    DrawCircle(e.x, e.y, 15, color);
    float healthPercent = (float)e.health / (float)e.maxHealth;
    DrawRectangle(e.x - 20, e.y - 25, 40, 5, GRAY);
    DrawRectangle(e.x - 20, e.y - 25, 40 * healthPercent, 5, GREEN);
    if (e.is_burning) DrawCircle(e.x + 15, e.y - 15, 5, ORANGE);
}

int EnemyReachedTower(Enemy e) {
    int pathLength;
    Vector2* currentPath = GetEnemyCurrentPath((Enemy*)&e, &pathLength);
    Vector2 finalWaypoint = currentPath[pathLength - 1];
    float distanceToFinal = Vector2Distance((Vector2){e.x, e.y}, finalWaypoint);
    return (e.currentWaypoint >= pathLength - 1) || (distanceToFinal < 20.0f);
}

void DrawEnemyDebugInfo(Enemy e, int index) {
    if (!e.active) return;
    const char* pathName = (e.pathIndex == 0) ? "INFERIOR" : "SUPERIOR";
    int pathLength;
    DrawText(TextFormat("Inimigo %d: %s (WP: %d/%d)", index, pathName, e.currentWaypoint, pathLength - 1), 10, 150 + (index * 15), 10, WHITE);
}