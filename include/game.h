// game.h

#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Definição do Waypoint aqui para ser globalmente acessível
#define MAX_WAYPOINTS 9

// O array 'path' será definido em game.c
extern Vector2 path[MAX_WAYPOINTS]; 

void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);

#endif