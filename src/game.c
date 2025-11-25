// game.c

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

const int WAVE_SIZE[] = {0, 15, 25, 40};

// ==============================
// TEXTURAS GLOBAIS DO JOGO
// ==============================
// ⚠️ NO SEU ARQUIVO .c (ex: src/game.c), onde a variável é definida
Texture2D background;
Texture2D backgroundFase2; 
Texture2D backgroundFase3;
Texture2D towerTexture;
// =======================
// VARIÁVEIS GLOBAIS
// =======================

// Inimigos
Enemy enemies[MAX_ENEMIES];
int enemyCount;
float enemyLastAttackTime[MAX_ENEMIES];
int enemyTargetHero[MAX_ENEMIES];

// Recursos e jogo
recursos gameRecursos;

// NOVO: Status da Torre (para o Dragão)
bool is_tower_burning;
float tower_burn_timer;

// ⭐️ NOVO: Ataque da Torre
float tower_attack_timer = 0.0f;
const float TOWER_ATTACK_INTERVAL = 1.0f; // Torre ataca a cada 1 segundo
const float TOWER_ATTACK_RANGE = 150.0f; // Alcance de ataque da torre (pixels)
const int TOWER_ATTACK_DAMAGE = 30; // Dano por ataque

// Spawn
float spawnTimer;
static const float SPAWN_INTERVAL = 2.0f;

// Estado do jogo
GameState current_game_state;
int enemies_defeated_count;
int towerHealth;

// Menu
bool menuAberto;
Heroi herois[MAX_HEROIS];

// Heróis colocados
PlacedHero placedHeroes[MAX_HEROIS];
int placedHeroCount;
bool placementMode;
int selectedHeroType;

// ✨ NOVO: Sistema de Upgrades
HeroUpgrade heroUpgrades[MAX_HEROIS];
bool menuUpgradesAberto = false;
int selectedUpgradeHero = 0; // Índice do herói selecionado para upgrade

// 💰 NOVO: Sistema de Venda de Heróis
bool sellMode = false; // Modo de seleção para vender heróis
int selectedHeroToSell = -1; // Herói selecionado para venda
float sellMessageTimer = 0.0f; // Timer para mostrar mensagem de venda
char sellMessage[100] = ""; // Mensagem de feedback da venda

// ✨ NOVO: recompensa quando não há heróis
float noHeroTimer = 0.0f; // acumula tempo para dar 5 moedas por segundo quando o último herói morreu

// ✨ NOVO: flag para detectar morte (diferente de venda)
bool lastHeroDied = false;

// =======================
// VARIÁVEIS GLOBAIS - CAMINHOS
// =======================
Vector2 pathInferior[84];      // Caminho de baixo
Vector2 pathSuperior[96];       // Caminho de cima
bool useMultiplePaths = false;                  // Se deve usar múltiplos caminhos
int activePathCount = 1;                        // Número de caminhos ativos

// =======================
// VARIÁVEIS GLOBAIS - WAVES/FASES
// =======================
int currentWave;  
int total_enemies_spawned = 0;                              // Fase atual (1, 2, 3)
const int MAX_WAVES = 3;                       // Total de fases
const int WAVE_REWARD = 150;                   // Recompensa por wave

// =======================
// DEFINIÇÕES DE CAMINHOS
// =======================
#define NUM_WAYPOINTS_BAIXO 84  // TOTAL DE PONTOS para caminho inferior
#define NUM_WAYPOINTS_CIMA 96   // TOTAL DE PONTOS para caminho superior
#define MAX_PATHS 2             // Máximo de caminhos simultâneos

// =======================
// OUTRAS DEFINIÇÕES
// =======================
#define MAX_ENEMIES 20
#define ENEMY_ATTACK_RANGE 75.0f // Alcance de ataque dos inimigos (pixels)
#define ENEMY_ATTACK_INTERVAL 1.5f // Intervalo de ataque do inimigo (segundos)

void ResetGame();

// ⭐️ NOVO: Função para obter um tipo de inimigo com base na frequência
int GetRandomEnemyType() {
    int r = GetRandomValue(1, 100); // Gera um número de 1 a 100

    // Frequência de Spawn
    if (r <= 50) {          // 50% de chance
        return INIMIGO_GOBLIN;
    } else if (r <= 75) {   // 25% de chance
        return INIMIGO_SPECTRO;
    } else if (r <= 95) {   // 20% de chance
        return INIMIGO_NECROMANTE;
    } else {                // 5% de chance
        return INIMIGO_DRAGAO;
    }
}

void GetCurrentPaths(Vector2** paths, int* pathLengths, int* pathCount) {
    if (currentWave == 1) {
        // Fase 1: apenas caminho inferior
        paths[0] = pathInferior;
        pathLengths[0] = NUM_WAYPOINTS_BAIXO;
        *pathCount = 1;
    } else {
        // Fase 2 e 3: ambos os caminhos
        paths[0] = pathInferior;  // Caminho inferior
        paths[1] = pathSuperior;  // Caminho superior  
        pathLengths[0] = NUM_WAYPOINTS_BAIXO;
        pathLengths[1] = NUM_WAYPOINTS_CIMA;
        *pathCount = 2;
    }
}

void DrawPause(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Fundo escurecido
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));

    // Caixa central
    int boxW = 400;
    int boxH = 300;
    int boxX = screenWidth / 2 - boxW / 2;
    int boxY = screenHeight / 2 - boxH / 2;

    DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.15f, 10, DARKGRAY);
    DrawRectangleRoundedLines((Rectangle){boxX, boxY, boxW, boxH}, 0.15f, 10, WHITE);

    // Título
    DrawText("PAUSADO", boxX + 100, boxY + 20, 40, WHITE);

    // ---- Botões ----
    Rectangle btnContinuar = { boxX + 100, boxY + 90, 200, 45 };
    Rectangle btnReiniciar = { boxX + 100, boxY + 150, 200, 45 };
    Rectangle btnMenu = { boxX + 100, boxY + 210, 200, 45 };

    Vector2 mouse = GetMousePosition();

    // CONTINUAR
    DrawRectangleRec(btnContinuar, CheckCollisionPointRec(mouse, btnContinuar) ? GRAY : DARKGRAY);
    DrawText("Continuar", btnContinuar.x + 40, btnContinuar.y + 10, 25, WHITE);

    // REINICIAR
    DrawRectangleRec(btnReiniciar, CheckCollisionPointRec(mouse, btnReiniciar) ? GRAY : DARKGRAY);
    DrawText("Reiniciar Fase", btnReiniciar.x + 25, btnReiniciar.y + 10, 25, WHITE);

    // MENU PRINCIPAL
    DrawRectangleRec(btnMenu, CheckCollisionPointRec(mouse, btnMenu) ? GRAY : DARKGRAY);
    DrawText("Menu Principal", btnMenu.x + 20, btnMenu.y + 10, 25, WHITE);

    // ----- LÓGICA DOS BOTÕES -----
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

    // Resetar inimigos
    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    // Resetar heróis colocados
    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    // Reset do castelo
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    lastHeroDied = false;
    noHeroTimer = 0.0f;

    // Reset recursos
    gameRecursos.moedas = 100;

    // Fechar menus
    menuAberto = false;

    // Estado principal
    current_game_state = PLAYING;
}

