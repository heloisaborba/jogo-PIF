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
    // int fontSize = 30; // Removido, agora será calculado

    printf("DEBUG: Menu iniciado - aguardando input\n");

    Texture2D background = LoadTexture("resources/fundoMenu.png");

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP)) {
            selected--;
            if (selected < 0) selected = MENU_TOTAL - 1;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            selected++;
            if (selected >= MENU_TOTAL) selected = 0;
        }

        BeginDrawing();

        ClearBackground(BLACK); 

        // === CÁLCULO DE ESCALA OTIMIZADO (Contain) ===
        float screenWidth = GetScreenWidth();
        float screenHeight = GetScreenHeight();
        float imageRatio = (float)background.width / (float)background.height;
        float screenRatio = screenWidth / screenHeight;

        float scale = (screenRatio < imageRatio) ? 
                      (screenWidth / (float)background.width) : 
                      (screenHeight / (float)background.height);

        float scaledWidth = background.width * scale;
        float scaledHeight = background.height * scale;

        float centeredX = (screenWidth - scaledWidth) / 2.0f;
        float centeredY = (screenHeight - scaledHeight) / 2.0f;

        Rectangle sourceRec = {0, 0, (float)background.width, (float)background.height};
        Rectangle destRec = {centeredX, centeredY, scaledWidth, scaledHeight};
        
        DrawTexturePro(background, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);

        // === DESENHO DO TEXTO RESPONSIVO ===
        
        // --- Título do Jogo ---
        const char* titleText = "=== TOWER DEFENSE ===";
        // Tamanho da fonte do título: Ajuste a base (por exemplo, 40) e o fator de escala
        int titleFontSize = (int)(40 * (screenHeight / 720.0f)); // Baseado em uma altura de 720p
        Vector2 titleSize = MeasureTextEx(GetFontDefault(), titleText, titleFontSize, 0);
        DrawText(titleText, (screenWidth - titleSize.x) / 2.4, screenHeight * 0.15f, titleFontSize, RAYWHITE); // Posição vertical percentual

        // --- Opções do Menu ---
        // Tamanho da fonte das opções
        int menuFontSize = (int)(30 * (screenHeight / 720.0f)); // Baseado em uma altura de 720p
        float startY = screenHeight * 0.40f; // Posição vertical inicial percentual para as opções
        float lineSpacing = screenHeight * 0.07f; // Espaçamento entre as linhas percentual

        for (int i = 0; i < MENU_TOTAL; i++) {
            Color color = (i == selected) ? GREEN : RAYWHITE;
            const char *prefix = (i == selected) ? "> " : "  ";
            const char* optionText = TextFormat("%s%s", prefix, options[i]);
            Vector2 optionSize = MeasureTextEx(GetFontDefault(), optionText, menuFontSize, 0);
            DrawText(optionText, (screenWidth - optionSize.x) / 2.2, startY + i * lineSpacing, menuFontSize, color);
        }

        // --- Instruções ---
        const char* instructionsText = "Use ARROWS + ENTER";
        int instructionsFontSize = (int)(20 * (screenHeight / 720.0f)); // Baseado em uma altura de 720p
        Vector2 instructionsSize = MeasureTextEx(GetFontDefault(), instructionsText, instructionsFontSize, 0);
        DrawText(instructionsText, (screenWidth - instructionsSize.x) / 2, screenHeight * 0.8f, instructionsFontSize, GRAY); // Posição vertical percentual

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)) {
            UnloadTexture(background);
            return (MenuOption)selected;
        }
    }

    UnloadTexture(background);
    return MENU_EXIT;
}