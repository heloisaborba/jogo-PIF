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
};;


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

    // Tamanho dos cards
    int cardWidth  = 170;
    int cardHeight = 300;   // Novo tamanho maior

    int spacing = 20;
    int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
    int startY = 160;

    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        // Card
        DrawRectangle(cardX, cardY, cardWidth, cardHeight, (Color){50, 50, 80, 255});
        DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);

        // Nome centralizado
        DrawText(herois[i].nome,
                 cardX + cardWidth/2 - MeasureText(herois[i].nome, 22)/2,
                 cardY + 15,
                 22, YELLOW);

        // Ícone centralizado
        int textureSize = 95;
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 60;
        DrawTexturePro(herois[i].texture,
                       (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                       (Rectangle){textureX, textureY, textureSize, textureSize},
                       (Vector2){0, 0}, 0.0f, WHITE);

        // Estatísticas centralizadas
        int statsStartY = cardY + 170;  // ajustado para centralizar melhor

        char custoTxt[32];
        char danoTxt[32];
        char alcanceTxt[32];

        sprintf(custoTxt,   "Custo: %d$", herois[i].custo);
        sprintf(danoTxt,    "Dano: %d",   herois[i].dano);
        sprintf(alcanceTxt, "Alcance: %d",herois[i].alcance);

        DrawText(custoTxt,
                 cardX + cardWidth/2 - MeasureText(custoTxt, 18)/2,
                 statsStartY,
                 18, GOLD);

        DrawText(danoTxt,
                 cardX + cardWidth/2 - MeasureText(danoTxt, 18)/2,
                 statsStartY + 25,
                 18, RED);

        DrawText(alcanceTxt,
                 cardX + cardWidth/2 - MeasureText(alcanceTxt, 18)/2,
                 statsStartY + 50,
                 18, BLUE);

        // Botão
        Color btnColor = (gameRecursos.moedas >= herois[i].custo) ? GREEN : RED;

        int btnWidth  = cardWidth - 40;
        int btnHeight = 35;
        int btnX = cardX + (cardWidth - btnWidth) / 2;
        int btnY = cardY + cardHeight - btnHeight - 15;

        DrawRectangle(btnX, btnY, btnWidth, btnHeight, btnColor);

        DrawText("COMPRAR",
                 btnX + btnWidth/2 - MeasureText("COMPRAR", 18)/2,
                 btnY + 5,
                 18, WHITE);
    }

    // Rodapé
    DrawText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões",
             screenWidth/2 - MeasureText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 18)/2,
             startY + cardHeight + 40,
             18, LIGHTGRAY);
}

// 💰 Função para verificar clique nos botões do menu
void VerificarCliqueMenu(void) {
    Vector2 mousePos = GetMousePosition();
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    int screenWidth = GetScreenWidth();

    // Use os mesmos parâmetros gráficos de DrawMenuHerois
    int cardWidth = 170;
    int cardHeight = 300;
    int spacing = 20;
    int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
    int startY = 160;

    if (!mouseReleased) return;

    for (int i = 0; i < MAX_HEROIS; i++) {
        int cardX = startX + i * (cardWidth + spacing);
        int cardY = startY;

        // Botão conforme desenhado em DrawMenuHerois (com hitbox expandida para maior tolerância)
        int btnWidth = cardWidth - 40;
        int btnHeight = 35;
        int btnX = cardX + (cardWidth - btnWidth) / 2; // = cardX + 20
        int btnY = cardY + cardHeight - btnHeight - 15; // alinhado com DrawMenuHerois

        // Expandir hitbox em ~15 pixels de cada lado para maior tolerância de clique
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
            placedHeroes[placedHeroCount].dano = herois[selectedHeroType].dano;
            placedHeroes[placedHeroCount].alcance = herois[selectedHeroType].alcance;
            placedHeroes[placedHeroCount].health = 100; // HP base do herói
            placedHeroes[placedHeroCount].lastAttackTime = 0;
            placedHeroes[placedHeroCount].texture = herois[selectedHeroType].texture;
             // Inicializa status de queima do herói
             placedHeroes[placedHeroCount].is_burning = false;
             placedHeroes[placedHeroCount].burn_timer = 0.0f;


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
                if (placedHeroes[i].lastAttackTime >= 1.0f) {
                    
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

        // ⭐️ VERIFICA SE CHEGOU NA TORRE ANTES DE QUALQUER COISA
        if (EnemyReachedTower(enemies[i])) {
            towerHealth -= enemies[i].damage; 
            enemies[i].active = 0;
            
            if (enemies[i].type == INIMIGO_DRAGAO) {
                is_tower_burning = true;
                tower_burn_timer = 5.0f;
            }

            if (towerHealth <= 0) {
                towerHealth = 0;
                current_game_state = GAME_OVER;
            }
            continue;
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
    // --- PROCESSAMENTO DE STATUS (QUEIMA) ---

    // Dano de Queima na Torre
    if (is_tower_burning) {
        tower_burn_timer -= dt;
        
        // Perde 10% da vida máxima da torre por segundo
        towerHealth -= (int)(CASTLE_MAX_HEALTH * 0.10f * dt); 
        
        if (towerHealth <= 0) {
            towerHealth = 0;
            is_tower_burning = false;
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
            
            // Perde 10% da vida máxima do herói (assumida como 100) por segundo
            placedHeroes[i].health -= (int)(100 * 0.10f * dt); 
            
            if (placedHeroes[i].health <= 0) {
                placedHeroes[i].health = 0;
                placedHeroes[i].is_burning = false;
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
  // Fundo semi-transparente para as informações
  DrawRectangle(10, 10, 280, 110, (Color){0, 0, 0, 128});
  
  // Torre HP
  DrawText(TextFormat("Torre HP: %d", towerHealth), 20, 20, 20, RED);
 
 // Indicador de Queima na Torre
 if (is_tower_burning) {
    DrawText("QUEIMANDO!", 20, 45, 15, ORANGE);
 }
  
  // 💰 Moedas
  DrawText(TextFormat("Moedas: %d", get_moedas(&gameRecursos)), 20, 70, 20, GOLD);
  
  // Instruções para abrir menu
  DrawText("H - Abrir loja de herois", 20, 100, 15, LIGHTGRAY);

  // Indicação de modo de colocação
  if (placementMode) {
    DrawText("Clique no mapa para colocar o herói", GetScreenWidth()/2 - MeasureText("Clique no mapa para colocar o herói", 20)/2, 20, 20, YELLOW);
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
