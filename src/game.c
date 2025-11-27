#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include "hero.h"
#include "tower.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "ranking.h"

const int WAVE_SIZE[] = {0, 15, 25, 99999};

Texture2D background;
Texture2D backgroundFase2;
Texture2D backgroundFase3;
Texture2D towerTexture;

Enemy enemies[MAX_ENEMIES];
int enemyCount;
float enemyLastAttackTime[MAX_ENEMIES];
int enemyTargetHero[MAX_ENEMIES];

recursos gameRecursos;

bool is_tower_burning;
float tower_burn_timer;

float tower_attack_timer = 0.0f;
const float TOWER_ATTACK_INTERVAL = 1.0f;
const float TOWER_ATTACK_RANGE = 150.0f;
const int TOWER_ATTACK_DAMAGE = 30;

float spawnTimer;
static const float SPAWN_INTERVAL = 2.0f;
static const float PHASE3_BASE_SPAWN = 1.5f;
static const float PHASE3_MIN_SPAWN = 0.3f;
static const float SPAWN_DECAY_RATE = 0.005f;
double phaseStartTime = 0.0;

GameState current_game_state;
int enemies_defeated_count;
int towerHealth;

bool menuAberto;
Heroi herois[HERO_TYPE_COUNT];

PlacedHero *placedHeroes = NULL;
int placedHeroCount;
int placedHeroCapacity = 0;
bool placementMode;
int selectedHeroType;

static int heatmapCellSize = 24;
static int heatmapCols = 0;
static int heatmapRows = 0;
static int **heatmap = NULL;
static bool showHeatmap = false;

static void InitHeatmap(void) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    heatmapCols = (w + heatmapCellSize - 1) / heatmapCellSize;
    heatmapRows = (h + heatmapCellSize - 1) / heatmapCellSize;
    heatmap = (int**)calloc((size_t)heatmapRows, sizeof(int*));
    if (!heatmap) return;
    for (int r = 0; r < heatmapRows; r++) {
        heatmap[r] = (int*)calloc((size_t)heatmapCols, sizeof(int));
        if (!heatmap[r]) {
            for (int k = 0; k < r; k++) free(heatmap[k]);
            free(heatmap);
            heatmap = NULL;
            heatmapCols = heatmapRows = 0;
            return;
        }
    }
}

static void FreeHeatmap(void) {
    if (!heatmap) return;
    for (int r = 0; r < heatmapRows; r++) {
        if (heatmap[r]) free(heatmap[r]);
    }
    free(heatmap);
    heatmap = NULL;
    heatmapCols = heatmapRows = 0;
}

static void ResetHeatmap(void) {
    if (!heatmap) return;
    for (int r = 0; r < heatmapRows; r++) {
        for (int c = 0; c < heatmapCols; c++) {
            heatmap[r][c] = 0;
        }
    }
}

static void HeatmapIncrementAt(float x, float y) {
    if (!heatmap) return;
    int col = (int)(x) / heatmapCellSize;
    int row = (int)(y) / heatmapCellSize;
    if (col < 0) col = 0;
    if (col >= heatmapCols) col = heatmapCols - 1;
    if (row < 0) row = 0;
    if (row >= heatmapRows) row = heatmapRows - 1;
    heatmap[row][col]++;
}

static void DrawHeatmap(void) {
    if (!heatmap || !showHeatmap) return;
    int maxv = 0;
    for (int r = 0; r < heatmapRows; r++) {
        for (int c = 0; c < heatmapCols; c++) {
            if (heatmap[r][c] > maxv) maxv = heatmap[r][c];
        }
    }
    if (maxv == 0) return;
    for (int r = 0; r < heatmapRows; r++) {
        for (int c = 0; c < heatmapCols; c++) {
            int v = heatmap[r][c];
            if (v <= 0) continue;
            float t = (float)v / (float)maxv;
            float tn = fminf(fmaxf(t, 0.0f), 1.0f);
            unsigned char alpha = (unsigned char)(tn * 180.0f);
            unsigned char red = (unsigned char)(200 + 55 * tn);
            unsigned char green = (unsigned char)(50 * (1.0f - tn));
            Rectangle rc = { (float)(c * heatmapCellSize), (float)(r * heatmapCellSize), (float)heatmapCellSize, (float)heatmapCellSize };
            DrawRectangleRec(rc, (Color){ red, green, 0, alpha });
        }
    }
}

HeroUpgrade heroUpgrades[HERO_TYPE_COUNT];
bool menuUpgradesAberto = false;
int selectedUpgradeHero = 0;

bool sellMode = false;
int selectedHeroToSell = -1;
float sellMessageTimer = 0.0f;
char sellMessage[100] = "";

double gameTimer = 0.0;
bool rankingSaved = false;
extern char playerName[64];

float noHeroTimer = 0.0f;
bool lastHeroDied = false;

Vector2 pathInferior[84];
Vector2 pathSuperior[96];
bool useMultiplePaths = false;
int activePathCount = 1;

int currentWave;
int total_enemies_spawned = 0;
const int MAX_WAVES = 3;
const int WAVE_REWARD = 150;

#define NUM_WAYPOINTS_BAIXO 84
#define NUM_WAYPOINTS_CIMA 96
#define MAX_PATHS 2

#define ENEMY_ATTACK_RANGE 75.0f
#define ENEMY_ATTACK_INTERVAL 1.5f

void ResetGame();

static void EnsurePlacedCapacity(int minNeeded) {
    if (placedHeroCapacity >= minNeeded) return;
    int newCap = placedHeroCapacity > 0 ? placedHeroCapacity * 2 : 8;
    while (newCap < minNeeded) newCap *= 2;
    PlacedHero *newArr = (PlacedHero*)realloc(placedHeroes, newCap * sizeof(PlacedHero));
    if (!newArr) {
        TraceLog(LOG_ERROR, "Falha ao alocar memória para placedHeroes (need=%d)", minNeeded);
        return;
    }
    for (int i = placedHeroCapacity; i < newCap; i++) {
        newArr[i].health = 0;
        newArr[i].is_burning = false;
        newArr[i].burn_timer = 0.0f;
        newArr[i].lastAttackTime = 0.0f;
    }
    placedHeroes = newArr;
    placedHeroCapacity = newCap;
}

int GetRandomEnemyType() {
    int r = GetRandomValue(1, 100);
    if (r <= 50) return INIMIGO_GOBLIN;
    else if (r <= 75) return INIMIGO_SPECTRO;
    else if (r <= 95) return INIMIGO_NECROMANTE;
    else return INIMIGO_DRAGAO;
}

void GetCurrentPaths(Vector2** paths, int* pathLengths, int* pathCount) {
    if (currentWave == 1) {
        paths[0] = pathInferior;
        pathLengths[0] = NUM_WAYPOINTS_BAIXO;
        *pathCount = 1;
    } else {
        paths[0] = pathInferior;
        paths[1] = pathSuperior;
        pathLengths[0] = NUM_WAYPOINTS_BAIXO;
        pathLengths[1] = NUM_WAYPOINTS_CIMA;
        *pathCount = 2;
    }
}

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
        if (CheckCollisionPointRec(mouse, btnContinuar)) { current_game_state = PLAYING; return; }
        if (CheckCollisionPointRec(mouse, btnReiniciar)) { ReiniciarFase(); return; }
        if (CheckCollisionPointRec(mouse, btnMenu)) { VoltarMenuPrincipal(); return; }
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
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    lastHeroDied = false;
    noHeroTimer = 0.0f;

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
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    lastHeroDied = false;
    noHeroTimer = 0.0f;

    gameRecursos.moedas = 100;

    menuAberto = false;
}