void VoltarMenuPrincipal(void) {
    // Retorna ao menu principal
    current_game_state = MENU;

    // Limpa inimigos
    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    // Limpa heróis colocados
    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    // Reseta torre
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    lastHeroDied = false;
    noHeroTimer = 0.0f;

    // Reseta moedas
    gameRecursos.moedas = 100;

    // Fecha menu de compra (caso esteja aberto)
    menuAberto = false;
}


// CAMINHO (WAYPOINTS) - FASE 1
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


// Ponteiro para o caminho atual
Vector2 *currentPath = pathInferior;

// 💰 Inicializa os heróis disponíveis
void InicializarHerois(void) {
  // Heroi 1: Guerreiro (TIPO 0)
  strcpy(herois[0].nome, "Guerreiro");
  herois[0].custo = 50;
  herois[0].dano = 60;
  herois[0].alcance = 80;
  herois[0].texture = LoadTexture("resources/Cavaleiro.png");
  
  // Herói 2: Bardo (TIPO 1)
  strcpy(herois[1].nome, "Bardo");
  herois[1].custo = 100;
  herois[1].dano = 50;
  herois[1].alcance = 110;
  herois[1].texture = LoadTexture("resources/Bardo.png");
  
  // Herói 3: Paladino (TIPO 2)
  strcpy(herois[2].nome, "Paladino");
  herois[2].custo = 120;
  herois[2].dano = 65;
  herois[2].alcance = 120;
  herois[2].texture = LoadTexture("resources/Paladino.png");
  
  // Herói 4: Mago (TIPO 3)
  strcpy(herois[3].nome, "Mago");
  herois[3].custo = 180;
  herois[3].dano = 95;
  herois[3].alcance = 140;
  herois[3].texture = LoadTexture("resources/SapoMago.png");
  
  // ✨ NOVO: Inicializar upgrades para cada herói
  for (int i = 0; i < MAX_HEROIS; i++) {
    heroUpgrades[i].nivel_vida = 0;
    heroUpgrades[i].nivel_dano = 0;
    heroUpgrades[i].nivel_velocidade = 0;
    heroUpgrades[i].vida_total = 100;
    heroUpgrades[i].dano_total = herois[i].dano;
    heroUpgrades[i].velocidade_mult = 1.0f;
  }
}

// 💰 Função para comprar upgrade de herói
int ComprarUpgradeHeroi(recursos *r, int tipoHeroi, int tipoUpgrade) {
  if (tipoHeroi < 0 || tipoHeroi >= MAX_HEROIS) return 0;
  
  int custoupgrade = 0;
  int nivelAtual = 0;

  // Salva valores anteriores para aplicar diffs aos heróis já colocados
  int prev_vida_total = heroUpgrades[tipoHeroi].vida_total;
  int prev_dano_total = heroUpgrades[tipoHeroi].dano_total;
  float prev_vel_mult = heroUpgrades[tipoHeroi].velocidade_mult;
  
  // Definir custo e nível atual baseado no tipo de upgrade
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
  
  // Verificar moedas suficientes
  if (r->moedas < custoupgrade) return 0;
  
  // Aplicar upgrade (debita primeiro)
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

  // Aplica alterações para heróis já colocados do mesmo tipo
  // Calcula diferenças para ajustar HP e dano de forma coerente
  int new_vida_total = heroUpgrades[tipoHeroi].vida_total;
  int new_dano_total = heroUpgrades[tipoHeroi].dano_total;
  float new_vel_mult = heroUpgrades[tipoHeroi].velocidade_mult;

  int diffVida = new_vida_total - prev_vida_total;
  int diffDano = new_dano_total - prev_dano_total;
  float diffVel = new_vel_mult - prev_vel_mult;

  for (int i = 0; i < placedHeroCount; i++) {
      if (placedHeroes[i].tipo != tipoHeroi) continue;

      // Atualiza dano (torna o herói imediatamente mais forte)
      placedHeroes[i].dano = new_dano_total;

      // Aumenta a vida máxima/atual de forma justa: se estava vivo, aumenta proporcionalmente
      // Se estava abaixo do 'old max', soma o diff; se estava no max ou acima, ajusta para novo max.
      if (placedHeroes[i].health > 0) {
          placedHeroes[i].health += diffVida;
          if (placedHeroes[i].health > new_vida_total) placedHeroes[i].health = new_vida_total;
      }
      // Nota: velocidade afeta apenas o intervalo de ataque no loop de ataque (ver UpdateGame).
  }

  // Atualiza também a "base" do herói para futuras colocações
  // Assim, novos placedHeroes receberão dano atualizado
  herois[tipoHeroi].dano = new_dano_total;

  return 1;
}

