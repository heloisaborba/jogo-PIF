#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"
#include "menu.h"
#include "game.h"  // ADICIONE ESTA LINHA
#include <stdio.h>  // Para printf

// Tamanho da tela
const int screenWidth = 800;
const int screenHeight = 600;

// ==================== MAIN ====================

int main(void) {
    InitWindow(screenWidth, screenHeight, "Tower Defense Medieval");
    SetTargetFPS(60);

    printf("DEBUG: Janela inicializada, entrando no loop principal\n");

    while (true) {
        printf("DEBUG: Chamando ShowMenu()\n");
        MenuOption option = ShowMenu();
        printf("DEBUG: ShowMenu() retornou: %d\n", option);

        if (option == MENU_EXIT) {
            printf("DEBUG: Opção EXIT selecionada, saindo...\n");
            break;
        }

        else if (option == MENU_START) {
            printf("DEBUG: Opção START selecionada, iniciando jogo...\n");
            
            // TESTE SIMPLES: Mostrar tela de carregamento primeiro
            printf("DEBUG: Mostrando tela de teste...\n");
            int frames = 0;
            
            while (!WindowShouldClose() && frames < 30) { // Executa por ~0.5 segundo
                frames++;
                
                BeginDrawing();
                ClearBackground(GREEN);
                DrawText("CARREGANDO JOGO...", 200, 250, 30, BLACK);
                DrawText(TextFormat("Frame: %d/30", frames), 200, 300, 20, DARKGRAY);
                EndDrawing();
                
                if (IsKeyPressed(KEY_ESCAPE)) {
                    printf("DEBUG: ESC pressionado durante carregamento\n");
                    break;
                }
            }
            
            // Se o teste passou, iniciar o jogo real
            if (frames >= 30) {
                printf("DEBUG: Iniciando jogo completo...\n");
                InitGame();

                while (!WindowShouldClose()) {
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        printf("DEBUG: ESC pressionado, voltando ao menu\n");
                        break;
                    }
                    UpdateGame();
                    DrawGame();
                }

                CloseGame();
                printf("DEBUG: Jogo fechado, voltando ao menu\n");
            } else {
                printf("DEBUG: Carregamento interrompido, voltando ao menu\n");
            }
        }

        else if (option == MENU_HOW_TO_PLAY) {
            printf("DEBUG: Opção HOW_TO_PLAY selecionada\n");
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(DARKBLUE);
                DrawText("HOW TO PLAY:", 250, 200, 30, RAYWHITE);
                DrawText("- Use arrows to move", 250, 250, 20, RAYWHITE);
                DrawText("- Defend your tower!", 250, 280, 20, RAYWHITE);
                DrawText("Press ESC to return", 250, 330, 20, YELLOW);
                EndDrawing();

                if (IsKeyPressed(KEY_ESCAPE)) {
                    printf("DEBUG: ESC pressionado em HOW_TO_PLAY\n");
                    break;
                }
            }
        }

        else if (option == MENU_RANKING) {
            printf("DEBUG: Opção RANKING selecionada\n");
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(DARKPURPLE);
                DrawText("RANKING", 320, 200, 30, RAYWHITE);
                DrawText("1. Player A - 5000", 300, 250, 20, RAYWHITE);
                DrawText("2. Player B - 3500", 300, 280, 20, RAYWHITE);
                DrawText("Press ESC to return", 300, 330, 20, YELLOW);
                EndDrawing();

                if (IsKeyPressed(KEY_ESCAPE)) {
                    printf("DEBUG: ESC pressionado em RANKING\n");
                    break;
                }
            }
        }
        
        else {
            printf("DEBUG: Opção desconhecida: %d\n", option);
        }
    }

    printf("DEBUG: Fechando janela...\n");
    CloseWindow();
    return 0;
}