Vector2 pathInferior[84] = {
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

Vector2 pathSuperior[96] = {
    { 485, 15 },{ 481, 18 },{ 479, 19 },{ 477, 22 },{ 473, 26 },
    { 464, 29 },{ 464, 30 },{ 459, 33 },{ 450, 39 },{ 442, 45 },
    { 435, 49 },{ 431, 50 },{ 423, 55 },{ 413, 58 },
    { 409, 60 },{ 403, 63 },{ 394, 65 },
    { 383, 68 },{ 371, 73 },
    { 364, 77 },
    { 351, 81 },{ 339, 85 },{ 330, 89 },{ 322, 92 },{ 312, 95 },
    { 303, 100 },
    { 287, 106 },{ 275, 112 },{ 264, 117 },
    { 259, 121 },{ 253, 125 },
    { 245, 129 },{ 233, 133 },{ 219, 140 },{ 210, 144 },{ 198, 148 },
    { 189, 151 },{ 177, 156 },{ 160, 162 },{ 149, 166 },{ 139, 172 },
    { 134, 176 },{ 127, 183 },{ 123, 189 },{ 122, 194 },{ 120, 203 },
    { 120, 208 },{ 126, 214 },{ 135, 220 },{ 147, 225 },
    { 161, 231 },{ 173, 233 },{ 186, 237 },{ 199, 243 },
    { 225, 254 },{ 227, 254 },{ 232, 254 },
    { 244, 255 },{ 253, 255 },
    { 257, 255 },{ 258, 254 },{ 259, 250 },{ 262, 246 },{ 271, 242 },
    { 283, 237 },{ 292, 234 },{ 304, 231 },{ 316, 225 },
    { 326, 221 },{ 335, 219 },
    { 348, 223 },
    { 366, 235 },{ 372, 237 },{ 384, 241 },{ 396, 244 },
    { 412, 248 },{ 435, 251 },{ 450, 251 },
    { 457, 247 },{ 470, 243 },{ 477, 239 },{ 490, 235 },{ 505, 230 },
    { 517, 224 },{ 527, 219 },{ 538, 214 },{ 549, 208 },{ 568, 200 },
    { 581, 195 },{ 589, 190 },{ 603, 184 },{ 615, 179 },
    { 625, 172 },{ 635, 162 },{ 643, 155 },
    { 652, 149 }
};


Vector2 *currentPath = pathInferior;

void InicializarHerois(void) {
  strcpy(herois[0].nome, "Guerreiro");
  herois[0].custo = 50;
  herois[0].dano = 60;
  herois[0].alcance = 80;
  herois[0].texture = LoadTexture("resources/Cavaleiro.png");
  
  strcpy(herois[1].nome, "Bardo");
  herois[1].custo = 100;
  herois[1].dano = 50;
  herois[1].alcance = 110;
  herois[1].texture = LoadTexture("resources/Bardo.png");
  
  strcpy(herois[2].nome, "Paladino");
  herois[2].custo = 120;
  herois[2].dano = 65;
  herois[2].alcance = 120;
  herois[2].texture = LoadTexture("resources/Paladino.png");
  
  strcpy(herois[3].nome, "Mago");
  herois[3].custo = 180;
  herois[3].dano = 95;
  herois[3].alcance = 140;
  herois[3].texture = LoadTexture("resources/SapoMago.png");
  
    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
    heroUpgrades[i].nivel_vida = 0;
    heroUpgrades[i].nivel_dano = 0;
    heroUpgrades[i].nivel_velocidade = 0;
    heroUpgrades[i].vida_total = 100;
    heroUpgrades[i].dano_total = herois[i].dano;
    heroUpgrades[i].velocidade_mult = 1.0f;
  }
}

int ComprarUpgradeHeroi(recursos *r, int tipoHeroi, int tipoUpgrade) {
    if (tipoHeroi < 0 || tipoHeroi >= HERO_TYPE_COUNT) return 0;
  
  int custoupgrade = 0;
  int nivelAtual = 0;

  int prev_vida_total = heroUpgrades[tipoHeroi].vida_total;
  int prev_dano_total = heroUpgrades[tipoHeroi].dano_total;
  float prev_vel_mult = heroUpgrades[tipoHeroi].velocidade_mult;
  
  if (tipoUpgrade == 0) { // Vida
    nivelAtual = heroUpgrades[tipoHeroi].nivel_vida;
    if (nivelAtual >= 5) return 0; // Máximo de 5 níveis
    custoupgrade = 50 * (nivelAtual + 1); // Custo cresce com nível
  } else if (tipoUpgrade == 1) { // Dano
    nivelAtual = heroUpgrades[tipoHeroi].nivel_dano;
    if (nivelAtual >= 5) return 0;
    custoupgrade = 60 * (nivelAtual + 1);
  } else if (tipoUpgrade == 2) { // Velocidade
    nivelAtual = heroUpgrades[tipoHeroi].nivel_velocidade;
    if (nivelAtual >= 5) return 0;
    custoupgrade = 40 * (nivelAtual + 1);
  }
  
  if (r->moedas < custoupgrade) return 0;
  
  r->moedas -= custoupgrade;
  
  if (tipoUpgrade == 0) { // Vida
    heroUpgrades[tipoHeroi].nivel_vida++;
    heroUpgrades[tipoHeroi].vida_total = 100 + (heroUpgrades[tipoHeroi].nivel_vida * 20);
    TraceLog(LOG_INFO, "Vida do %s aumentada para %d! Custo: %d moedas.", herois[tipoHeroi].nome, heroUpgrades[tipoHeroi].vida_total, custoupgrade);
  } else if (tipoUpgrade == 1) { // Dano
    heroUpgrades[tipoHeroi].nivel_dano++;
    heroUpgrades[tipoHeroi].dano_total = herois[tipoHeroi].dano + (heroUpgrades[tipoHeroi].nivel_dano * 15);
    TraceLog(LOG_INFO, "Dano do %s aumentado para %d! Custo: %d moedas.", herois[tipoHeroi].nome, heroUpgrades[tipoHeroi].dano_total, custoupgrade);
  } else if (tipoUpgrade == 2) { // Velocidade
    heroUpgrades[tipoHeroi].nivel_velocidade++;
    heroUpgrades[tipoHeroi].velocidade_mult = 1.0f + (heroUpgrades[tipoHeroi].nivel_velocidade * 0.15f);
    TraceLog(LOG_INFO, "Velocidade do %s aumentada para %.2fx! Custo: %d moedas.", herois[tipoHeroi].nome, heroUpgrades[tipoHeroi].velocidade_mult, custoupgrade);
  }

  int new_vida_total = heroUpgrades[tipoHeroi].vida_total;
  int new_dano_total = heroUpgrades[tipoHeroi].dano_total;
  float new_vel_mult = heroUpgrades[tipoHeroi].velocidade_mult;

    int diffVida = new_vida_total - prev_vida_total;
    (void)new_dano_total; 
    (void)prev_dano_total;
    (void)new_vel_mult;
    (void)prev_vel_mult;

  for (int i = 0; i < placedHeroCount; i++) {
      if (placedHeroes[i].tipo != tipoHeroi) continue;

      placedHeroes[i].dano = new_dano_total;

      if (placedHeroes[i].health > 0) {
          placedHeroes[i].health += diffVida;
          if (placedHeroes[i].health > new_vida_total) placedHeroes[i].health = new_vida_total;
      }
  }

  herois[tipoHeroi].dano = new_dano_total;

  return 1;
}

