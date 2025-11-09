#include "raylib.h"
#include "menu.h"
#include <stdio.h>

MenuOption ShowMenu() {
    const char *options[MENU_TOTAL] = {
        "Start Game",
        "How to Play", 
        "Ranking",
        "Exit"
    };

    int selected = 0;
    int fontSize = 30;
    int frameCount = 0;

    printf("DEBUG: Menu iniciado - aguardando input\n");

    while (!WindowShouldClose()) {
        frameCount++;
        
        // DEBUG: Monitorar estado a cada 60 frames
        if (frameCount % 60 == 0) {
            printf("DEBUG: Menu frame %d - selected: %d\n", frameCount, selected);
        }

        // Controle por teclado - COM VERIFICAÇÃO DE SEGURANÇA
        if (IsKeyPressed(KEY_UP)) {
            selected--;
            if (selected < 0) selected = MENU_TOTAL - 1;
            printf("DEBUG: KEY_UP - nova seleção: %d\n", selected);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            selected++;
            if (selected >= MENU_TOTAL) selected = 0;
            printf("DEBUG: KEY_DOWN - nova seleção: %d\n", selected);
        }

        // Desenho do menu - COM TRY/CATCH IMPLÍCITO
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("=== TOWER DEFENSE ===", 220, 100, 40, RAYWHITE);

        for (int i = 0; i < MENU_TOTAL; i++) {
            Color color = (i == selected) ? YELLOW : RAYWHITE;
            const char *prefix = (i == selected) ? "> " : "  ";
            DrawText(TextFormat("%s%s", prefix, options[i]), 300, 200 + i * 50, fontSize, color);
        }

        // Info de debug na tela
        DrawText("Use ARROWS + ENTER", 250, 500, 20, GRAY);
        DrawText(TextFormat("Frame: %d | Selected: %d", frameCount, selected), 10, 10, 15, GREEN);

        EndDrawing();

        // Verificação de ENTER - COM LOG DETALHADO
        if (IsKeyPressed(KEY_ENTER)) {
            printf("DEBUG: ==========================================\n");
            printf("DEBUG: ENTER DETECTADO! Opção selecionada: %d (%s)\n", 
                   selected, options[selected]);
            printf("DEBUG: Retornando para main()...\n");
            printf("DEBUG: ==========================================\n");
            return (MenuOption)selected;
        }

        // Pequena pausa para estabilidade
        WaitTime(0.016);
        
        // Safety check: se ficar preso por muitos frames
        if (frameCount > 600) { // ~10 segundos
            printf("DEBUG: SAFETY TIMEOUT - Menu preso por muito tempo\n");
            printf("DEBUG: Retornando MENU_EXIT por segurança\n");
            return MENU_EXIT;
        }
    }

    printf("DEBUG: WindowShouldClose() = true, saindo do menu\n");
    return MENU_EXIT;
}