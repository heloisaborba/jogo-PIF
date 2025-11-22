#include "raylib.h"
#include "menu.h"
#include <stdio.h>

void ShowHowMenu(PersonagemInfo personagens[]) {

    // Pequeno delay ao entrar: evita ENTER duplicado
    double t = GetTime();
    while (GetTime() - t < 0.15) { }

    const char *options[2] = {
        "Controles",
        "Personagens"
    };

    int selected = 0;

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_UP)) {
            selected--;
            if (selected < 0) selected = 1;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            selected++;
            if (selected > 1) selected = 0;
        }

        if (IsKeyPressed(KEY_BACKSPACE)) return;

        if (IsKeyPressed(KEY_ENTER)) {

            // ===================================
            // === TELA CONTROLES ================
            // ===================================
            if (selected == 0) {
                while (!WindowShouldClose()) {

                    BeginDrawing();
                    ClearBackground(BLACK);

                    DrawText("CONTROLES", 50, 40, 50, WHITE);
                    DrawText("Use as setas para mover.", 80, 200, 30, WHITE);
                    DrawText("ENTER para confirmar.", 80, 250, 30, WHITE);
                    DrawText("BACKSPACE para voltar.", 80, 300, 30, WHITE);

                    EndDrawing();

                    if (IsKeyPressed(KEY_BACKSPACE)) return;
                }
            }

            // ===================================
            // === TELA PERSONAGENS ==============
            // ===================================
            if (selected == 1) {
                while (!WindowShouldClose()) {

                    BeginDrawing();
                    ClearBackground(BLACK);

                    DrawText("PERSONAGENS", 50, 40, 50, WHITE);

                    int cardX = 60;
                    int cardYStart = 140;
                    int cardSpacing = 230;

                    for (int i = 0; i < 4; i++) {

                        int y = cardYStart + i * cardSpacing;

                        // Card de fundo
                        Rectangle card = { cardX, y, 650, 200 };
                        DrawRectangleRounded(card, 0.12f, 8, Fade(WHITE, 0.05f));
                        DrawRectangleRoundedLines(card, 0.12f, 8, Fade(WHITE, 0.12f));

                        // Escala automática
                        float targetHeight = 140.0f;
                        float scale = targetHeight / personagens[i].img.height;

                        // Centralizar verticalmente
                        int imgX = cardX + 20;
                        int imgY = y + (200/2) - (personagens[i].img.height * scale / 2);

                        DrawTextureEx(personagens[i].img, (Vector2){imgX, imgY}, 0, scale, WHITE);

                        // Nome e descrição
                        DrawText(personagens[i].nome, cardX + 200, y + 30, 34, WHITE);
                        DrawText(personagens[i].descricao, cardX + 200, y + 90, 20, LIGHTGRAY);
                    }

                    EndDrawing();

                    if (IsKeyPressed(KEY_BACKSPACE)) return;
                }
            }
        }

        // ===================================
        // === MENU DO HOW TO PLAY ===========
        // ===================================

        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("HOW TO PLAY", 50, 40, 50, WHITE);

        for (int i = 0; i < 2; i++) {
            Color color = (i == selected) ? YELLOW : WHITE;
            DrawText(options[i], 80, 150 + 60 * i, 35, color);
        }

        DrawText("ENTER para abrir | BACKSPACE para voltar",
                 80, 500, 20, GRAY);

        EndDrawing();
    }
}


// ============================================================================
// =============================== MENU PRINCIPAL =============================
// ============================================================================

MenuOption ShowMenu() {
    const char *options[MENU_TOTAL] = {
        "Start Game",
        "How to Play",
        "Ranking",
        "Exit"
    };

    int selected = 0;

    Texture2D background = LoadTexture("resources/fundoMenu.png");

    // ====================
    // PERSONAGENS PERFIL
    // ====================
    PersonagemInfo personagens[4];

    personagens[0].img = LoadTexture("resources/Cavaleiro.png");
    personagens[0].nome = "Cavaleiro";
    personagens[0].descricao = "Um combatente forte que protege a linha de frente.";

    personagens[1].img = LoadTexture("resources/SapoMago.png");
    personagens[1].nome = "Mago";
    personagens[1].descricao = "Ataca com magia poderosa e dano em área.";

    personagens[2].img = LoadTexture("resources/Bardo.png");
    personagens[2].nome = "Bardo";
    personagens[2].descricao = "Toca músicas para atacar e buffar aliados.";

    personagens[3].img = LoadTexture("resources/Paladino.png");
    personagens[3].nome = "Paladino";
    personagens[3].descricao = "Protege aliados com escudo e cura.";

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_UP)) {
            selected--;
            if (selected < 0) selected = MENU_TOTAL - 1;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            selected++;
            if (selected >= MENU_TOTAL) selected = 0;
        }

        if (IsKeyPressed(KEY_ENTER)) {

            if (selected == MENU_HOW_TO_PLAY) {
                ShowHowMenu(personagens);
                selected = -1;      // ← impede ENTER duplicado
                continue;
            } else {
                break;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        float sw = GetScreenWidth();
        float sh = GetScreenHeight();

        float ratioImg = (float)background.width / background.height;
        float ratioScreen = sw / sh;

        float scale = (ratioScreen < ratioImg ? sw / background.width : sh / background.height);
        float w = background.width * scale;
        float h = background.height * scale;

        Rectangle src = {0, 0, background.width, background.height};
        Rectangle dst = {(sw - w) / 2, (sh - h) / 2, w, h};

        DrawTexturePro(background, src, dst, (Vector2){0, 0}, 0, WHITE);

        DrawText("=== TOWER DEFENSE ===", sw * 0.22, sh * 0.15, 40 * (sh / 720.0f), WHITE);

        int menuFont = (int)(30 * (sh / 720.0f));
        float yStart = sh * 0.40;
        float spacing = sh * 0.07;

        for (int i = 0; i < MENU_TOTAL; i++) {
            Color color = (i == selected) ? GREEN : WHITE;
            const char *txt = TextFormat("%s%s", i == selected ? "> " : "  ", options[i]);

            Vector2 s = MeasureTextEx(GetFontDefault(), txt, menuFont, 0);
            DrawText(txt, (sw - s.x) / 2.2, yStart + i * spacing, menuFont, color);
        }

        DrawText("Use ARROWS + ENTER", sw * 0.35, sh * 0.8, 20, GRAY);

        EndDrawing();
    }

    for (int i = 0; i < 4; i++) UnloadTexture(personagens[i].img);
    UnloadTexture(background);

    return (MenuOption)selected;
}