void DrawMenuUpgrades(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(40, 40, screenWidth - 80, screenHeight - 60, (Color){0, 0, 0, 220});
    DrawRectangleLines(40, 40, screenWidth - 80, screenHeight - 60, GOLD);

    DrawText("UPGRADES DE HERÓIS", screenWidth/2 - MeasureText("UPGRADES DE HERÓIS", 30)/2, 60, 30, GOLD);
    DrawText("Pressione U para voltar",
             screenWidth/2 - MeasureText("Pressione U para voltar", 20)/2,
             100, 20, LIGHTGRAY);

    int cardWidth = 140;
    int cardHeight = 180;
    int spacing = 15;
    int startX = 60;
    int startY = 140;

    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        Color cardColor = (i == selectedUpgradeHero) ? (Color){100, 150, 255, 255} : (Color){50, 50, 80, 255};
        DrawRectangle(cardX, cardY, cardWidth, cardHeight, cardColor);
        DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);

        DrawText(herois[i].nome,
                 cardX + cardWidth/2 - MeasureText(herois[i].nome, 18)/2,
                 cardY + 15,
                 18, YELLOW);

        int textureSize = 110;
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 25;
        DrawTexturePro(herois[i].texture,
                       (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                       (Rectangle){textureX, textureY, textureSize, textureSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }

    int upgradeX = 60;
    int upgradeY = 360;
    int upgradeBoxWidth = screenWidth - 120;
    int upgradeBoxHeight = screenHeight - 440;

    DrawRectangle(upgradeX, upgradeY, upgradeBoxWidth, upgradeBoxHeight, (Color){50, 50, 80, 255});
    DrawRectangleLines(upgradeX, upgradeY, upgradeBoxWidth, upgradeBoxHeight, GOLD);

    if (selectedUpgradeHero < 0 || selectedUpgradeHero >= HERO_TYPE_COUNT) {
        const char *msg = "Clique em um heroi acima para escolher qual evoluir!";
        int msgWidth = MeasureText(msg, 20);
        DrawText(msg, 
                 upgradeX + (upgradeBoxWidth - msgWidth) / 2, 
                 upgradeY + (upgradeBoxHeight - 20) / 2, 
                 20, YELLOW);
        return;
    }

    char heroTitle[50];
    sprintf(heroTitle, "Upgrades: %s", herois[selectedUpgradeHero].nome);
    DrawText(heroTitle, upgradeX + 20, upgradeY + 15, 22, YELLOW);

    char moedaText[50];
    sprintf(moedaText, "Moedas: %d", get_moedas(&gameRecursos));
    DrawText(moedaText, upgradeX + upgradeBoxWidth - 200, upgradeY + 15, 20, GOLD);

    const char *nomeUpgrades[] = {"VIDA", "DANO", "VELOCIDADE"};
    int btnWidth = 200;
    int btnHeight = 100;
    int btnSpacing = 20;
    int totalWidth = (3 * btnWidth) + (2 * btnSpacing);
    int startUpgradeX = upgradeX + (upgradeBoxWidth - totalWidth) / 2;
    int startUpgradeY = upgradeY + 80;

    for (int i = 0; i < 3; i++) {
        int btnX = startUpgradeX + i * (btnWidth + btnSpacing);
        int btnY = startUpgradeY;

        int nivelAtual = 0;
        int custoupgrade = 0;
        char infoText[100];

        if (i == 0) { 
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_vida;
            custoupgrade = 50 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nVida: %d", nivelAtual, heroUpgrades[selectedUpgradeHero].vida_total);
        } else if (i == 1) { 
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_dano;
            custoupgrade = 60 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nDano: %d", nivelAtual, heroUpgrades[selectedUpgradeHero].dano_total);
        } else if (i == 2) { 
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_velocidade;
            custoupgrade = 40 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nVel: %.2fx", nivelAtual, heroUpgrades[selectedUpgradeHero].velocidade_mult);
        }

        Color btnColor = (nivelAtual >= 5) ? (Color){100, 100, 100, 255} : 
                         (gameRecursos.moedas >= custoupgrade) ? (Color){0, 150, 0, 255} : (Color){150, 0, 0, 255};

        DrawRectangle(btnX, btnY, btnWidth, btnHeight, btnColor);
        DrawRectangleLines(btnX, btnY, btnWidth, btnHeight, WHITE);

        DrawText(nomeUpgrades[i],
                 btnX + btnWidth/2 - MeasureText(nomeUpgrades[i], 20)/2,
                 btnY + 10,
                 20, WHITE);

        DrawText(infoText,
                 btnX + btnWidth/2 - MeasureText(infoText, 16)/2,
                 btnY + 40,
                 16, LIGHTGRAY);

        char custoText[30];
        sprintf(custoText, "Custo: %d", custoupgrade);
        DrawText(custoText,
                 btnX + btnWidth/2 - MeasureText(custoText, 14)/2,
                 btnY + 80,
                 14, GOLD);
    }
}

void VerificarCliqueUpgrades(void) {
    if (currentWave < 2) return; 
    
    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (!mousePressed) return;

    int cardWidth = 140;
    int spacing = 15;
    int startX = 60;
    int startY = 140;

    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        Rectangle cardRect = { cardX, cardY, cardWidth, 180 };
        if (CheckCollisionPointRec(mousePos, cardRect)) {
            selectedUpgradeHero = i;
            TraceLog(LOG_INFO, "Herói %d selecionado!", i);
            return;
        }
    }

    int screenWidth = GetScreenWidth();
    int btnWidth = 200;
    int btnHeight = 100;
    int btnSpacing = 20;
    int upgradeX = 60;
    int upgradeY = 360;
    int upgradeBoxWidth = screenWidth - 120;
    int totalWidth = (3 * btnWidth) + (2 * btnSpacing);
    int startUpgradeX = upgradeX + (upgradeBoxWidth - totalWidth) / 2;
    int startUpgradeY = upgradeY + 80;

    for (int i = 0; i < 3; i++) {
        int btnX = startUpgradeX + i * (btnWidth + btnSpacing);
        int btnY = startUpgradeY;

        Rectangle btnRect = { btnX, btnY, btnWidth, btnHeight };
        if (CheckCollisionPointRec(mousePos, btnRect)) {
            if (ComprarUpgradeHeroi(&gameRecursos, selectedUpgradeHero, i)) {
                TraceLog(LOG_INFO, "Upgrade adquirido!");
            } else {
                TraceLog(LOG_WARNING, "Não foi possível comprar upgrade!");
            }
            return;
        }
    }
}

