#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"

// Tamanho da tela
const int screenWidth = 800;
const int screenHeight = 600;

// Background
Texture2D background;

// Herói
Hero hero;

// Inimigos
#define MAX_ENEMIES 10
Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;

// Torres (pode adicionar futuramente tiros)
#define MAX_TOWERS 5
Tower towers[MAX_TOWERS];
int towerCount = 0;

// Inicializar jogo
void InitGame() {
    InitWindow(screenWidth, screenHeight, "Tower Defense Medieval");
    SetTargetFPS(60);

    // Carregar fundo
    background = LoadTexture("resources/background.png");

    // Inicializar herói
    hero = InitHero(100, 300);

    // Inicializar inimigos
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = InitEnemy(-50 * i, 300);
        enemyCount++;
    }

    // Inicializar torres (exemplo)
    towers[0] = InitTower(400, 300);
    towerCount = 1;
}

// Atualizar jogo
void UpdateGame() {
    UpdateHero(&hero);

    for (int i = 0; i < enemyCount; i++) {
        UpdateEnemy(&enemies[i]);
    }

    // Futuro: atualizar torres, tiros, colisões etc
}

// Desenhar jogo
void DrawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Desenhar background
    DrawTexture(background, 0, 0, WHITE);

    // Desenhar herói
    DrawHero(hero);

    // Desenhar inimigos
    for (int i = 0; i < enemyCount; i++) {
        DrawEnemy(enemies[i]);
    }

    // Desenhar torres
    for (int i = 0; i < towerCount; i++) {
        DrawTower(towers[i]);
    }

    EndDrawing();
}

// Fechar jogo
void CloseGame() {
    UnloadTexture(background);
    CloseWindow();
}

int main(void) {
    InitGame();

    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }

    CloseGame();
    return 0;
}
