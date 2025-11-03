#ifndef ENEMY_H
#define ENEMY_H

typedef struct {
    int x, y;
    int health;
} Enemy;

Enemy InitEnemy(int x, int y);
void UpdateEnemy(Enemy *e);
void DrawEnemy(Enemy e);

#endif