int ComprarHeroiEspecifico(recursos *r, int tipoHeroi) {
  TraceLog(LOG_DEBUG, "TENTATIVA COMPRA: tipo=%d, wave=%d, moedas=%d, placed=%d, HERO_TYPES=%d", tipoHeroi, currentWave, r->moedas, placedHeroCount, HERO_TYPE_COUNT);
  if (tipoHeroi >= 0 && tipoHeroi < HERO_TYPE_COUNT) {
        if (r->moedas >= herois[tipoHeroi].custo) {
            r->moedas -= herois[tipoHeroi].custo;
            placementMode = true;
            selectedHeroType = tipoHeroi;
            menuAberto = false; 
            TraceLog(LOG_INFO, "%s comprado! Clique no mapa para colocar. Moedas restantes: %d", herois[tipoHeroi].nome, r->moedas);
            return 1; 
        } else {
            TraceLog(LOG_WARNING, "Moedas insuficientes para comprar %s: custo=%d, moedas=%d", herois[tipoHeroi].nome, herois[tipoHeroi].custo, r->moedas);
        }
  } else {
      TraceLog(LOG_WARNING, "Tipo de heroi inválido: %d", tipoHeroi);
  }
  return 0; 
}

void DrawMenuHerois(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(40, 40, screenWidth - 80, screenHeight - 60, (Color){0, 0, 0, 220});
    DrawRectangleLines(40, 40, screenWidth - 80, screenHeight - 60, GOLD);

    DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 60, 30, GOLD);
    DrawText("Pressione H para fechar",
             screenWidth/2 - MeasureText("Pressione H para fechar", 20)/2,
             100, 20, LIGHTGRAY);

    int cardWidth  = 160;
    int cardHeight = 320;

    int spacing = 30;
    int startX = (screenWidth - (HERO_TYPE_COUNT * cardWidth + (HERO_TYPE_COUNT - 1) * spacing)) / 2;
    int startY = 180;

    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        DrawRectangle(cardX, cardY, cardWidth, cardHeight, (Color){50, 50, 80, 255});
        DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);

        DrawText(herois[i].nome,
                 cardX + cardWidth/2 - MeasureText(herois[i].nome, 20)/2,
                 cardY + 10,
                 20, YELLOW);

        int textureSize = 85;
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 45;
        DrawTexturePro(herois[i].texture,
                       (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                       (Rectangle){textureX, textureY, textureSize, textureSize},
                       (Vector2){0, 0}, 0.0f, WHITE);

        int statsStartY = cardY + 150;

        char custoTxt[32];
        char danoTxt[32];
        char alcanceTxt[32];

        sprintf(custoTxt,   "Custo: %d$", herois[i].custo);
        sprintf(danoTxt,    "Dano: %d",   herois[i].dano);
        sprintf(alcanceTxt, "Alcance: %d",herois[i].alcance);

        DrawText(custoTxt,
                 cardX + cardWidth/2 - MeasureText(custoTxt, 14)/2,
                 statsStartY,
                 14, GOLD);

        DrawText(danoTxt,
                 cardX + cardWidth/2 - MeasureText(danoTxt, 14)/2,
                 statsStartY + 20,
                 14, RED);

        DrawText(alcanceTxt,
                 cardX + cardWidth/2 - MeasureText(alcanceTxt, 14)/2,
                 statsStartY + 40,
                 14, BLUE);

        Color btnColor = (gameRecursos.moedas >= herois[i].custo) ? GREEN : RED;

        int btnWidth  = cardWidth - 20;
        int btnHeight = 40;
        int btnX = cardX + (cardWidth - btnWidth) / 2;
        int btnY = cardY + cardHeight - btnHeight - 10;

        DrawRectangle(btnX, btnY, btnWidth, btnHeight, btnColor);
        DrawRectangleLines(btnX, btnY, btnWidth, btnHeight, WHITE);

        DrawText("COMPRAR",
                 btnX + btnWidth/2 - MeasureText("COMPRAR", 16)/2,
                 btnY + 8,
                 16, WHITE);
    }

    DrawText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões",
             screenWidth/2 - MeasureText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 16)/2,
             startY + cardHeight + 50,
             16, LIGHTGRAY);
    
    if (currentWave >= 2) {
        Rectangle btnUpgrades = { screenWidth - 150, 55, 120, 35 };
        Color btnUpgradesColor = CheckCollisionPointRec(GetMousePosition(), btnUpgrades) ? YELLOW : GOLD;
        DrawRectangleRec(btnUpgrades, btnUpgradesColor);
        DrawRectangleLines((int)btnUpgrades.x, (int)btnUpgrades.y, (int)btnUpgrades.width, (int)btnUpgrades.height, WHITE);
        DrawText("U - UPGRADES", btnUpgrades.x + 10, btnUpgrades.y + 8, 14, BLACK);
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btnUpgrades)) {
           
            menuUpgradesAberto = true;
            menuAberto = false;
            selectedUpgradeHero = -1; 
            return;
        }
    }
}

void VerificarCliqueMenu(void) {
    Vector2 mousePos = GetMousePosition();
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    int screenWidth = GetScreenWidth();

    int cardWidth = 160;
    int cardHeight = 320;
    int spacing = 30;
    int startX = (screenWidth - (HERO_TYPE_COUNT * cardWidth + (HERO_TYPE_COUNT - 1) * spacing)) / 2;
    int startY = 180;

    if (!mouseReleased) return;

    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        int btnWidth = cardWidth - 20;
        int btnHeight = 40;
        int btnX = cardX + (cardWidth - btnWidth) / 2;
        int btnY = cardY + cardHeight - btnHeight - 10;

        int hitboxPadding = 15;
        Rectangle btnRect = { 
            btnX - hitboxPadding, 
            btnY - hitboxPadding, 
            btnWidth + (hitboxPadding * 2), 
            btnHeight + (hitboxPadding * 2) 
        };

        if (CheckCollisionPointRec(mousePos, btnRect)) {
            if (ComprarHeroiEspecifico(&gameRecursos, i)) {
                TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[i].nome, gameRecursos.moedas);
            } else {
                TraceLog(LOG_WARNING, "Não foi possível comprar %s! Moedas: %d", herois[i].nome, gameRecursos.moedas);
            }
            break;
        }
    }
}

void IniciarFase2(void) {
    TraceLog(LOG_INFO, "Iniciando Fase 2...");
    
    gameRecursos.moedas += WAVE_REWARD;
    
    currentWave = 2;
    
    
    background = backgroundFase2; 
    
   
    enemyCount = 0;
    enemies_defeated_count = 0;
    total_enemies_spawned = 0; 
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    current_game_state = PLAYING; 

    phaseStartTime = gameTimer;
    ResetHeatmap();
    
    for (int i = 0; i < placedHeroCount; i++) {
        placedHeroes[i].health = 0;
        placedHeroes[i].is_burning = false;
        placedHeroes[i].burn_timer = 0.0f;
    }

    spawnTimer = 0.0f; 
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemyLastAttackTime[i] = 0.0f;
        enemyTargetHero[i] = -1;
        enemies[i].active = 0;
        enemies[i].is_burning = false;
        enemies[i].burning_timer = 0.0f;
    }
    
    TraceLog(LOG_INFO, "Recompensa de %d moedas concedida! Total: %d", WAVE_REWARD, gameRecursos.moedas);
}