// 💰 Função para desenhar a aba de upgrades
void DrawMenuUpgrades(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Fundo do menu
    DrawRectangle(40, 40, screenWidth - 80, screenHeight - 60, (Color){0, 0, 0, 220});
    DrawRectangleLines(40, 40, screenWidth - 80, screenHeight - 60, GOLD);

    // Título
    DrawText("UPGRADES DE HERÓIS", screenWidth/2 - MeasureText("UPGRADES DE HERÓIS", 30)/2, 60, 30, GOLD);
    DrawText("Pressione U para voltar",
             screenWidth/2 - MeasureText("Pressione U para voltar", 20)/2,
             100, 20, LIGHTGRAY);

    // Cards de seleção de herói
    int cardWidth = 140;
    int cardHeight = 180;
    int spacing = 15;
    int startX = 60;
    int startY = 140;

    // Desenhar seletores de herói
    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        Color cardColor = (i == selectedUpgradeHero) ? (Color){100, 150, 255, 255} : (Color){50, 50, 80, 255};
        DrawRectangle(cardX, cardY, cardWidth, cardHeight, cardColor);
        DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);

        // Nome do herói
        DrawText(herois[i].nome,
                 cardX + cardWidth/2 - MeasureText(herois[i].nome, 18)/2,
                 cardY + 15,
                 18, YELLOW);

        // Ícone ampliado
        int textureSize = 110;
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 25;
        DrawTexturePro(herois[i].texture,
                       (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                       (Rectangle){textureX, textureY, textureSize, textureSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    }

    // Área de upgrades do herói selecionado - EXPANDIDA PARA BAIXO
    int upgradeX = 60;
    int upgradeY = 360;
    int upgradeBoxWidth = screenWidth - 120;
    int upgradeBoxHeight = screenHeight - 440;

    DrawRectangle(upgradeX, upgradeY, upgradeBoxWidth, upgradeBoxHeight, (Color){50, 50, 80, 255});
    DrawRectangleLines(upgradeX, upgradeY, upgradeBoxWidth, upgradeBoxHeight, GOLD);

    // Verificar se nenhum herói foi selecionado
    if (selectedUpgradeHero < 0 || selectedUpgradeHero >= MAX_HEROIS) {
        // Mensagem pedindo para selecionar um herói
        const char *msg = "Clique em um heroi acima para escolher qual evoluir!";
        int msgWidth = MeasureText(msg, 20);
        DrawText(msg, 
                 upgradeX + (upgradeBoxWidth - msgWidth) / 2, 
                 upgradeY + (upgradeBoxHeight - 20) / 2, 
                 20, YELLOW);
        return;
    }

    // Título de upgrade
    char heroTitle[50];
    sprintf(heroTitle, "Upgrades: %s", herois[selectedUpgradeHero].nome);
    DrawText(heroTitle, upgradeX + 20, upgradeY + 15, 22, YELLOW);

    // Informações de moedas
    char moedaText[50];
    sprintf(moedaText, "Moedas: %d", get_moedas(&gameRecursos));
    DrawText(moedaText, upgradeX + upgradeBoxWidth - 200, upgradeY + 15, 20, GOLD);

    // Desenhar botões de upgrade (Vida, Dano, Velocidade) - CENTRALIZADOS
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

        // Informações do upgrade
        int nivelAtual = 0;
        int custoupgrade = 0;
        char infoText[100];

        if (i == 0) { // Vida
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_vida;
            custoupgrade = 50 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nVida: %d", nivelAtual, heroUpgrades[selectedUpgradeHero].vida_total);
        } else if (i == 1) { // Dano
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_dano;
            custoupgrade = 60 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nDano: %d", nivelAtual, heroUpgrades[selectedUpgradeHero].dano_total);
        } else if (i == 2) { // Velocidade
            nivelAtual = heroUpgrades[selectedUpgradeHero].nivel_velocidade;
            custoupgrade = 40 * (nivelAtual + 1);
            sprintf(infoText, "Nível: %d/5\nVel: %.2fx", nivelAtual, heroUpgrades[selectedUpgradeHero].velocidade_mult);
        }

        // Cor do botão
        Color btnColor = (nivelAtual >= 5) ? (Color){100, 100, 100, 255} : 
                         (gameRecursos.moedas >= custoupgrade) ? (Color){0, 150, 0, 255} : (Color){150, 0, 0, 255};

        DrawRectangle(btnX, btnY, btnWidth, btnHeight, btnColor);
        DrawRectangleLines(btnX, btnY, btnWidth, btnHeight, WHITE);

        // Nome do upgrade
        DrawText(nomeUpgrades[i],
                 btnX + btnWidth/2 - MeasureText(nomeUpgrades[i], 20)/2,
                 btnY + 10,
                 20, WHITE);

        // Informações - COM MELHOR ESPAÇAMENTO
        DrawText(infoText,
                 btnX + btnWidth/2 - MeasureText(infoText, 16)/2,
                 btnY + 40,
                 16, LIGHTGRAY);

        // Custo - MAIS ABAIXO
        char custoText[30];
        sprintf(custoText, "Custo: %d", custoupgrade);
        DrawText(custoText,
                 btnX + btnWidth/2 - MeasureText(custoText, 14)/2,
                 btnY + 80,
                 14, GOLD);
    }
}

// 💰 Função para verificar clique nos upgrades (apenas fase 2+)
void VerificarCliqueUpgrades(void) {
    if (currentWave < 2) return; // Não funciona na fase 1
    
    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (!mousePressed) return;

    // Verificar clique nos seletores de herói
    int cardWidth = 140;
    int spacing = 15;
    int startX = 60;
    int startY = 140;

    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        Rectangle cardRect = { cardX, cardY, cardWidth, 180 };
        if (CheckCollisionPointRec(mousePos, cardRect)) {
            selectedUpgradeHero = i;
            TraceLog(LOG_INFO, "Herói %d selecionado!", i);
            return;
        }
    }

    // Verificar clique nos botões de upgrade
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

// 💰 Função para comprar herói específico
int ComprarHeroiEspecifico(recursos *r, int tipoHeroi) {
  if (tipoHeroi >= 0 && tipoHeroi < MAX_HEROIS) {
        if (r->moedas >= herois[tipoHeroi].custo) {
            // Verifica se ainda cabe mais heróis antes de debitar moedas
            if (placedHeroCount < MAX_HEROIS) {
                r->moedas -= herois[tipoHeroi].custo;
                // Entrar no modo de colocação
                placementMode = true;
                selectedHeroType = tipoHeroi;
                menuAberto = false; // Fecha o menu
                TraceLog(LOG_INFO, "%s comprado! Clique no mapa para colocar. Moedas restantes: %d", herois[tipoHeroi].nome, r->moedas);
                return 1; // Compra realizada
            } else {
                TraceLog(LOG_WARNING, "Limite de heróis atingido! Não foi possível comprar %s.", herois[tipoHeroi].nome);
                return 0; // Não há espaço para colocar
            }
        }
  }
  return 0; // Moedas insuficientes ou tipo inválido
}

// 💰 Função para desenhar o menu de heróis
void DrawMenuHerois(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Fundo do menu (mais alto)
    DrawRectangle(40, 40, screenWidth - 80, screenHeight - 60, (Color){0, 0, 0, 220});
    DrawRectangleLines(40, 40, screenWidth - 80, screenHeight - 60, GOLD);

    // Título
    DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 60, 30, GOLD);
    DrawText("Pressione H para fechar",
             screenWidth/2 - MeasureText("Pressione H para fechar", 20)/2,
             100, 20, LIGHTGRAY);

    // Tamanho dos cards - AUMENTADO E MELHOR ESPAÇADO
    int cardWidth  = 160;
    int cardHeight = 320;

    int spacing = 30;
    int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
    int startY = 180;

    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        // Card
        DrawRectangle(cardX, cardY, cardWidth, cardHeight, (Color){50, 50, 80, 255});
        DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);

        // Nome centralizado
        DrawText(herois[i].nome,
                 cardX + cardWidth/2 - MeasureText(herois[i].nome, 20)/2,
                 cardY + 10,
                 20, YELLOW);

        // Ícone centralizado - REDIMENSIONADO
        int textureSize = 85;
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 45;
        DrawTexturePro(herois[i].texture,
                       (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                       (Rectangle){textureX, textureY, textureSize, textureSize},
                       (Vector2){0, 0}, 0.0f, WHITE);

        // Estatísticas centralizadas - MELHOR ESPAÇAMENTO
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

        // Botão - MELHOR POSICIONADO
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

    // Rodapé
    DrawText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões",
             screenWidth/2 - MeasureText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 16)/2,
             startY + cardHeight + 50,
             16, LIGHTGRAY);
    
    // ✨ NOVO: Botão de aba de Upgrades - REDUZIDO (apenas fase 2+)
    if (currentWave >= 2) {
        Rectangle btnUpgrades = { screenWidth - 150, 55, 120, 35 };
        Color btnUpgradesColor = CheckCollisionPointRec(GetMousePosition(), btnUpgrades) ? YELLOW : GOLD;
        DrawRectangleRec(btnUpgrades, btnUpgradesColor);
        DrawRectangleLines((int)btnUpgrades.x, (int)btnUpgrades.y, (int)btnUpgrades.width, (int)btnUpgrades.height, WHITE);
        DrawText("U - UPGRADES", btnUpgrades.x + 10, btnUpgrades.y + 8, 14, BLACK);
        
        // Verificar clique no botão de upgrades
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btnUpgrades)) {
            // Abre o menu de upgrades e garante que o menu de compra seja fechado.
            menuUpgradesAberto = true;
            menuAberto = false;
            selectedUpgradeHero = -1; // Força o jogador a escolher um herói
            return;
        }
    }
}

