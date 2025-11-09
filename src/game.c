// game.c

#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include <stdio.h>

#define MAX_ENEMIES 5

Vector2 path[MAX_WAYPOINTS] = {
// INÍCIO - NÃO ALTERAR, parte perfeita
    {50, 440},  

    // Ponto 1: Entra na seção reta (X=150)
    {150, 450}, 

    // Ponto 2: Meio da seção reta (Y é quase o mesmo)
    {400, 450}, // Y ajustado para 450 (era 480/540)

    // Ponto 3: Topo da Coluna Vertical (Início da subida principal)
    {400, 200}, 

    // Ponto 4: Vira para a seção horizontal superior
    {650, 200}, 
    
    // Ponto 5: Curva para a Ponte
    {700, 180},
    
    // Ponto 6: Meio da Ponte
    {700, 130}, 

    // Ponto 7: Curva final
    {750, 100}, 

    // Ponto 8: Chegada na Torre (posição final)
    {780, 100}  // Fim do Array (Índice 8)
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