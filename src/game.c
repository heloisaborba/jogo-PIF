#include "raylib.h"
#include "game.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"
#include <stdio.h>

// Variáveis globais do jogo
Texture2D background;

// Herói
Hero hero;

// Inimigos
#define MAX_ENEMIES 10
Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;

// Torres
#define MAX_TOWERS 5
Tower towers[MAX_TOWERS];
int towerCount = 0;

// Inicializar jogo
void InitGame(void) {
    printf("DEBUG: Inicializando jogo...\n");
    
    // Carregar background JPG
    background = LoadTexture("resources/background.jpg");
    if (background.id == 0) {
        printf("ERRO: Não foi possível carregar background.jpg\n");
        printf("DEBUG: Verifique se resources/background.jpg existe\n");
    } else {
        printf("DEBUG: Background carregado: %dx%d\n", background.width, background.height);
    }

    // Inicializar herói no canto inferior esquerdo
    hero = InitHero(100, 500);

    // Inicializar inimigos - vindo da direita
    enemyCount = 3;
    for (int i = 0; i < enemyCount; i++) {
        enemies[i] = InitEnemy(900 + i * 100, 300); // Começam fora da tela à direita
    }

    // Inicializar torres em posições estratégicas
    towerCount = 2;
    towers[0] = InitTower(400, 300); // Torre no meio
    towers[1] = InitTower(600, 200); // Torre mais à frente
}

// Atualizar jogo
// Atualizar jogo
void UpdateGame(void) {
    // Futuro: adicionar lógica do jogo aqui
    // Por enquanto não faz nada, apenas mantém o loop funcionando
    
    // Você pode remover todo o código abaixo ou deixar comentado para usar depois:
    
    /*
    // Atualizar herói (movimento com teclado)
    UpdateHero(&hero);

    // Atualizar inimigos (andam para a esquerda)
    for (int i = 0; i < enemyCount; i++) {
        UpdateEnemy(&enemies[i]);
        
        // Se inimigo saiu da tela pela esquerda, reposiciona à direita
        if (enemies[i].x < -50) {
            enemies[i].x = 850;
            enemies[i].y = 100 + (i * 100); // Posição Y variada
        }
    }
    */
}

// Desenhar jogo
// Desenhar jogo
void DrawGame(void) {
    // Obter o tamanho atual da janela
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    BeginDrawing();
    
    // Limpar tela com fallback
    ClearBackground(BLACK);

    // Desenhar background redimensionado para CABER na tela sem zoom excessivo
    if (background.id != 0) {
        // Calcular escala para CABER na tela (usar a MENOR escala)
        float scaleX = (float)screenWidth / background.width;
        float scaleY = (float)screenHeight / background.height;
        float scale = (scaleX < scaleY) ? scaleX : scaleY; // Usar a MENOR escala
        
        int scaledWidth = background.width * scale;
        int scaledHeight = background.height * scale;
        
        // Centralizar a imagem
        int posX = (screenWidth - scaledWidth) / 2;
        int posY = (screenHeight - scaledHeight) / 2;
        
        DrawTextureEx(background, (Vector2){posX, posY}, 0.0f, scale, WHITE);
    } else {
        // Fallback visual se o background não carregar
        ClearBackground(DARKGRAY);
        DrawText("BACKGROUND NAO CARREGADO - Use ESC para menu", 100, 300, 20, RED);
    }

    // UI básica - ajustar posição relativa ao tamanho da tela
    DrawText("TOWER DEFENSE - Press ESC to return to menu", 10, 10, 20, YELLOW);

    EndDrawing();
}

// Fechar jogo
void CloseGame(void) {
    printf("DEBUG: Fechando jogo...\n");
    
    // Liberar background apenas se foi carregado
    if (background.id != 0) {
        UnloadTexture(background);
    }
}