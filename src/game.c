#include <raylib.h>
#include "game.h"
#include "tower.h"
#include "enemy.h"
#include "hero.h"

Texture2D background;

void InitGame() {
    // Carregar imagem do fundo
    background = LoadTexture("resources/background.png");
}

void DrawGame() {
    // Desenhar o fundo
    DrawTexture(background, 0, 0, WHITE);

    // Depois desenha heróis, torres, inimigos por cima
}

void CloseGame() {
    UnloadTexture(background); // liberar memória
}