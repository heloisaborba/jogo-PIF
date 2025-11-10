// game.c

#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include <stdio.h>

#define MAX_ENEMIES 5

#define MAX_WAYPOINTS 9

Vector2 path[MAX_WAYPOINTS] = {
    {10, 440},      // 0: Início (não alterado)
    
    // --- Nova Curva 1 (Horizontal Inferior) ---
    {120, 440},     // 1: Início da curva para baixo
    {150, 520},     // 2: NOVO PONTO (Topo da primeira curva vermelha)
    {400, 520},     // 3: AJUSTADO FINAL: Seção horizontal inferior (X movido de 410 para 400)
    {400, 450},     // 4: AJUSTADO: Final da curva inferior (X movido de 410 para 400)
    
    // --- Caminho Original Ajustado ---
    {400, 200},     // 5: Subida (X ajustado para 400 para manter o alinhamento vertical)
    {650, 200},     // 6: Curva superior
    {700, 180},     // 7: Preparação para ponte
    {700, 130},     // 8: Ponte
    {750, 100},
};

static Enemy enemies[MAX_ENEMIES];
static int enemyCount = 0;
static Texture2D background;
static Texture2D towerTexture;
static int towerHealth = 100;

// Inicialização
void InitGame(void) {
    // Carrega as texturas
    background = LoadTexture("resources/background.jpg");
    towerTexture = LoadTexture("resources/tower.png");
    
    towerHealth = 100;
    enemyCount = 1;

    for (int i = 0; i < enemyCount; i++) {
        // Inicializa o inimigo na primeira posição do caminho
        enemies[i] = InitEnemy((int)path[0].x, (int)path[0].y); 
    }
}

// Atualização
void UpdateGame(void) {
    // Dentro de UpdateGame()
    for (int i = 0; i < enemyCount; i++) {
        UpdateEnemy(&enemies[i]);
        
        if (EnemyReachedTower(enemies[i]) && enemies[i].active) { // Verifica se chegou e ainda está ativo
            towerHealth -= 1; 
            enemies[i].active = 0; // Desativa ele APÓS causar o dano
        }
    }

    if (towerHealth <= 0) {
        towerHealth = 0;
    }
}

// Desenho
void DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    // 🔹 Fundo
    DrawTexturePro(
        background,
        (Rectangle){ 0, 0, background.width, background.height }, 
        (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, 
        (Vector2){ 0, 0 }, 
        0.0f, 
        WHITE 
    );

    // 🎯 DEBUG: Desenha o caminho dos inimigos (adicione estas linhas)
    for (int i = 0; i < MAX_WAYPOINTS - 1; i++) {
        DrawLineEx(path[i], path[i + 1], 3.0f, (Color){255, 255, 0, 128});
    }
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        DrawCircle(path[i].x, path[i].y, 5.0f, BLUE);
        DrawText(TextFormat("%d", i), path[i].x + 10, path[i].y - 10, 10, WHITE);
    }

    // 🔹 Torre
    DrawTexture(towerTexture, 650, 100, WHITE);
    DrawText(TextFormat("Tower HP: %d", towerHealth), 20, 20, 20, BLACK);

    // 🔹 Inimigos
    for (int i = 0; i < enemyCount; i++) {
        DrawEnemy(enemies[i]);
    }

    EndDrawing();
}

// Finalização
void CloseGame(void) {
    UnloadTexture(background);
    UnloadTexture(towerTexture);
}