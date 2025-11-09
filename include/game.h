#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"

// Declarações das funções do jogo
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void CloseGame(void);

#endif