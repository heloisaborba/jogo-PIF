
#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_ENEMIES 20

#define NUM_WAYPOINTS 84 
#define ENEMY_DAMAGE_TO_CASTLE 20 

#define ENEMY_ATTACK_RANGE 75.0f 
#define ENEMY_DAMAGE_TO_HERO 5 
#define ENEMY_ATTACK_INTERVAL 1.5f 

static Texture2D background;         
static Texture2D backgroundFase2;    
static Texture2D towerTexture;       


Enemy enemies[MAX_ENEMIES];
int enemyCount;
float enemyLastAttackTime[MAX_ENEMIES];
int enemyTargetHero[MAX_ENEMIES];

recursos gameRecursos;

float spawnTimer;
static const float SPAWN_INTERVAL = 2.0f;

GameState current_game_state;
int enemies_defeated_count;
int towerHealth;

bool menuAberto;
Heroi herois[MAX_HEROIS];

PlacedHero placedHeroes[MAX_HEROIS];
int placedHeroCount;
bool placementMode;
int selectedHeroType;

void DrawPause(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));

    int boxW = 400;
    int boxH = 300;
    int boxX = screenWidth / 2 - boxW / 2;
    int boxY = screenHeight / 2 - boxH / 2;

    DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.15f, 10, DARKGRAY);
    DrawRectangleRoundedLines((Rectangle){boxX, boxY, boxW, boxH}, 0.15f, 10, WHITE);

    DrawText("PAUSADO", boxX + 100, boxY + 20, 40, WHITE);

    Rectangle btnContinuar = { boxX + 100, boxY + 90, 200, 45 };
    Rectangle btnReiniciar = { boxX + 100, boxY + 150, 200, 45 };
    Rectangle btnMenu = { boxX + 100, boxY + 210, 200, 45 };

    Vector2 mouse = GetMousePosition();

    DrawRectangleRec(btnContinuar, CheckCollisionPointRec(mouse, btnContinuar) ? GRAY : DARKGRAY);
    DrawText("Continuar", btnContinuar.x + 40, btnContinuar.y + 10, 25, WHITE);

    DrawRectangleRec(btnReiniciar, CheckCollisionPointRec(mouse, btnReiniciar) ? GRAY : DARKGRAY);
    DrawText("Reiniciar Fase", btnReiniciar.x + 25, btnReiniciar.y + 10, 25, WHITE);

    DrawRectangleRec(btnMenu, CheckCollisionPointRec(mouse, btnMenu) ? GRAY : DARKGRAY);
    DrawText("Menu Principal", btnMenu.x + 20, btnMenu.y + 10, 25, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

        if (CheckCollisionPointRec(mouse, btnContinuar)) {
            current_game_state = PLAYING;
            return;
        }

        if (CheckCollisionPointRec(mouse, btnReiniciar)) {
            ReiniciarFase();
            return;
        }

        if (CheckCollisionPointRec(mouse, btnMenu)) {
            VoltarMenuPrincipal();
            return;
        }
    }
}

void ReiniciarFase(void) {

    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    towerHealth = CASTLE_MAX_HEALTH;

    gameRecursos.moedas = 100;  

    menuAberto = false;

    current_game_state = PLAYING;
}

void VoltarMenuPrincipal(void) {
    current_game_state = MENU;

    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    towerHealth = CASTLE_MAX_HEALTH;

    gameRecursos.moedas = 100;

    menuAberto = false;
}



