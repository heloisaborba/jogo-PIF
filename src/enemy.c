// enemy.c - Versão com movimento suave
#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include <math.h>

#define ENEMY_SPEED 100.0f  // Em pixels por segundo
#define WAYPOINT_THRESHOLD 3.0f

extern Vector2 path[MAX_WAYPOINTS];

Enemy InitEnemy(int x, int y) {
    Enemy e = { 
        .x = x, 
        .y = y, 
        .health = 50, 
        .active = 1, 
        .currentWaypoint = 0,
        .recompensa_moedas = 50
    };
    return e;
}

void UpdateEnemy(Enemy *e) {
    if (!e->active) return;
    
    if (e->currentWaypoint >= MAX_WAYPOINTS - 1) return;

    int targetIndex = e->currentWaypoint + 1;
    Vector2 target = path[targetIndex];
    
    float dx = target.x - e->x;
    float dy = target.y - e->y;
    float distance = sqrtf(dx*dx + dy*dy);

    if (distance > WAYPOINT_THRESHOLD) {
        // Movimento baseado em tempo (mais suave)
        float factor = (ENEMY_SPEED * GetFrameTime()) / distance;
        e->x += dx * factor;
        e->y += dy * factor;
    } else {
        // Avança para o próximo waypoint
        e->currentWaypoint++;
        e->x = target.x;
        e->y = target.y;
    }
}

void DrawEnemy(Enemy e) {
    if (!e.active) return;
    
    // Inimigo mais visual
    DrawCircle(e.x, e.y, 15, RED);
    DrawCircle(e.x, e.y, 12, MAROON);
    
    // Barra de vida
    DrawRectangle(e.x - 15, e.y - 25, 30, 5, BLACK);
    DrawRectangle(e.x - 15, e.y - 25, (30 * e.health) / 50, 5, GREEN);
}

int EnemyReachedTower(Enemy e) {
    return (e.currentWaypoint >= MAX_WAYPOINTS - 1);
}