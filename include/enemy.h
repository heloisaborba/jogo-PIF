// enemy.h

#ifndef ENEMY_H
#define ENEMY_H

typedef struct {
    int x, y;
    int health;
    int active;
    int currentWaypoint; // NOVO: Índice do ponto de destino atual
} Enemy;

Enemy InitEnemy(int x, int y);
void UpdateEnemy(Enemy *e);
void DrawEnemy(Enemy e);
int EnemyReachedTower(Enemy e);

#endif