Vector2 path[NUM_WAYPOINTS] = {
    { 50, 565 }, { 65, 560 }, { 80, 555 }, { 95, 550 }, { 110, 545 },
    { 125, 540 }, { 140, 535 }, { 155, 530 }, { 170, 525 }, { 185, 520 }, 
    { 200, 515 }, { 215, 510 }, { 230, 505 }, { 245, 500 }, { 270, 495 }, 
    { 300, 488 }, { 330, 480 }, { 360, 473 }, { 390, 465 }, { 420, 458 }, 
    { 430, 435 }, { 450, 435 }, { 470, 435 }, { 490, 435 }, { 490, 420 }, 
    { 490, 405 }, { 490, 390 }, { 480, 385 }, { 465, 380 }, { 450, 375 }, 
    { 435, 370 }, { 420, 365 }, { 405, 360 }, { 390, 360 }, { 375, 366 }, 
    { 360, 372 }, { 345, 378 }, { 330, 384 }, { 315, 390 }, { 300, 396 }, 
    { 285, 402 }, { 270, 408 }, { 250, 420 }, { 230, 435 }, 
    { 215, 427 }, { 200, 419 }, { 185, 411 }, { 170, 403 }, 
    { 155, 395 }, { 140, 387 },
    { 140, 372 },
    { 155, 364 }, { 170, 356 }, { 185, 348 }, { 200, 340 }, 
    { 215, 332 }, { 230, 324 }, { 245, 316 },
    { 260, 316 }, { 245, 308 }, { 230, 300 }, { 215, 292 }, { 200, 284 }, 
    { 185, 276 },
    { 205, 271 }, { 225, 266 }, { 245, 261 }, { 265, 256 }, 
    { 280, 249 }, { 295, 242 }, { 310, 235 },
    { 325, 228 }, { 340, 221 }, { 360, 231 }, { 380, 241 }, { 400, 251 }, 
    { 420, 261 }, { 440, 271 },
    { 460, 251 },
    { 497, 239 },
    { 534, 227 },
    { 571, 215 },
    { 608, 203 },
    { 645, 176 }
};

void InicializarHerois(void) {
  strcpy(herois[0].nome, "Guerreiro");
  herois[0].custo = 50;
  herois[0].dano = 10;
  herois[0].alcance = 150;
    herois[0].texture = LoadTexture("resources/Cavaleiro.png");
  
  strcpy(herois[1].nome, "Bardo");
  herois[1].custo = 100;
  herois[1].dano = 20;
  herois[1].alcance = 300;
    herois[1].texture = LoadTexture("resources/Bardo.png");
  
  strcpy(herois[2].nome, "Paladino");
  herois[2].custo = 200;
  herois[2].dano = 15;
  herois[2].alcance = 200;
    herois[2].texture = LoadTexture("resources/Paladino.png");
  
  strcpy(herois[3].nome, "Mago");
  herois[3].custo = 150;
  herois[3].dano = 25;
  herois[3].alcance = 250;
    herois[3].texture = LoadTexture("resources/SapoMago.png");
}

int ComprarHeroiEspecifico(recursos *r, int tipoHeroi) {
  if (tipoHeroi >= 0 && tipoHeroi < MAX_HEROIS) {
    if (r->moedas >= herois[tipoHeroi].custo) {
      r->moedas -= herois[tipoHeroi].custo;
      
      if (placedHeroCount < MAX_HEROIS) {
        placementMode = true;
        selectedHeroType = tipoHeroi;
        menuAberto = false;
        TraceLog(LOG_INFO, "%s comprado! Clique no mapa para colocar. Moedas restantes: %d", herois[tipoHeroi].nome, r->moedas);
      } else {
        TraceLog(LOG_WARNING, "Limite de heróis atingido! Não foi possível comprar %s.", herois[tipoHeroi].nome);
      }
      return 1; 
    }
  }
  return 0; 
}

