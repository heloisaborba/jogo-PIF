#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum {
    MENU_START,
    MENU_HOW_TO_PLAY,
    MENU_RANKING,
    MENU_EXIT,
    MENU_TOTAL
} MenuOption;

typedef struct {
    Texture2D img;
    const char *nome;
    const char *descricao;
} PersonagemInfo;

typedef enum {
    HOW_MENU,
    HOW_CONTROLES,
    HOW_PERSONAGENS
} HowState;

MenuOption ShowMenu();
void ShowHowMenu(PersonagemInfo personagens[]);

#endif
