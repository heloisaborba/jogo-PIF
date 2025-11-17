#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include <math.h>

// Funções úteis que podem ser usadas no projeto
float Vector2Distance(Vector2 v1, Vector2 v2);
int CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);
float GetFrameTime(void); // Se já não estiver disponível

// Outras funções utilitárias que você precise
#endif