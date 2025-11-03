#ifndef TOWER_H
#define TOWER_H

typedef struct {
    int x, y;
    int damage;
} Tower;

Tower InitTower(int x, int y);
void DrawTower(Tower t);

#endif