void IniciarFase3(void) {
    TraceLog(LOG_INFO, "Iniciando Fase 3...");
    
    gameRecursos.moedas += WAVE_REWARD;
    
    currentWave = 3;
    
    
    enemyCount = 0;
    enemies_defeated_count = 0; 
    total_enemies_spawned = 0;
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    current_game_state = PLAYING; 
    background = backgroundFase3;
    if (placedHeroes) {
        free(placedHeroes);
        placedHeroes = NULL;
    }
    placedHeroCount = 0;
    placedHeroCapacity = 0;

    spawnTimer = 0.0f; 
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemyLastAttackTime[i] = 0.0f;
        enemyTargetHero[i] = -1;
        enemies[i].active = 0;
        enemies[i].is_burning = false;
        enemies[i].burning_timer = 0.0f;
    }
    
    TraceLog(LOG_INFO, "Recompensa de %d moedas concedida! Total: %d", WAVE_REWARD, gameRecursos.moedas);
    
    phaseStartTime = gameTimer;
    
    ResetHeatmap();
}

void InitGame(void) {

    currentWave = 1;
    total_enemies_spawned = 0; 

    background = LoadTexture("resources/background_novo.jpg");
    towerTexture = LoadTexture("resources/tower.png");
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");
    backgroundFase3 = LoadTexture("resources/backgroundFase3.jpg");

    
    inicializar_recursos(&gameRecursos);
    InicializarHerois();

    current_game_state = PLAYING;
    towerHealth = CASTLE_MAX_HEALTH;
    enemies_defeated_count = 0;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;

    gameTimer = 0.0;
    rankingSaved = false;

    
    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].pathIndex = 0; 
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    
    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    menuAberto = false;
    
    SetRandomSeed(GetTime()); 
    InitHeatmap();
}


