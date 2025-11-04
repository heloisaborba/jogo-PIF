#include "raylib.h"
#include "menu.h"


MenuOption ShowMenu() {
    const char *options[MENU_TOTAL] = {
        "Start Game",
        "How to Play",
        "Ranking",
        "Exit"
    };

    int selected = 0;
    int fontSize = 30;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP)) selected--;
        if (IsKeyPressed(KEY_DOWN)) selected++;
        if (selected < 0) selected = MENU_TOTAL - 1;
        if (selected >= MENU_TOTAL) selected = 0;

        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("=== TOWER DEFENSE ===", 220, 100, 40, RAYWHITE);

        for (int i = 0; i < MENU_TOTAL; i++) {
            Color color = (i == selected) ? YELLOW : RAYWHITE;
            const char *prefix = (i == selected) ? "> " : "  ";

            DrawText(TextFormat("%s%s", prefix, options[i]), 300, 200 + i * 50, fontSize, color);
        }

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)) {
            return (MenuOption)selected;
        }
    }

    return MENU_EXIT;
}