void DrawMenuHerois(void) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  
  DrawRectangle(40, 90, screenWidth - 80, screenHeight - 180, (Color){0, 0, 0, 220});
  
  DrawRectangleLines(40, 90, screenWidth - 80, screenHeight - 180, GOLD);
  
  DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 110, 30, GOLD);
  DrawText("Pressione M para fechar", screenWidth/2 - MeasureText("Pressione M para fechar", 20)/2, 150, 20, LIGHTGRAY);
  
  int cardWidth = 185; 
  int cardHeight = 245; 
  int spacing = 20;   
  int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
  int startY = 190;
  
  for (int i = 0; i < MAX_HEROIS; i++) {
    int cardX = startX + i * (cardWidth + spacing);
    int cardY = startY;
    
    Color cardColor = (Color){50, 50, 80, 255};
    DrawRectangle(cardX, cardY, cardWidth, cardHeight, cardColor);
    DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);
    
    DrawText(herois[i].nome, cardX + cardWidth/2 - MeasureText(herois[i].nome, 20)/2, cardY + 20, 20, YELLOW);
    
    int textureSize = 95; 
    int textureX = cardX + (cardWidth - textureSize) / 2;
    int textureY = cardY + 50;
    DrawTexturePro(herois[i].texture,
           (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
           (Rectangle){textureX, textureY, textureSize, textureSize},
           (Vector2){0, 0}, 0.0f, WHITE);
    
    DrawText(TextFormat("Custo: %d$", herois[i].custo), cardX + 20, cardY + 160, 17, GOLD);
    DrawText(TextFormat("Dano: %d", herois[i].dano), cardX + 20, cardY + 180, 17, RED);
    DrawText(TextFormat("Alcance: %d", herois[i].alcance), cardX + 20, cardY + 200, 17, BLUE);
    
    Color btnColor = (gameRecursos.moedas >= herois[i].custo) ? GREEN : RED;
    DrawRectangle(cardX + 20, cardY + cardHeight - 40, cardWidth - 40, 30, btnColor);
    DrawText("COMPRAR", cardX + cardWidth/2 - MeasureText("COMPRAR", 17)/2, cardY + cardHeight - 35, 17, WHITE);
    
    DrawText(TextFormat("[%d]", i + 1), cardX + cardWidth - 25, cardY + cardHeight - 35, 17, YELLOW);
  }
  
  DrawText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 
       screenWidth/2 - MeasureText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 17)/2, 
       startY + cardHeight + 25, 17, LIGHTGRAY);
}

void VerificarCliqueMenu(void) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mousePos = GetMousePosition();
    int screenWidth = GetScreenWidth();
    
    int cardWidth = 185;
    int cardHeight = 245;
    int spacing = 20;
    int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
    int startY = 190;
    
    for (int i = 0; i < MAX_HEROIS; i++) {
      int cardX = startX + i * (cardWidth + spacing);
      int cardY = startY;
      
      Rectangle btnRect = {cardX + 20, cardY + cardHeight - 40, cardWidth - 40, 30};
      if (CheckCollisionPointRec(mousePos, btnRect)) {
        if (ComprarHeroiEspecifico(&gameRecursos, i)) {
          TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[i].nome, gameRecursos.moedas);
        } else {
          TraceLog(LOG_WARNING, "Moedas insuficientes para comprar %s!", herois[i].nome);
        }
        break;
      }
    }
  }
}

void IniciarFase2(void) {
    TraceLog(LOG_INFO, "Iniciando Fase 2...");
    
    background = backgroundFase2; 
    
    enemyCount = 0;
    enemies_defeated_count = 0; 
    towerHealth = CASTLE_MAX_HEALTH; 
    current_game_state = PLAYING; 

    placedHeroCount = 0;

    spawnTimer = 0.0f; 
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemyLastAttackTime[i] = 0.0f;
        enemyTargetHero[i] = -1;
    }
    
}

void InitGame(void) {

    background = LoadTexture("resources/background_novo.jpg");
    towerTexture = LoadTexture("resources/tower.png");
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");

    inicializar_recursos(&gameRecursos);
    InicializarHerois();

    current_game_state = PLAYING;
    towerHealth = CASTLE_MAX_HEALTH;
    enemies_defeated_count = 0;

    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    menuAberto = false;

}