void UpdateGame(void) {
    float dt = GetFrameTime();

    if (currentWave == 1) {
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active) {
                enemies[i].pathIndex = 0;
            }
        }
    }
    if (IsKeyPressed(KEY_P)) {
        if (current_game_state == PLAYING) {
            current_game_state = PAUSED;
        }
        else if (current_game_state == PAUSED) {
            current_game_state = PLAYING;
        }
    }

    if (current_game_state == PAUSED) {
        
        if (menuAberto) {
            VerificarCliqueMenu();
            return;
        }
        return;
    }

    if (IsKeyPressed(KEY_H)) {
        menuAberto = !menuAberto;  
        menuUpgradesAberto = false; 
        return;
    }

    if (IsKeyPressed(KEY_U) && currentWave >= 2) {
        menuUpgradesAberto = !menuUpgradesAberto;
        menuAberto = false; 
        sellMode = false; 
        if (menuUpgradesAberto) {
            selectedUpgradeHero = -1; 
            TraceLog(LOG_INFO, "Menu de upgrades aberto!");
        }
        return;
    }

    if (IsKeyPressed(KEY_V)) {
        sellMode = !sellMode;
        menuAberto = false;
        menuUpgradesAberto = false; 
        selectedHeroToSell = -1;
        TraceLog(LOG_INFO, "Modo de venda: %s", sellMode ? "ATIVADO" : "DESATIVADO");
        return;
    }

    if (IsKeyPressed(KEY_T)) {
        showHeatmap = !showHeatmap;
        TraceLog(LOG_INFO, "Heatmap: %s", showHeatmap ? "ON" : "OFF");
    }

    if (current_game_state == WAVE_WON || current_game_state == GAME_OVER) {
 
        return;
    }


    if (menuAberto) {
        VerificarCliqueMenu();
        return;
    }

    if (menuUpgradesAberto) {
        VerificarCliqueUpgrades();
        return;
    }

        if (sellMode) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                bool heroVendido = false;
                
                for (int i = 0; i < placedHeroCount; i++) {
                    if (placedHeroes[i].health <= 0) continue;
                    
                    float dx = mousePos.x - placedHeroes[i].x;
                    float dy = mousePos.y - placedHeroes[i].y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    
                    if (dist <= 50 && placedHeroes[i].health == 100) {
                        int sell_price = (herois[placedHeroes[i].tipo].custo * 70) / 100;
                        adicionar_moedas(&gameRecursos, sell_price);
                        
                        sprintf(sellMessage, "Vendido! +%d moedas", sell_price);
                        sellMessageTimer = 2.0f;
                        
                        TraceLog(LOG_INFO, "Herói %s vendido por %d moedas!", herois[placedHeroes[i].tipo].nome, sell_price);
                        
                        for (int j = i; j < placedHeroCount - 1; j++) {
                            placedHeroes[j] = placedHeroes[j + 1];
                        }
                        placedHeroCount--;
                        
                        heroVendido = true;
                        lastHeroDied = false;
                        break;
                    }
                    else if (dist <= 50 && placedHeroes[i].health < 100) {
                        sprintf(sellMessage, "Herói danificado! Recupere a vida antes de vender");
                        sellMessageTimer = 2.0f;
                        
                        TraceLog(LOG_WARNING, "Este herói está danificado! Recupere a vida antes de vender.");
                        heroVendido = true;
                        break;
                    }
                }
                
                if (heroVendido) {
                    selectedHeroToSell = -1;
                    sellMode = false;
                }
            }
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                sellMode = false;
                selectedHeroToSell = -1;
            }
            return;
        }


    if (placementMode) {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            Vector2 mousePos = GetMousePosition();

            int heroSize = 80;
            int halfSize = heroSize / 2;
            int x = (int)mousePos.x;
            int y = (int)mousePos.y;

            if (x - halfSize < 0) x = halfSize;
            if (y - halfSize < 0) y = halfSize;
            if (x + halfSize > GetScreenWidth()) x = GetScreenWidth() - halfSize;
            if (y + halfSize > GetScreenHeight()) y = GetScreenHeight() - halfSize;

            EnsurePlacedCapacity(placedHeroCount + 1);
            placedHeroes[placedHeroCount].x = x;
            placedHeroes[placedHeroCount].y = y;
            placedHeroes[placedHeroCount].tipo = selectedHeroType;
            placedHeroes[placedHeroCount].dano = heroUpgrades[selectedHeroType].dano_total;
            placedHeroes[placedHeroCount].alcance = herois[selectedHeroType].alcance;
            placedHeroes[placedHeroCount].health = heroUpgrades[selectedHeroType].vida_total; 
             placedHeroes[placedHeroCount].lastAttackTime = 0;
             placedHeroes[placedHeroCount].texture = herois[selectedHeroType].texture;
              placedHeroes[placedHeroCount].is_burning = false;
              placedHeroes[placedHeroCount].burn_timer = 0.0f;

            lastHeroDied = false;
            
            placedHeroCount++;

            placementMode = false;
            selectedHeroType = -1;
        }

        return;
    }

    if (current_game_state != PLAYING) return;

    gameTimer += dt;

    if (lastHeroDied) {
        int alive = 0;
        for (int h = 0; h < placedHeroCount; h++) {
            if (placedHeroes[h].health > 0) { alive++; break; }
        }
        
        if (alive == 0) {
            noHeroTimer += dt;
            if (noHeroTimer >= 1.0f) {
                int seconds = (int)noHeroTimer;
                adicionar_moedas(&gameRecursos, 5 * seconds);
                noHeroTimer -= (float)seconds;
            }
        } else {
            noHeroTimer = 0.0f;
        }
    } else {
        noHeroTimer = 0.0f;
    }
    for (int i = 0; i < placedHeroCount; i++) {
         if (placedHeroes[i].health <= 0) continue;
 
         float minDist = (float)placedHeroes[i].alcance;
         int targetEnemy = -1;
 
         for (int j = 0; j < enemyCount; j++) {
             if (!enemies[j].active) continue;
 
             float dx = placedHeroes[i].x - enemies[j].x;
             float dy = placedHeroes[i].y - enemies[j].y;
             float dist = sqrtf(dx*dx + dy*dy);
 
             if (dist < minDist) {
                 minDist = dist;
                 targetEnemy = j;
             }
         }
 
         if (targetEnemy != -1) {
                placedHeroes[i].lastAttackTime += dt;
                float velMult = heroUpgrades[placedHeroes[i].tipo].velocidade_mult;
                if (velMult <= 0.0f) velMult = 1.0f;
                float attackInterval = 1.0f / velMult;
 
                if (placedHeroes[i].lastAttackTime >= attackInterval) {
                     
                     int hero_bit = (1 << placedHeroes[i].tipo);
                     
                     if ((enemies[targetEnemy].resistance & hero_bit) == 0) {
                        enemies[targetEnemy].health -= placedHeroes[i].dano;
                        
                        if (enemies[targetEnemy].health <= 0) {
                            adicionar_moedas(&gameRecursos, enemies[targetEnemy].recompensa_moedas);
                            enemies[targetEnemy].active = 0;
                            enemies_defeated_count++;
                        }
                     } else {
                         TraceLog(LOG_WARNING, "Inimigo %d é resistente ao Herói %s!", targetEnemy, herois[placedHeroes[i].tipo].nome);
                     }
 
                    placedHeroes[i].lastAttackTime = 0;
                 }
             }
        
         }
        bool shouldSpawn = (towerHealth > 0);
        if (currentWave < 3) {
            shouldSpawn = shouldSpawn && (total_enemies_spawned < WAVE_SIZE[currentWave]);
        }
        
        if (shouldSpawn) { 
            spawnTimer += dt;
                float spawnInterval;
                if (currentWave == 3) {
                    double elapsed = gameTimer - phaseStartTime;
                    float decayFactor = expf(-SPAWN_DECAY_RATE * (float)elapsed);
                    spawnInterval = PHASE3_BASE_SPAWN * decayFactor;
                    if (spawnInterval < PHASE3_MIN_SPAWN) spawnInterval = PHASE3_MIN_SPAWN;
                } else {
                    spawnInterval = SPAWN_INTERVAL;
                }
                if (spawnTimer >= spawnInterval) {
                int activeEnemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) activeEnemies++;
                }
                
                if (activeEnemies < MAX_ENEMIES) {
                    EnemyType newEnemyType = GetRandomEnemyType(); 
                    
                    float startX, startY;
                    int pathIndex = 0;
                    
                    if (currentWave == 1) {
                        startX = pathInferior[0].x;
                        startY = pathInferior[0].y;
                        pathIndex = 0;
                    } else {
                        if (GetRandomValue(0, 1) == 0) {
                            startX = pathInferior[0].x;
                            startY = pathInferior[0].y;
                            pathIndex = 0;
                        } else {
                            startX = pathSuperior[0].x;
                            startY = pathSuperior[0].y;
                            pathIndex = 1;
                        }
                    }
                    
                    if (currentWave < 3) {
                        total_enemies_spawned++;
                    }
                    
                    enemies[enemyCount] = InitEnemy(startX, startY, newEnemyType);
                    enemies[enemyCount].pathIndex = pathIndex;
                    enemyLastAttackTime[enemyCount] = 0;
                    enemyTargetHero[enemyCount] = -1;
                    enemyCount++;
                }
                spawnTimer = 0;
            }
        }
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        
        if (enemies[i].type == INIMIGO_SPECTRO) {
            enemies[i].resistance = 0x07; 
        } 
        else if (enemies[i].type == INIMIGO_NECROMANTE) {
            enemies[i].resistance = 0x03; 
        }
        
        if (enemies[i].type == INIMIGO_NECROMANTE) {
            enemies[i].necromante_heal_timer -= dt;

            if (enemies[i].necromante_heal_timer <= 0) {
                int heal_amount = (int)(enemies[i].maxHealth * 0.15f);
                
                for (int j = 0; j < enemyCount; j++) {
                    if (enemies[j].active) {
                        float dist = Vector2Distance((Vector2){enemies[i].x, enemies[i].y}, (Vector2){enemies[j].x, enemies[j].y});
                        if (dist <= enemies[i].range) {
                            enemies[j].health += heal_amount;
                            if (enemies[j].health > enemies[j].maxHealth) {
                                enemies[j].health = enemies[j].maxHealth;
                            }
                        }
                    }
                }
                enemies[i].necromante_heal_timer = 5.0f; 
            }
        }
    }


        if (currentWave < MAX_WAVES && currentWave < 3) {
            if (enemies_defeated_count >= WAVE_SIZE[currentWave]) {

                int activeEnemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        activeEnemies++;
                        break; 
                    }
                }
                
                if (activeEnemies == 0) {
                    current_game_state = WAVE_WON; 
                }
            }
        }
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;

        if (EnemyReachedTower(enemies[i])) {
            if (enemies[i].type == INIMIGO_DRAGAO) {
                is_tower_burning = true;
                tower_burn_timer = 5.0f;
            }
        }

        int targetHero = enemyTargetHero[i];

        if (targetHero == -1 || targetHero >= placedHeroCount || placedHeroes[targetHero].health <= 0) {
            targetHero = -1;
            float minDist = enemies[i].range;

            for (int h = 0; h < placedHeroCount; h++) {
                if (placedHeroes[h].health <= 0) continue;

                float dx = enemies[i].x - placedHeroes[h].x;
                float dy = enemies[i].y - placedHeroes[h].y;
                float dist = sqrtf(dx*dx + dy*dy);

                if (dist <= minDist) {
                    minDist = dist;
                    targetHero = h;
                }
            }
            enemyTargetHero[i] = targetHero;
        }

        if (targetHero != -1 && targetHero < placedHeroCount) {
            
            float dx = enemies[i].x - placedHeroes[targetHero].x;
            float dy = enemies[i].y - placedHeroes[targetHero].y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist <= enemies[i].range) {
                enemyLastAttackTime[i] += dt;

                if (enemyLastAttackTime[i] >= ENEMY_ATTACK_INTERVAL) {
                    placedHeroes[targetHero].health -= enemies[i].damage;

                    if (enemies[i].type == INIMIGO_DRAGAO) {
                        placedHeroes[targetHero].is_burning = true;
                        placedHeroes[targetHero].burn_timer = 2.5f;
                    }

                    if (placedHeroes[targetHero].health <= 0) {
                        placedHeroes[targetHero].health = 0;
                        lastHeroDied = true; 
                        enemyTargetHero[i] = -1;
                    }

                    enemyLastAttackTime[i] = 0;
                }
            } else {
                enemyTargetHero[i] = -1;
                UpdateEnemy(&enemies[i]);
            }
        } else {
            UpdateEnemy(&enemies[i]);
        }
    }

    if (heatmap) {
        for (int _hi = 0; _hi < enemyCount; _hi++) {
            if (enemies[_hi].active) HeatmapIncrementAt(enemies[_hi].x, enemies[_hi].y);
        }
    }

    tower_attack_timer += dt;
    if (tower_attack_timer >= TOWER_ATTACK_INTERVAL) {
        tower_attack_timer = 0.0f;
        
        Vector2 towerPos = {650 + 40, 100 + 40};
        
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].active) continue;
            
            float dx = enemies[i].x - towerPos.x;
            float dy = enemies[i].y - towerPos.y;
            float dist = sqrtf(dx*dx + dy*dy);
            
            if (dist <= TOWER_ATTACK_RANGE) {
                enemies[i].health -= TOWER_ATTACK_DAMAGE;
                
                TraceLog(LOG_DEBUG, "Torre atacou inimigo! Dano: %d, Vida restante: %d", TOWER_ATTACK_DAMAGE, enemies[i].health);
                
                if (enemies[i].health <= 0) {
                    enemies[i].active = 0;
                    enemies[i].health = 0;
                    TraceLog(LOG_INFO, "Inimigo foi destruído pela torre!");
                }
            }
        }
    }
    
    Vector2 towerPos = {650 + 40, 100 + 40};
    static float tower_enemy_attack_timer = 0.0f;
    
    tower_enemy_attack_timer += dt;
    
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        float dx = enemies[i].x - towerPos.x;
        float dy = enemies[i].y - towerPos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        
        if (dist <= enemies[i].range && EnemyReachedTower(enemies[i])) {
          
            if (tower_enemy_attack_timer >= ENEMY_ATTACK_INTERVAL) {
                towerHealth -= enemies[i].damage;
                
                TraceLog(LOG_WARNING, "Inimigo atacou a torre! Dano: %d, HP restante: %d", enemies[i].damage, towerHealth);
                
                if (towerHealth <= 0) {
                    towerHealth = 0;
                    current_game_state = GAME_OVER;
                    TraceLog(LOG_ERROR, "Torre foi destruída!");
                }
                
                tower_enemy_attack_timer = 0.0f;
            }
        }
    }


    if (is_tower_burning) {
        tower_burn_timer -= dt;
        
        towerHealth -= (int)(CASTLE_MAX_HEALTH * 0.10f * dt); 
        
        if (towerHealth <= 0) {
            towerHealth = 0;
            current_game_state = GAME_OVER;
            return;
        }

        if (tower_burn_timer <= 0.0f) {
            is_tower_burning = false;
        }
    }
    
    for (int i = 0; i < placedHeroCount; i++) {
        if (placedHeroes[i].health > 0 && placedHeroes[i].is_burning) {
            placedHeroes[i].burn_timer -= dt;
            
            placedHeroes[i].health -= (int)(40 * dt); 
            
            if (placedHeroes[i].health <= 0) {
                placedHeroes[i].health = 0;
                placedHeroes[i].is_burning = false;
                lastHeroDied = true; 
            }
            
            if (placedHeroes[i].burn_timer <= 0.0f) {
                placedHeroes[i].is_burning = false;
            }
        }
    }

    if (currentWave < 3 && enemyCount >= MAX_ENEMIES &&
        enemies_defeated_count >= MAX_ENEMIES) {

        current_game_state = WAVE_WON;
    }
}