// 💰 Função para verificar clique nos botões do menu
void VerificarCliqueMenu(void) {
    Vector2 mousePos = GetMousePosition();
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    int screenWidth = GetScreenWidth();

    // Use os mesmos parâmetros gráficos de DrawMenuHerois
    int cardWidth = 160;
    int cardHeight = 320;
    int spacing = 30;
    int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
    int startY = 180;

    if (!mouseReleased) return;

    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        // Botão conforme desenhado em DrawMenuHerois (com hitbox expandida para maior tolerância)
        int btnWidth = cardWidth - 20;
        int btnHeight = 40;
        int btnX = cardX + (cardWidth - btnWidth) / 2;
        int btnY = cardY + cardHeight - btnHeight - 10;

        // Expandir hitbox em ~15 pixels de cada lado para maior tolerância
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

// ✨ ADIÇÃO 2: Função para iniciar a Fase 2 (transição)
void IniciarFase2(void) {
    TraceLog(LOG_INFO, "Iniciando Fase 2...");
    
    // 1. Dar recompensa ao jogador
    gameRecursos.moedas += WAVE_REWARD;
    
    // 2. Atualizar fase atual
    currentWave = 2;
    
    // 3. Troca a textura de fundo.
    background = backgroundFase2; 
    
    // 4. Resetar o estado do jogo para a próxima onda
    enemyCount = 0;
    enemies_defeated_count = 0;
    total_enemies_spawned = 0; 
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    current_game_state = PLAYING; 

    placedHeroCount = 0;
    
    // Resetar status dos heróis
    for (int i = 0; i < MAX_HEROIS; i++) {
        placedHeroes[i].health = 0;
        placedHeroes[i].is_burning = false;
        placedHeroes[i].burn_timer = 0.0f;
    }

    // 5. Resetar variáveis de spawn e inimigos
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
    
    // 1. Dar recompensa ao jogador
    gameRecursos.moedas += WAVE_REWARD;
    
    // 2. Atualizar fase atual
    currentWave = 3;
    
    // 3. Troca a textura de fundo (adicione uma textura para fase 3)
    // background = backgroundFase3; // Se tiver textura diferente
    
    // 4. Resetar o estado do jogo para a próxima onda
    enemyCount = 0;
    enemies_defeated_count = 0; 
    total_enemies_spawned = 0;
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;
    current_game_state = PLAYING; 

    placedHeroCount = 0;

    background = backgroundFase3;
    
    // Resetar status dos heróis
    for (int i = 0; i < MAX_HEROIS; i++) {
        placedHeroes[i].health = 0;
        placedHeroes[i].is_burning = false;
        placedHeroes[i].burn_timer = 0.0f;
    }

    // 5. Resetar variáveis de spawn e inimigos
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
// Inicialização
void InitGame(void) {

    currentWave = 1;
    // Em src/game.c, dentro de InitGame(void)

// ... (outras inicializações)
    total_enemies_spawned = 0; // Garante que o contador começa do zero
// ...
    // ======================
    // TEXTURAS
    // ======================
    background = LoadTexture("resources/background_novo.jpg");
    towerTexture = LoadTexture("resources/tower.png");
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");
    backgroundFase3 = LoadTexture("resources/backgroundFase3.jpg");

    // ======================
    // RECURSOS E HERÓIS
    // ======================
    inicializar_recursos(&gameRecursos);
    InicializarHerois();

    // ======================
    // ESTADOS INICIAIS
    // ======================
    current_game_state = PLAYING;
    towerHealth = CASTLE_MAX_HEALTH;
    enemies_defeated_count = 0;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;

    // ======================
    // INIMIGOS
    // ======================
    enemyCount = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].pathIndex = 0; // 
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    // ======================
    // HERÓIS COLOCADOS
    // ======================
    placedHeroCount = 0;
    placementMode = false;
    selectedHeroType = -1;

    // ======================
    // MENU DE COMPRA
    // ======================
    menuAberto = false;
    
    // Inicializa o gerador de números aleatórios para o spawn
    SetRandomSeed(GetTime()); 
}


// Atualização
void UpdateGame(void) {
    float dt = GetFrameTime();

    if (currentWave == 1) {
        // Garante que TODOS os inimigos ativos estão no caminho inferior
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].active) {
                enemies[i].pathIndex = 0;
            }
        }
    }
    // =========================================================
    // 🔹 1. ABRIR / FECHAR PAUSE COM A TECLA P
    // =========================================================
    if (IsKeyPressed(KEY_P)) {
        if (current_game_state == PLAYING) {
            current_game_state = PAUSED;
        }
        else if (current_game_state == PAUSED) {
            current_game_state = PLAYING;
        }
    }

    // =========================================================
    // 🔹 2. ESTADO PAUSADO — (REINICIAR / MENU / MENU DE HERÓIS)
    // =========================================================

    if (current_game_state == PAUSED) {
        // Lógica de botões dentro de DrawPause ou VerificarCliqueMenu
        if (menuAberto) {
            VerificarCliqueMenu();
            return;
        }
        return;
    }

    // =========================================================
    // 🔹 ABRIR / FECHAR MENU DE HERÓIS (funciona SEMPRE)
    // =========================================================
    if (IsKeyPressed(KEY_H)) {
        menuAberto = !menuAberto;  // alterna entre abrir e fechar
        menuUpgradesAberto = false; // Fecha upgrades se abrir menu de compra
        return;
    }

    // ✨ NOVO: ABRIR / FECHAR MENU DE UPGRADES (apenas fase 2+)
    if (IsKeyPressed(KEY_U) && currentWave >= 2) {
        menuUpgradesAberto = !menuUpgradesAberto;
        menuAberto = false; // Fecha menu de compra se abrir upgrades
        sellMode = false; // Desativa modo de venda
        if (menuUpgradesAberto) {
            selectedUpgradeHero = -1; // Força o jogador a escolher um herói
            TraceLog(LOG_INFO, "Menu de upgrades aberto!");
        }
        return;
    }

    // 💰 NOVO: ATIVAR / DESATIVAR MODO DE VENDA DE HERÓIS
    if (IsKeyPressed(KEY_V)) {
        sellMode = !sellMode;
        menuAberto = false; // Fecha menu de compra
        menuUpgradesAberto = false; // Fecha menu de upgrades
        selectedHeroToSell = -1;
        TraceLog(LOG_INFO, "Modo de venda: %s", sellMode ? "ATIVADO" : "DESATIVADO");
        return;
    }

    // =========================================================
    // 🔹 3. TELA DE VITÓRIA / GAME OVER
    // =========================================================
    if (current_game_state == WAVE_WON || current_game_state == GAME_OVER) {
        // A lógica de clique é tratada na função DrawGame
        return;
    }


    // =========================================================
    // 🔹 4. MENU DE COMPRA DE HERÓIS ABERTO
    // =========================================================
    if (menuAberto) {
        VerificarCliqueMenu();
        return;
    }

    // ✨ NOVO: 4B. MENU DE UPGRADES ABERTO
    if (menuUpgradesAberto) {
        VerificarCliqueUpgrades();
        return;
    }

    // 💰 NOVO: MODO DE VENDA DE HERÓIS ATIVADO
        if (sellMode) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                bool heroVendido = false;
                
                // Verificar clique em algum herói
                for (int i = 0; i < placedHeroCount; i++) {
                    if (placedHeroes[i].health <= 0) continue;
                    
                    float dx = mousePos.x - placedHeroes[i].x;
                    float dy = mousePos.y - placedHeroes[i].y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    
                    // Se clicou em um herói com vida cheia, vender
                    if (dist <= 50 && placedHeroes[i].health == 100) {
                        int sell_price = (herois[placedHeroes[i].tipo].custo * 70) / 100;
                        adicionar_moedas(&gameRecursos, sell_price);
                        
                        sprintf(sellMessage, "Vendido! +%d moedas", sell_price);
                        sellMessageTimer = 2.0f;
                        
                        TraceLog(LOG_INFO, "Herói %s vendido por %d moedas!", herois[placedHeroes[i].tipo].nome, sell_price);
                        
                        // Remover herói vendido do array deslocando elems posteriores
                        for (int j = i; j < placedHeroCount - 1; j++) {
                            placedHeroes[j] = placedHeroes[j + 1];
                        }
                        placedHeroCount--;
                        
                        heroVendido = true;
                        // Impede recompensa por "ausência" se o jogador vendeu o herói
                        lastHeroDied = false;
                        break;
                    }
                    // Se clicou em um herói danificado, mostrar mensagem (sem fazer nada)
                    else if (dist <= 50 && placedHeroes[i].health < 100) {
                        sprintf(sellMessage, "Herói danificado! Recupere a vida antes de vender");
                        sellMessageTimer = 2.0f;
                        
                        TraceLog(LOG_WARNING, "Este herói está danificado! Recupere a vida antes de vender.");
                        heroVendido = true;
                        break;
                    }
                }
                
                // Se vendeu um herói, desativar modo de venda
                if (heroVendido) {
                    selectedHeroToSell = -1;
                    sellMode = false;
                }
            }
            
            // Cancelar modo de venda com ESC
            if (IsKeyPressed(KEY_ESCAPE)) {
                sellMode = false;
                selectedHeroToSell = -1;
            }
            return;
        }


    // =========================================================
    // 🔹 5. MODO DE POSICIONAMENTO DE HERÓI
    // =========================================================
    if (placementMode) {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            Vector2 mousePos = GetMousePosition();

            int heroSize = 80;
            int halfSize = heroSize / 2;
            int x = (int)mousePos.x;
            int y = (int)mousePos.y;

            // Clamp position to keep hero fully on screen
            if (x - halfSize < 0) x = halfSize;
            if (y - halfSize < 0) y = halfSize;
            if (x + halfSize > GetScreenWidth()) x = GetScreenWidth() - halfSize;
            if (y + halfSize > GetScreenHeight()) y = GetScreenHeight() - halfSize;

            // Preenche a struct PlacedHero com as estatísticas do herói
            placedHeroes[placedHeroCount].x = x;
            placedHeroes[placedHeroCount].y = y;
            placedHeroes[placedHeroCount].tipo = selectedHeroType;
            // Usa valores já modificados pelos upgrades (vida e dano)
            placedHeroes[placedHeroCount].dano = heroUpgrades[selectedHeroType].dano_total;
            placedHeroes[placedHeroCount].alcance = herois[selectedHeroType].alcance;
            placedHeroes[placedHeroCount].health = heroUpgrades[selectedHeroType].vida_total; // HP base do herói (considera upgrades)
             placedHeroes[placedHeroCount].lastAttackTime = 0;
             placedHeroes[placedHeroCount].texture = herois[selectedHeroType].texture;
              // Inicializa status de queima do herói
              placedHeroes[placedHeroCount].is_burning = false;
              placedHeroes[placedHeroCount].burn_timer = 0.0f;

            // Colocou um herói vivo — cancela cenário de "último herói morreu"
            lastHeroDied = false;
            
            placedHeroCount++;

            placementMode = false;
            selectedHeroType = -1;
        }

        return;
    }


    // =========================================================
    // 🔹 6. LÓGICA NORMAL DO JOGO (somente PLAYING)
    // =========================================================
    if (current_game_state != PLAYING) return;

    // --- RECOMPENSA: +5 MOEDAS/SEGUNDO SE NÃO HOUVER HERÓIS COLOCADOS ---
    if (lastHeroDied) {
        // conta quantos heróis VIVOS restam
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
    // --- FIM RECOMPENSA ---

    // --- ATAQUE DOS HERÓIS (COM RESISTÊNCIA) ---
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
                // Intervalo base 1s, modificado pelo multiplicador de velocidade dos upgrades
                float velMult = heroUpgrades[placedHeroes[i].tipo].velocidade_mult;
                if (velMult <= 0.0f) velMult = 1.0f;
                float attackInterval = 1.0f / velMult;
 
                if (placedHeroes[i].lastAttackTime >= attackInterval) {
                     
                     // ✅ CORRETO: Calcula a máscara de bit do herói
                     int hero_bit = (1 << placedHeroes[i].tipo);
                     
                     // ✅ CORRETO: Verifica se o inimigo NÃO tem resistência contra este herói
                     if ((enemies[targetEnemy].resistance & hero_bit) == 0) {
                         // ⭐️ O herói PODE ATINGIR o inimigo (NÃO tem resistência)
                        enemies[targetEnemy].health -= placedHeroes[i].dano;
                        
                        if (enemies[targetEnemy].health <= 0) {
                            adicionar_moedas(&gameRecursos, enemies[targetEnemy].recompensa_moedas);
                            enemies[targetEnemy].active = 0;
                            enemies_defeated_count++;
                        }
                     } else {
                         // ⭐️ O herói NÃO pode atingir (tem resistência)
                         TraceLog(LOG_WARNING, "Inimigo %d é resistente ao Herói %s!", targetEnemy, herois[placedHeroes[i].tipo].nome);
                     }
 
                    placedHeroes[i].lastAttackTime = 0;
                 }
             }
             // ⭐️⭐️⭐️ FIM DA SUBSTITUIÇÃO ⭐️⭐️⭐️
         }

        // --- SPAWN DE INIMIGOS ---
        // ⚠️ NOVO: Adiciona a verificação total_enemies_spawned < WAVE_SIZE[currentWave]
        if (towerHealth > 0 && enemyCount < MAX_ENEMIES && total_enemies_spawned < WAVE_SIZE[currentWave]) { 
            spawnTimer += dt;
            if (spawnTimer >= SPAWN_INTERVAL) {
                int activeEnemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) activeEnemies++;
                }
                
                if (activeEnemies < MAX_ENEMIES) {
                    EnemyType newEnemyType = GetRandomEnemyType(); 
                    
                    // ⭐️⭐️⭐️ LÓGICA DE CAMINHO (Já está Correta para as Fases)
                    float startX, startY;
                    int pathIndex = 0;
                    
                    if (currentWave == 1) {
                        // FASE 1: 100% caminho inferior (pathIndex = 0)
                        startX = pathInferior[0].x;
                        startY = pathInferior[0].y;
                        pathIndex = 0;
                    } else {
                        // FASE 2+: 50% inferior, 50% superior (pathIndex 0 ou 1)
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
                    
                    // 💰 NOVO: Incrementa a contagem de inimigos spawnados
                    total_enemies_spawned++; 
                    
                    enemies[enemyCount] = InitEnemy(startX, startY, newEnemyType);
                    enemies[enemyCount].pathIndex = pathIndex;
                    enemyLastAttackTime[enemyCount] = 0;
                    enemyTargetHero[enemyCount] = -1;
                    enemyCount++;
                }
                spawnTimer = 0;
            }
        }
    // --- LÓGICA DE HABILIDADES ESPECIAIS (NECROMANTE E DRAGÃO) ---
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        // ⭐️ NOVO: Configurar resistências especiais para alguns inimigos
        if (enemies[i].type == INIMIGO_SPECTRO) {
            // Fantasma: só pode ser atacado pelo Mago (tipo 3)
            // Resistência a Guerreiro(0), Bardo(1), Paladino(2)
            enemies[i].resistance = 0x07; // bits 0, 1, 2 = tipos 0, 1, 2
        } 
        else if (enemies[i].type == INIMIGO_NECROMANTE) {
            // Necromante: só pode ser atacado pelo Paladino (tipo 2)
            // Resistência a Guerreiro(0), Bardo(1), Mago(3)
            enemies[i].resistance = 0x0D; // bits 0, 1, 3 = tipos 0, 1, 3
        }
        
        // 1. Cura do Necromante
        if (enemies[i].type == INIMIGO_NECROMANTE) {
            enemies[i].necromante_heal_timer -= dt;

            if (enemies[i].necromante_heal_timer <= 0) {
                // Cura: 15% da vida máxima do inimigo
                int heal_amount = (int)(enemies[i].maxHealth * 0.15f);
                
                // Aplica a cura em inimigos no alcance
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
                enemies[i].necromante_heal_timer = 5.0f; // Reseta o timer para 5 segundos
            }
        }
    }

        // =========================================================
        // 🔹 7. LÓGICA DE TRANSIÇÃO DE ONDA / VITÓRIA (INSERIR AQUI) ⬅️
        // =========================================================

        if (currentWave < MAX_WAVES) {
            // Verifica se todos os inimigos previstos para a onda foram derrotados.
            if (enemies_defeated_count >= WAVE_SIZE[currentWave]) {

                // Verifica se não há inimigos ativos restantes no mapa
                int activeEnemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        activeEnemies++;
                        break; 
                    }
                }
                
                // Se todos foram derrotados E não há mais ninguém no mapa
                if (activeEnemies == 0) {
                    current_game_state = WAVE_WON; 
                }
            }
        } else {
            // Lógica de Vitória Final (se for a última onda)
            if (enemies_defeated_count >= WAVE_SIZE[currentWave]) {
                // ... (Verificação final e transição para GAME_OVER (Vitória))
            }
        }


    // --- MOVIMENTO & ATAQUE DOS INIMIGOS ---
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;

        // ⭐️ VERIFICA SE CHEGOU NA TORRE - AGORA A TORRE ATACA O INIMIGO
        if (EnemyReachedTower(enemies[i])) {
            // Inimigo chegou na torre, sofre dano da torre
            // Não morre instantaneamente mais, a torre vai atacar
            if (enemies[i].type == INIMIGO_DRAGAO) {
                is_tower_burning = true;
                tower_burn_timer = 5.0f;
            }
            // Torre será atacada se o inimigo ficar perto dela
            // continue para permitir que o inimigo continue vivo perto da torre
        }

        int targetHero = enemyTargetHero[i];

        // ⭐️ CORREÇÃO: TODOS os inimigos buscam heróis
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
            // Verifica se ainda está no alcance
            float dx = enemies[i].x - placedHeroes[targetHero].x;
            float dy = enemies[i].y - placedHeroes[targetHero].y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist <= enemies[i].range) {
                enemyLastAttackTime[i] += dt;

                if (enemyLastAttackTime[i] >= ENEMY_ATTACK_INTERVAL) {
                    placedHeroes[targetHero].health -= enemies[i].damage;

                    // Lógica de Queima (Dragão)
                    if (enemies[i].type == INIMIGO_DRAGAO) {
                        placedHeroes[targetHero].is_burning = true;
                        placedHeroes[targetHero].burn_timer = 5.0f;
                    }

                    if (placedHeroes[targetHero].health <= 0) {
                        placedHeroes[targetHero].health = 0;
                        lastHeroDied = true; // marcou que um herói morreu (poderá ser o último)
                        enemyTargetHero[i] = -1;
                    }

                    enemyLastAttackTime[i] = 0;
                }
            } else {
                // Herói saiu do alcance
                enemyTargetHero[i] = -1;
                UpdateEnemy(&enemies[i]);
            }
        } else {
            // Nenhum herói no alcance, move para a torre
            UpdateEnemy(&enemies[i]);
        }
    }

    // --- ATAQUE DA TORRE CONTRA INIMIGOS ---
    tower_attack_timer += dt;
    if (tower_attack_timer >= TOWER_ATTACK_INTERVAL) {
        tower_attack_timer = 0.0f;
        
        // Posição aproximada da torre (centro)
        Vector2 towerPos = {650 + 40, 100 + 40};
        
        // Torre ataca inimigos no alcance
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].active) continue;
            
            float dx = enemies[i].x - towerPos.x;
            float dy = enemies[i].y - towerPos.y;
            float dist = sqrtf(dx*dx + dy*dy);
            
            // Se inimigo está no alcance da torre, sofre dano
            if (dist <= TOWER_ATTACK_RANGE) {
                enemies[i].health -= TOWER_ATTACK_DAMAGE;
                
                TraceLog(LOG_DEBUG, "Torre atacou inimigo! Dano: %d, Vida restante: %d", TOWER_ATTACK_DAMAGE, enemies[i].health);
                
                // Se inimigo morre pelo ataque da torre
                if (enemies[i].health <= 0) {
                    enemies[i].active = 0;
                    enemies[i].health = 0;
                    TraceLog(LOG_INFO, "Inimigo foi destruído pela torre!");
                }
            }
        }
    }
    
    // --- INIMIGOS ATACAM A TORRE (CONTRA-ATAQUE) ---
    Vector2 towerPos = {650 + 40, 100 + 40};
    static float tower_enemy_attack_timer = 0.0f;
    
    tower_enemy_attack_timer += dt;
    
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        float dx = enemies[i].x - towerPos.x;
        float dy = enemies[i].y - towerPos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        
        // Se inimigo chegou muito perto da torre (alcance de ataque do inimigo)
        if (dist <= enemies[i].range && EnemyReachedTower(enemies[i])) {
            // Inimigo está atacando a torre
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


    // --- PROCESSAMENTO DE STATUS (QUEIMA) ---

    // Dano de Queima na Torre
    if (is_tower_burning) {
        tower_burn_timer -= dt;
        
        // Perde 10% da vida máxima da torre por segundo
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
    
    // Dano de Queima nos Heróis
    for (int i = 0; i < placedHeroCount; i++) {
        if (placedHeroes[i].health > 0 && placedHeroes[i].is_burning) {
            placedHeroes[i].burn_timer -= dt;
            
            // ⭐️ AUMENTADO: Perde 20 pontos de vida por segundo (maior que antes)
            placedHeroes[i].health -= (int)(40 * dt); 
            
            if (placedHeroes[i].health <= 0) {
                placedHeroes[i].health = 0;
                placedHeroes[i].is_burning = false;
                lastHeroDied = true; // morreu por queima
            }
            
            if (placedHeroes[i].burn_timer <= 0.0f) {
                placedHeroes[i].is_burning = false;
            }
        }
    }


    // =========================================================
    // 🔹 7. CHECAR VITÓRIA
    // =========================================================
    if (enemyCount >= MAX_ENEMIES &&
        enemies_defeated_count >= MAX_ENEMIES) {

        current_game_state = WAVE_WON;
    }
}

// 🔹 Função para desenhar UI normal
void DrawGameUI(void) {
  // Fundo semi-transparente com bordas
  DrawRectangle(10, 10, 220, 150, Fade(BLACK, 0.7f));
  DrawRectangleLines(10, 10, 220, 150, Fade(GOLD, 0.6f));
  
  // Torre HP (no topo esquerdo)
  DrawText("TORRE", 20, 17, 12, GOLD);
  DrawText(TextFormat("HP: %d", towerHealth), 20, 32, 18, RED);
 
  // Indicador de Queima na Torre
  if (is_tower_burning) {
    DrawText("QUEIMANDO!", 20, 52, 11, ORANGE);
  }
  
  // 💰 Moedas (logo embaixo do HP da Torre)
  DrawText("MOEDAS:", 20, 67, 12, GOLD);
  DrawText(TextFormat("%d", get_moedas(&gameRecursos)), 20, 82, 18, YELLOW);
  
  // Separador
  DrawLine(10, 105, 230, 105, Fade(GOLD, 0.4f));
  
  // Instruções para abrir menu (estilizadas)
  DrawText("[H] Loja", 20, 112, 13, Fade(GOLD, 0.8f));
  DrawText("[U] Upgrades", 20, 128, 13, Fade(GOLD, 0.8f));
  DrawText("[V] Vender", 20, 144, 13, Fade(GOLD, 0.8f));

  // Indicação de modo de colocação
  if (placementMode) {
    int textWidth = MeasureText("Clique no mapa para colocar o heroi", 18);
    DrawRectangle(GetScreenWidth()/2 - textWidth/2 - 10, 15, textWidth + 20, 35, Fade(YELLOW, 0.2f));
    DrawRectangleLines(GetScreenWidth()/2 - textWidth/2 - 10, 15, textWidth + 20, 35, YELLOW);
    DrawText("Clique no mapa para colocar o heroi", GetScreenWidth()/2 - textWidth/2, 22, 18, YELLOW);
  }
}

// Desenho
void DrawGame(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    // 🔹 Fundo
    DrawTexturePro(
        background,
        (Rectangle){ 0, 0, background.width, background.height }, 
        (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, 
        (Vector2){ 0, 0 }, 
        0.0f, 
        WHITE 
    );

    // =======================================================
    // ➤ TELA DE PAUSA
    // =======================================================
    if (current_game_state == PAUSED) {

        DrawPause();

        // 🔹 Se o menu de heróis estiver aberto mesmo em PAUSED, desenha ele
        if (menuAberto) {
            DrawMenuHerois();
        }

        EndDrawing();
        return;   // Para aqui!
    }

    // SUBSTITUA a parte dos caminhos por:
    /* Desenha caminho inferior
    for (int i = 0; i < 83; i++) {
        DrawLineEx(pathInferior[i], pathInferior[i + 1], 3.0f, YELLOW);
    }
    */

    /*Se fase 2+, desenha caminho superior também
    if (currentWave > 1) {
        for (int i = 0; i < 95; i++) {
            DrawLineEx(pathSuperior[i], pathSuperior[i + 1], 3.0f, GREEN);
        }
    }
    */
    
    // 🔹 Torre
    DrawTexture(towerTexture, 650, 100, WHITE);
    
    // 💰 UI Normal
    DrawGameUI();

    // 🔹 Inimigos e Heróis (Só desenha se o jogo estiver rodando)
    if (current_game_state == PLAYING) {

        // Inimigos
        for (int i = 0; i < enemyCount; i++) {
            DrawEnemy(enemies[i]);
        }

        // Heróis
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

            // Desenhar textura do herói
            DrawTexturePro(placedHeroes[i].texture,
                (Rectangle){0, 0, placedHeroes[i].texture.width, placedHeroes[i].texture.height},
                (Rectangle){placedHeroes[i].x - 40, placedHeroes[i].y - 40, 80, 80},
                (Vector2){40, 40}, 0.0f, WHITE);

            DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});

            // Indicador de Queima no Herói
            if (placedHeroes[i].is_burning) {
                DrawCircle(placedHeroes[i].x + 20, placedHeroes[i].y - 20, 5, ORANGE);
            }

            // Barra de vida

            int barWidth = 40;
            int barHeight = 5;
            int barX = placedHeroes[i].x - barWidth / 2;
            int barY = placedHeroes[i].y - 50;

            DrawRectangle(barX, barY, barWidth, barHeight, RED);
            DrawRectangle(barX, barY, (int)(barWidth * (placedHeroes[i].health / 100.0f)), barHeight, GREEN);
        }
    }

    // 💰 Menu de heróis (quando o jogo NÃO está pausado)
    if (menuAberto) {
        DrawMenuHerois();
    }
    
    // ✨ NOVO: Menu de Upgrades (apenas fase 2+)
    if (menuUpgradesAberto && currentWave >= 2) {
        DrawMenuUpgrades();
    }
    
    // 💰 NOVO: Modo de venda - Destacar heróis disponíveis para venda
    if (sellMode) {
        // Efeito de pulsação (animação)
        float pulse = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
        
        for (int i = 0; i < placedHeroCount; i++) {
            if (placedHeroes[i].health <= 0) continue;
            
            if (placedHeroes[i].health == 100) {
                // Heróis disponíveis para venda
                float circleRadius = 55 + (5 * pulse); // Pulsação do círculo
                
                // Círculos concêntricos com efeito de glow
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius + 5, Fade(GOLD, 0.3f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius + 2, Fade(GOLD, 0.6f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, circleRadius, GOLD);
                
                // Preenchimento semi-transparente
                DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 45, Fade(GOLD, 0.15f));
                
                // Ícone de moeda no herói
                DrawText("$", placedHeroes[i].x - 5, placedHeroes[i].y - 65, 20, GOLD);
                
                // Preço de venda acima do herói
                int sell_price = (herois[placedHeroes[i].tipo].custo * 70) / 100;
                char priceText[20];
                sprintf(priceText, "+%d", sell_price);
                int textWidth = MeasureText(priceText, 16);
                DrawText(priceText, placedHeroes[i].x - textWidth/2, placedHeroes[i].y - 85, 16, GOLD);
            } else {
                // Heróis NÃO disponíveis (vida baixa)
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, 55, Fade(DARKGRAY, 0.4f));
                DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, 58, Fade(DARKGRAY, 0.3f));
                
                // Símbolo de proibição
                DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 45, Fade(DARKGRAY, 0.1f));
                DrawText("X", placedHeroes[i].x - 5, placedHeroes[i].y - 8, 20, Fade(RED, 0.5f));
            }
        }
        
        // Painel de instruções melhorado
        int panelWidth = 480;
        int panelHeight = 100;
        int panelX = GetScreenWidth()/2 - panelWidth/2;
        int panelY = 40;
        
        // Fundo com gradiente
        DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(BLACK, 0.8f));
        DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GOLD);
        DrawRectangleLines(panelX+2, panelY+2, panelWidth-4, panelHeight-4, Fade(GOLD, 0.5f));
        
        // Título
        const char *titleSell = "MODO VENDA ATIVADO";
        int titleWidth = MeasureText(titleSell, 22);
        DrawText(titleSell, panelX + panelWidth/2 - titleWidth/2, panelY + 10, 22, GOLD);
        
        // Instrução principal
        const char *instrSell = "Clique em um heroi com VIDA CHEIA para vender";
        int instrWidth = MeasureText(instrSell, 14);
        DrawText(instrSell, panelX + panelWidth/2 - instrWidth/2, panelY + 38, 14, WHITE);
        
        // Instrução de cancelamento
        const char *cancelSell = "ESC = Cancelar | Verde = Vendável | Cinza = Danificado";
        int cancelWidth = MeasureText(cancelSell, 12);
        DrawText(cancelSell, panelX + panelWidth/2 - cancelWidth/2, panelY + 58, 12, LIGHTGRAY);
    }
    
    // =======================================================
    // ➤ TELA DE DERROTA
    // =======================================================
    if (current_game_state == GAME_OVER) {
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

    }

    // =======================================================
    // ➤ TELA DE VITÓRIA (CORRIGIDA PARA FASES)
    // =======================================================
    else if (current_game_state == WAVE_WON) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGREEN, 0.8f));
        
        const char *message;
        const char *button_text;
        
        // ⭐️ Lógica de Mensagem e Botão
        if (currentWave == 1) {
            message = "ONDA 1 COMPLETA! IR PARA A FASE 2";
            button_text = "INICIAR FASE 2";
        } else if (currentWave == 2) {
            message = "ONDA 2 COMPLETA! IR PARA A FASE 3"; // ⬅️ MENSAGEM PARA FASE 3
            button_text = "INICIAR FASE 3";
        } else {
            message = "PARABÉNS! VOCÊ VENCEU O JOGO!";
            button_text = "VOLTAR AO MENU";
        }
        
        // Desenha Mensagem
        int message_len = MeasureText(message, 40);
        DrawText(message, (GetScreenWidth() / 2) - (message_len / 2),
                 GetScreenHeight() / 2 - 50, 40, GOLD);
        
        // Botão
        Rectangle button_rect = { (GetScreenWidth() / 2) - 100,
                                 GetScreenHeight() / 2 + 50,
                                 200, 50 };
        Color buttonColor = CheckCollisionPointRec(GetMousePosition(), button_rect) ? GOLD : YELLOW;
        
        DrawRectangleRec(button_rect, buttonColor);
        DrawText(button_text, button_rect.x + (strcmp(button_text, "VOLTAR AO MENU") == 0 ? 5 : 20), button_rect.y + 15, 20, BLACK);
        
        // ⭐️ Lógica de Clique
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_rect)) {
            if (currentWave == 1) {
                  IniciarFase2(); // Chama Fase 2
            } else if (currentWave == 2) {
                  IniciarFase3(); // ⬅️ CHAMADA PARA FASE 3
            } else {
                  current_game_state = MENU; // Se for a última fase
            }
        }
    }

    EndDrawing();
    
}


