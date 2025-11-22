// main.c
#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"
#include "menu.h"
#include "game.h" // Contém GameState e current_game_state
#include <stdio.h> // Para printf

// Tamanho da tela
const int screenWidth = 800;
const int screenHeight = 600;

// ==================== MAIN ====================

int main(void) {
    InitWindow(screenWidth, screenHeight, "Tower Defense Medieval");
    SetTargetFPS(60);

    printf("DEBUG: Janela inicializada, entrando no loop principal\n");

    while (!WindowShouldClose()) {
        printf("DEBUG: Chamando ShowMenu()\n");
        MenuOption option = ShowMenu();
        printf("DEBUG: ShowMenu() retornou: %d\n", option);

        if (option == MENU_EXIT) {
            printf("DEBUG: Opção EXIT selecionada, saindo...\n");
            break;
        }

        else if (option == MENU_START) {
            printf("DEBUG: Opção START selecionada, iniciando jogo...\n");
            
            // --- Bloco de Carregamento (Mantido) ---
            printf("DEBUG: Mostrando tela de teste...\n");
            int frames = 0;
            
            while (!WindowShouldClose() && frames < 30) { 
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
            // ----------------------------------------
            
            // Se o teste passou, iniciar o loop de jogo real
            if (frames >= 30) {
                printf("DEBUG: Iniciando jogo completo...\n");
                InitGame(); // Inicializa o jogo e define current_game_state = PLAYING

                // O NOVO LOOP DE JOGO:
                // Ele continua enquanto o estado não for forçado para MENU
                while (!WindowShouldClose() && 
                       (current_game_state == PLAYING || 
                        current_game_state == PAUSED || 
                        current_game_state == WAVE_WON || 
                        current_game_state == GAME_OVER)) 
                {
                    // Lógica do jogo (Atualiza e Desenha)
                    UpdateGame();
                    DrawGame();
                }

                CloseGame(); // Limpa recursos do jogo
                printf("DEBUG: Jogo encerrado, voltando ao ShowMenu() \n");
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
        
        // Se o loop de jogo terminou (porque current_game_state != PLAYING/PAUSED/etc), 
        // o programa continua e repete o loop principal chamando ShowMenu()
    }

    printf("DEBUG: Fechando janela...\n");
    CloseWindow();
    return 0;
}