#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"
#include "menu.h"

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
    // Janela e FPS já iniciados no menu, não repetir InitWindow
    background = LoadTexture("resources/background.png");

    // Inicializar herói
    hero = InitHero(100, 300);

    // Inicializar inimigos
    enemyCount = 0;
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
}

// ==================== MAIN ====================

int main(void) {
    InitWindow(screenWidth, screenHeight, "Tower Defense Medieval");
    SetTargetFPS(60);

    while (true) {
        MenuOption option = ShowMenu();

        if (option == MENU_EXIT)
            break;

        else if (option == MENU_START) {
            InitGame();

            while (!WindowShouldClose()) {
                if (IsKeyPressed(KEY_ESCAPE)) break; // voltar ao menu
                UpdateGame();
                DrawGame();
            }

            CloseGame();
        }

        else if (option == MENU_HOW_TO_PLAY) {
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(DARKBLUE);
                DrawText("HOW TO PLAY:", 250, 200, 30, RAYWHITE);
                DrawText("- Use arrows to move", 250, 250, 20, RAYWHITE);
                DrawText("- Defend your tower!", 250, 280, 20, RAYWHITE);
                DrawText("Press ESC to return", 250, 330, 20, YELLOW);
                EndDrawing();

                if (IsKeyPressed(KEY_ESCAPE)) break;
            }
        }

        else if (option == MENU_RANKING) {
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(DARKPURPLE);
                DrawText("RANKING", 320, 200, 30, RAYWHITE);
                DrawText("1. Player A - 5000", 300, 250, 20, RAYWHITE);
                DrawText("2. Player B - 3500", 300, 280, 20, RAYWHITE);
                DrawText("Press ESC to return", 300, 330, 20, YELLOW);
                EndDrawing();

                if (IsKeyPressed(KEY_ESCAPE)) break;
            }
        }
    }

    CloseWindow();
    return 0;
}