void DrawGameUI(void) {

  DrawRectangle(10, 10, 220, 150, Fade(BLACK, 0.7f));
  DrawRectangleLines(10, 10, 220, 150, Fade(GOLD, 0.6f));
  
  DrawText("TORRE", 20, 17, 12, GOLD);
  DrawText(TextFormat("HP: %d", towerHealth), 20, 32, 18, RED);
 
  if (is_tower_burning) {
    DrawText("QUEIMANDO!", 20, 52, 11, ORANGE);
  }
  
  DrawText("MOEDAS:", 20, 67, 12, GOLD);
  DrawText(TextFormat("%d", get_moedas(&gameRecursos)), 20, 82, 18, YELLOW);
  
  DrawLine(10, 105, 230, 105, Fade(GOLD, 0.4f));
  
  DrawText("[H] Loja", 20, 112, 13, Fade(GOLD, 0.8f));
  DrawText("[U] Upgrades", 20, 128, 13, Fade(GOLD, 0.8f));
  DrawText("[V] Vender", 20, 144, 13, Fade(GOLD, 0.8f));

  if (placementMode) {
    int textWidth = MeasureText("Clique no mapa para colocar o heroi", 18);
    DrawRectangle(GetScreenWidth()/2 - textWidth/2 - 10, 15, textWidth + 20, 35, Fade(YELLOW, 0.2f));
    DrawRectangleLines(GetScreenWidth()/2 - textWidth/2 - 10, 15, textWidth + 20, 35, YELLOW);
    DrawText("Clique no mapa para colocar o heroi", GetScreenWidth()/2 - textWidth/2, 22, 18, YELLOW);
  }

  int totalSeconds = (int)gameTimer;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  char timeText[32];
  sprintf(timeText, "%02d:%02d", minutes, seconds);
  int timeTextWidth = MeasureText(timeText, 32);
  int timerX = GetScreenWidth() / 2 - timeTextWidth / 2;
  
  DrawRectangle(timerX - 20, 10, timeTextWidth + 40, 50, BLACK);
  DrawRectangleLines(timerX - 20, 10, timeTextWidth + 40, 50, GOLD);
  
  DrawText(timeText, timerX, 20, 32, WHITE);
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

    DrawHeatmap();

    if (current_game_state == PAUSED) {

        DrawPause();

        if (menuAberto) {
            DrawMenuHerois();
        }

        EndDrawing();
        return;   
    }

    
    DrawTexture(towerTexture, 650, 100, WHITE);
    
    DrawGameUI();

    if (current_game_state == PLAYING) {

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

            DrawTexturePro(placedHeroes[i].texture,
                (Rectangle){0, 0, placedHeroes[i].texture.width, placedHeroes[i].texture.height},
                (Rectangle){placedHeroes[i].x - 40, placedHeroes[i].y - 40, 80, 80},
                (Vector2){40, 40}, 0.0f, WHITE);

            DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});

            if (placedHeroes[i].is_burning) {
                DrawCircle(placedHeroes[i].x + 20, placedHeroes[i].y - 20, 5, ORANGE);
            }


            int barWidth = 40;
            int barHeight = 5;
            int barX = placedHeroes[i].x - barWidth / 2;
            int barY = placedHeroes[i].y - 50;

            DrawRectangle(barX, barY, barWidth, barHeight, RED);
            DrawRectangle(barX, barY, (int)(barWidth * (placedHeroes[i].health / 100.0f)), barHeight, GREEN);
        }
    }

    if (menuAberto) {
        DrawMenuHerois();
    }
    
    if (menuUpgradesAberto && currentWave >= 2) {
        DrawMenuUpgrades();
    }
    
    if (sellMode) {
        float pulse = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
        
        for (int i = 0; i < placedHeroCount; i++) {
            if (placedHeroes[i].health <= 0) continue;
            
            if (placedHeroes[i].health == 100) {
                float circleRadius = 55 + (5 * pulse);
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius + 5, Fade(GOLD, 0.3f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius + 2, Fade(GOLD, 0.6f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius, GOLD);
                
                DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 45, Fade(GOLD, 0.15f));
                
                DrawText("$", placedHeroes[i].x - 5, placedHeroes[i].y - 65, 20, GOLD);
                
                int sell_price = (herois[placedHeroes[i].tipo].custo * 70) / 100;
                char priceText[20];
                sprintf(priceText, "+%d", sell_price);
                int textWidth = MeasureText(priceText, 16);
                DrawText(priceText, placedHeroes[i].x - textWidth/2, placedHeroes[i].y - 85, 16, GOLD);
            } else {
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, 55, Fade(DARKGRAY, 0.4f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, 58, Fade(DARKGRAY, 0.3f));
                
                DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 45, Fade(DARKGRAY, 0.1f));
                DrawText("X", placedHeroes[i].x - 5, placedHeroes[i].y - 8, 20, Fade(RED, 0.5f));
            }
        }
        
        int panelWidth = 480;
        int panelHeight = 100;
        int panelX = GetScreenWidth()/2 - panelWidth/2;
        int panelY = 40;
        
        DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(BLACK, 0.8f));
        DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GOLD);
        DrawRectangleLines(panelX+2, panelY+2, panelWidth-4, panelHeight-4, Fade(GOLD, 0.5f));
        
        const char *titleSell = "MODO VENDA ATIVADO";
        int titleWidth = MeasureText(titleSell, 22);
        DrawText(titleSell, panelX + panelWidth/2 - titleWidth/2, panelY + 10, 22, GOLD);
        
        const char *instrSell = "Clique em um heroi com VIDA CHEIA para vender";
        int instrWidth = MeasureText(instrSell, 14);
        DrawText(instrSell, panelX + panelWidth/2 - instrWidth/2, panelY + 38, 14, WHITE);
        
        const char *cancelSell = "ESC = Cancelar | Verde = Vendável | Cinza = Danificado";
        int cancelWidth = MeasureText(cancelSell, 12);
        DrawText(cancelSell, panelX + panelWidth/2 - cancelWidth/2, panelY + 58, 12, LIGHTGRAY);
    }
    
    if (current_game_state == GAME_OVER) {
        if (!rankingSaved) {
            Ranking_Add(gameTimer, playerName);
            Ranking_Save();
            rankingSaved = true;
        }
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RED, 0.8f)); 
        const char *message = "VOCÊ PERDEU! A TORRE FOI DESTRUÍDA.";
        int message_len = MeasureText(message, 40);
        DrawText(message, (GetScreenWidth() / 2) - (message_len / 2),
                 GetScreenHeight() / 2 - 50, 40, WHITE);
        
        Rectangle botaoMenu = { (GetScreenWidth()/2) - 100, GetScreenHeight()/2 + 50, 200, 50 };
        DrawRectangleRec(botaoMenu, CheckCollisionPointRec(GetMousePosition(), botaoMenu) ? GRAY : DARKGRAY);
        DrawText("MENU PRINCIPAL", botaoMenu.x + 15, botaoMenu.y + 15, 20, WHITE);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), botaoMenu)) {
               current_game_state = MENU;
        }

         int topN = 5;
         int rankingStartY = GetScreenHeight()/2 + 120;
         int blockWidth = 400;
         int rankingX = GetScreenWidth()/2 - blockWidth/2;
         Ranking_Draw(rankingX, rankingStartY, topN);

    }

    else if (current_game_state == WAVE_WON) {
        if (!rankingSaved) {
            Ranking_Add(gameTimer, playerName);
            Ranking_Save();
            rankingSaved = true;
        }
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGREEN, 0.8f));
        
        const char *message;
        const char *button_text;
        
        if (currentWave == 1) {
            message = "IR PARA A FASE 2!";
            button_text = "INICIAR FASE 2";
        } else if (currentWave == 2) {
            message = "IR PARA A FASE 3!"; 
            button_text = "INICIAR FASE 3";
        } else {
            message = "PARABÉNS! VOCÊ VENCEU O JOGO!";
            button_text = "VOLTAR AO MENU";
        }
        
        int message_len = MeasureText(message, 40);
        DrawText(message, (GetScreenWidth() / 2) - (message_len / 2),
                 GetScreenHeight() / 2 - 50, 40, GOLD);
        
        Rectangle button_rect = { (GetScreenWidth() / 2) - 100,
                                 GetScreenHeight() / 2 + 50,
                                 200, 50 };
        Color buttonColor = CheckCollisionPointRec(GetMousePosition(), button_rect) ? GOLD : YELLOW;
        
        DrawRectangleRec(button_rect, buttonColor);
        DrawText(button_text, button_rect.x + (strcmp(button_text, "VOLTAR AO MENU") == 0 ? 5 : 20), button_rect.y + 15, 20, BLACK);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_rect)) {
            if (currentWave == 1) {
                  IniciarFase2();
            } else if (currentWave == 2) {
                  IniciarFase3(); 
            } else {
                  current_game_state = MENU; 
            }
        }
    }

    EndDrawing();
    
}


void ResetGame(void)
{
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;

    enemyCount = 0;
    enemies_defeated_count = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    placedHeroCount = 0;
    if (placedHeroes) {
        free(placedHeroes);
        placedHeroes = NULL;
        placedHeroCapacity = 0;
    }

    ResetHeatmap();

    inicializar_recursos(&gameRecursos);

    menuAberto = false;
    placementMode = false;
    selectedHeroType = -1;
    lastHeroDied = false;
    noHeroTimer = 0.0f;
 
    current_game_state = PLAYING;
    gameTimer = 0.0;
    rankingSaved = false;
}

void CloseGame(void) {
    UnloadTexture(background);
    UnloadTexture(backgroundFase2);
    
  UnloadTexture(towerTexture);
  
    for (int i = 0; i < HERO_TYPE_COUNT; i++) {
        UnloadTexture(herois[i].texture);
    }

        FreeHeatmap();

      if (!rankingSaved && current_game_state == PLAYING) {
          Ranking_Add(gameTimer, playerName);
            Ranking_Save();
            rankingSaved = true;
    }

    Ranking_Free();
}