void UpdateGame(void) {

    if (IsKeyPressed(KEY_P)) {
        if (current_game_state == PLAYING) {
            current_game_state = PAUSED;
        }
        else if (current_game_state == PAUSED) {
            current_game_state = PLAYING;
        }
    }

    if (current_game_state == PAUSED) {

        if (IsKeyPressed(KEY_R)) {
            ResetGame();
            current_game_state = PLAYING;
            return;
        }

        if (IsKeyPressed(KEY_M)) {
            current_game_state = MENU;
            return;
        }

        return; 
    }

    if (current_game_state == WAVE_WON) {

        Rectangle botaoContinuar = {
            (GetScreenWidth()/2) - 100,
            GetScreenHeight()/2 + 50,
            200,
            50
        };

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(GetMousePosition(), botaoContinuar)) {
                IniciarFase2();
                return;
            }
        }

        return;
    }


    if (current_game_state == GAME_OVER) {

        Rectangle botaoMenu = {
            (GetScreenWidth()/2) - 100,
            GetScreenHeight()/2 + 50,
            200,
            50
        };

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(GetMousePosition(), botaoMenu)) {
                current_game_state = MENU;
                return;
            }
        }

        return;
    }


    if (menuAberto) {
        VerificarCliqueMenu();
        return;
    }


    if (placementMode) {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            Vector2 mousePos = GetMousePosition();

            placedHeroes[placedHeroCount].x = mousePos.x;
            placedHeroes[placedHeroCount].y = mousePos.y;
            placedHeroes[placedHeroCount].tipo = selectedHeroType;
            placedHeroes[placedHeroCount].dano = herois[selectedHeroType].dano;
            placedHeroes[placedHeroCount].alcance = herois[selectedHeroType].alcance;
            placedHeroes[placedHeroCount].health = 100;
            placedHeroes[placedHeroCount].lastAttackTime = 0;
            placedHeroes[placedHeroCount].texture = herois[selectedHeroType].texture;

            placedHeroCount++;

            placementMode = false;
            selectedHeroType = -1;
        }

        return;
    }


    if (current_game_state != PLAYING) return;


    for (int i = 0; i < placedHeroCount; i++) {
        if (placedHeroes[i].health <= 0) continue;

        float minDist = placedHeroes[i].alcance;
        int targetEnemy = -1;

        for (int j = 0; j < enemyCount; j++) {
            if (!enemies[j].active) continue;

            float dx = placedHeroes[i].x - enemies[j].x;
            float dy = placedHeroes[i].y - enemies[j].y;
            float dist = sqrt(dx*dx + dy*dy);

            if (dist < minDist) {
                minDist = dist;
                targetEnemy = j;
            }
        }

        if (targetEnemy != -1) {
            placedHeroes[i].lastAttackTime += GetFrameTime();
            if (placedHeroes[i].lastAttackTime >= 1.0f) {

                enemies[targetEnemy].health -= placedHeroes[i].dano;

                if (enemies[targetEnemy].health <= 0) {
                    adicionar_moedas(&gameRecursos, enemies[targetEnemy].recompensa_moedas);
                    enemies[targetEnemy].active = 0;
                    enemies_defeated_count++;
                }

                placedHeroes[i].lastAttackTime = 0;
            }
        }
    }


    if (towerHealth > 0 && enemyCount < MAX_ENEMIES) {
        spawnTimer += GetFrameTime();
        if (spawnTimer >= SPAWN_INTERVAL) {
            enemies[enemyCount] = InitEnemy(path[0].x, path[0].y);
            enemyLastAttackTime[enemyCount] = 0;
            enemyTargetHero[enemyCount] = -1;
            enemyCount++;
            spawnTimer = 0;
        }
    }


    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;

        int targetHero = enemyTargetHero[i];

        if (targetHero == -1 || placedHeroes[targetHero].health <= 0) {

            targetHero = -1;
            float minDist = ENEMY_ATTACK_RANGE;

            for (int h = 0; h < placedHeroCount; h++) {
                if (placedHeroes[h].health <= 0) continue;

                float dx = enemies[i].x - placedHeroes[h].x;
                float dy = enemies[i].y - placedHeroes[h].y;
                float dist = sqrt(dx*dx + dy*dy);

                if (dist <= minDist) {
                    minDist = dist;
                    targetHero = h;
                    break;
                }
            }

            enemyTargetHero[i] = targetHero;
        }


        if (targetHero != -1) {  

            enemyLastAttackTime[i] += GetFrameTime();

            if (enemyLastAttackTime[i] >= ENEMY_ATTACK_INTERVAL) {
                placedHeroes[targetHero].health -= ENEMY_DAMAGE_TO_HERO;

                if (placedHeroes[targetHero].health <= 0)
                    placedHeroes[targetHero].health = 0;

                enemyLastAttackTime[i] = 0;
            }

        } else {  

            UpdateEnemy(&enemies[i]);
            enemyTargetHero[i] = -1;
        }


        if (EnemyReachedTower(enemies[i]) && enemies[i].active) {

            towerHealth -= ENEMY_DAMAGE_TO_CASTLE;
            enemies[i].active = 0;

            if (towerHealth <= 0) {
                towerHealth = 0;
                current_game_state = GAME_OVER;
            }
        }
    }


    if (enemyCount >= MAX_ENEMIES &&
        enemies_defeated_count >= MAX_ENEMIES) {

        current_game_state = WAVE_WON;
    }
}