void ResetGame(void)
{
    // Resetar vida da torre
    towerHealth = CASTLE_MAX_HEALTH;
    is_tower_burning = false;
    tower_burn_timer = 0.0f;

    // Resetar inimigos
    enemyCount = 0;
    enemies_defeated_count = 0;
    spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemyLastAttackTime[i] = 0;
        enemyTargetHero[i] = -1;
    }

    // Resetar heróis colocados
    placedHeroCount = 0;

    for (int i = 0; i < MAX_HEROIS; i++) {
        placedHeroes[i].health = 0;
        placedHeroes[i].x = 0;
        placedHeroes[i].y = 0;
        placedHeroes[i].tipo = -1;
        placedHeroes[i].is_burning = false;
        placedHeroes[i].burn_timer = 0.0f;
    }

    // Resetar recursos
    inicializar_recursos(&gameRecursos);

    // Fechar menus e modos
    menuAberto = false;
    placementMode = false;
    selectedHeroType = -1;
    lastHeroDied = false;
    noHeroTimer = 0.0f;
 
    // Voltar ao gameplay
    current_game_state = PLAYING;
}

// Finalização
void CloseGame(void) {
    UnloadTexture(background);
    UnloadTexture(backgroundFase2);
    
  UnloadTexture(towerTexture);
  
  // 💰 Descarrega texturas dos heróis
  for (int i = 0; i < MAX_HEROIS; i++) {
    UnloadTexture(herois[i].texture);
  }
}
