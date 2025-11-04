#ifndef MENU_H
#define MENU_H

typedef enum {
    MENU_START,
    MENU_HOW_TO_PLAY,
    MENU_RANKING,
    MENU_EXIT,
    MENU_TOTAL
} MenuOption;

MenuOption ShowMenu();

#endif