void DrawGameUI(void) {
  DrawRectangle(10, 10, 280, 90, (Color){0, 0, 0, 128});
  
  DrawText(TextFormat("Torre HP: %d", towerHealth), 20, 20, 20, RED);
  
  DrawText(TextFormat("Moedas: %d", get_moedas(&gameRecursos)), 20, 50, 20, GOLD);
  
  DrawText("M - Abrir loja de herois", 20, 80, 15, LIGHTGRAY);

  if (placementMode) {
    DrawText("Clique no mapa para colocar o herói", GetScreenWidth()/2 - MeasureText("Clique no mapa para colocar o herói", 20)/2, 20, 20, YELLOW);
  }
}

void DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(
        background,
        (Rectangle){ 0, 0, background.width, background.height }, 
        (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, 
        (Vector2){ 0, 0 }, 
        0.0f, 
        WHITE 
    );

    if (current_game_state == PAUSE) {
        DrawPause();
        EndDrawing();
        return;   
    }

    for (int i = 0; i < MAX_WAYPOINTS - 1; i++) {
        DrawLineEx(path[i], path[i + 1], 3.0f, (Color){255, 255, 0, 128});
    }
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        DrawCircle(path[i].x, path[i].y, 5.0f, BLUE);
        DrawText(TextFormat("%d", i), path[i].x + 10, path[i].y - 10, 10, WHITE);
    }

    DrawTexture(towerTexture, 650, 100, WHITE);
    
    DrawGameUI();

    if (current_game_state == PLAYING) {
        // Inimigos
        for (int i = 0; i < enemyCount; i++) {
            DrawEnemy(enemies[i]);
        }

        for (int i = 0; i < placedHeroCount; i++) {
            if (placedHeroes[i].health <= 0) continue;

            Color heroColor;
            switch (placedHeroes[i].tipo) {
                case 0: heroColor = BLUE; break;
                case 1: heroColor = GREEN; break;
                case 2: heroColor = YELLOW; break;
                case 3: heroColor = PURPLE; break;
                default: heroColor = WHITE; break;
            }

            DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 20, heroColor);
            DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});

            int barWidth = 40;
            int barHeight = 5;
            int barX = placedHeroes[i].x - barWidth / 2;
            int barY = placedHeroes[i].y - 30;

            DrawRectangle(barX, barY, barWidth, barHeight, RED);
            DrawRectangle(barX, barY, (int)(barWidth * (placedHeroes[i].health / 100.0f)), barHeight, GREEN);
        }
    }

    if (menuAberto) {
        DrawMenuHerois();
    }
    
    if (current_game_state == GAME_OVER) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RED, 0.8f)); 
        const char *message = "VOCÊ PERDEU! A TORRE FOI DESTRUÍDA.";
        int message_len = MeasureText(message, 40);
        DrawText(message, (GetScreenWidth() / 2) - (message_len / 2),
                 GetScreenHeight() / 2 - 50, 40, WHITE);
    }

    else if (current_game_state == WAVE_WON) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGREEN, 0.8f));
        
        const char *message = "VOCÊ GANHOU! IR PARA A FASE 2";
        int message_len = MeasureText(message, 40);
        DrawText(message, (GetScreenWidth() / 2) - (message_len / 2),
                 GetScreenHeight() / 2 - 50, 40, GOLD);
        
        Rectangle button_rect = { (GetScreenWidth() / 2) - 100,
                                  GetScreenHeight() / 2 + 50,
                                  200, 50 };
        DrawRectangleRec(button_rect, YELLOW);
        DrawText("CONTINUAR", button_rect.x + 50, button_rect.y + 15, 20, BLACK);
    }

    EndDrawing();
}

void CloseGame(void) {
    UnloadTexture(background);
    UnloadTexture(backgroundFase2);
    
  UnloadTexture(towerTexture);
  
  for (int i = 0; i < MAX_HEROIS; i++) {
    UnloadTexture(herois[i].texture);
  }
}