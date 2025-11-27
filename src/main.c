// main.c
#include "raylib.h"
#include "hero.h"
#include "enemy.h"
#include "tower.h"
#include "menu.h"
#include "game.h" // Contém GameState e current_game_state
#include "ranking.h"
#include <stdio.h> // Para printf
#include <string.h>

// Armazena o nome do jogador para ranking
char playerName[64] = "Anon";

// Prompt simples para inserir nome do jogador antes do jogo começar
void PromptPlayerName(void) {
    // buffer local para edição
    char buffer[64];
    buffer[0] = '\0';
    int len = 0;

    bool done = false;

    // Use GetCharPressed() to capture typed characters (supports letters, numbers, space)
    while (!WindowShouldClose() && !done) {
        int c = GetCharPressed();
        // handle typed characters
        while (c > 0) {
            // filter non-printable
            if (c >= 32 && c < 128 && len < (int)sizeof(buffer)-1) {
                buffer[len++] = (char)c;
                buffer[len] = '\0';
            }
            c = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (len > 0) { len--; buffer[len] = '\0'; }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            done = true;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            // keep default Anon and exit
            buffer[0] = '\0';
            done = true;
        }

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        const char *title = "DIGITE SEU NOME";
        int tw = MeasureText(title, 28);
        DrawText(title, GetScreenWidth()/2 - tw/2, 120, 28, GOLD);

        // input box
        int boxW = 500; int boxH = 60;
        int boxX = GetScreenWidth()/2 - boxW/2;
        int boxY = GetScreenHeight()/2 - boxH/2;
        DrawRectangle(boxX, boxY, boxW, boxH, LIGHTGRAY);
        DrawRectangleLines(boxX, boxY, boxW, boxH, DARKGRAY);

        // Draw current buffer (allow space characters)
        DrawText(buffer, boxX + 10, boxY + 12, 28, BLACK);

        // hint
        DrawText("Enter para confirmar. Esc para anon.", GetScreenWidth()/2 - 200, boxY + boxH + 10, 14, GRAY);

        EndDrawing();
    }

    // copy buffer if not empty
    if (buffer[0] != '\0') {
        strncpy(playerName, buffer, sizeof(playerName)-1);
        playerName[sizeof(playerName)-1] = '\0';
    }
}

// Tamanho da tela
const int screenWidth = 800;
const int screenHeight = 600;

// ==================== MAIN ====================

int main(void) {
    InitWindow(screenWidth, screenHeight, "Tower Defense Medieval");
    SetTargetFPS(60);

    // Inicializa ranking a partir do arquivo
    Ranking_Init("ranking.txt");

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
            // Pergunta o nome do jogador antes de iniciar
            PromptPlayerName();
            printf("DEBUG: Nome do jogador: %s\n", playerName);
            
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

        else if (option == MENU_RANKING) {
            printf("DEBUG: Opção RANKING selecionada\n");
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(DARKPURPLE);

                DrawText("RANKING", GetScreenWidth()/2 - MeasureText("RANKING", 30)/2, 120, 30, RAYWHITE);

                // Desenha top 5 do ranking carregado (centralizado)
                int topN = 5;
                int startY = 170;
                // Compute an X position that centers the widest expected text area
                // We use a fixed width offset to center the block rather than measuring each line.
                int blockWidth = 400;
                int x = GetScreenWidth()/2 - blockWidth/2;
                Ranking_Draw(x, startY, topN);

                int escY = startY + topN * 30 + 10;
                DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 20)/2, escY, 18, YELLOW);

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
    // Liberar memória do ranking antes de sair
    Ranking_Free();
    CloseWindow();
    return 0;
}