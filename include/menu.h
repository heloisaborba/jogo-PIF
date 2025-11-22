#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef enum {
    MENU_START,
    MENU_HEROES,
    MENU_HOW_TO_PLAY,
    MENU_RANKING,  // Mantido
    MENU_EXIT,
    MENU_TOTAL
} MenuOption;

typedef struct {
    Texture2D img;
    const char *nome;
    const char *descricao;
} PersonagemInfo;

MenuOption ShowMenu();
void ShowHeroesMenu(PersonagemInfo personagens[]);
void ShowHowToPlayMenu();
void ShowRankingMenu();  // Adicionado

#endif