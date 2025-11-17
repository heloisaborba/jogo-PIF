// game.c

#include "raylib.h"
#include "raymath.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include "hero.h"
#include "tower.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int currentWave;              // Fase atual (1, 2, 3)
const int MAX_WAVES = 3;      // Total de fases
const int WAVE_REWARD = 150;

// ==============================
// TEXTURAS GLOBAIS DO JOGO
// ==============================
static Texture2D background;         // textura da fase 1
static Texture2D backgroundFase2;    // textura da fase 2
static Texture2D towerTexture;       // textura da torre/castelo

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

// ⭐️ NOVO: Status da Torre (para o Dragão)
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

void ResetGame();

#define MAX_ENEMIES 20

#define NUM_WAYPOINTS 84 // TOTAL DE PONTOS
// #define ENEMY_DAMAGE_TO_CASTLE 20 // REMOVIDO: Agora o dano vem da struct Enemy
// #define ENEMY_DAMAGE_TO_HERO 5 // REMOVIDO: Agora o dano vem da struct Enemy
#define ENEMY_ATTACK_RANGE 75.0f // Alcance de ataque dos inimigos (pixels)
#define ENEMY_ATTACK_INTERVAL 1.5f // Intervalo de ataque do inimigo (segundos)

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


// CAMINHO (WAYPOINTS)
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

// 💰 Inicializa os heróis disponíveis
void InicializarHerois(void) {
  // Heroi 1: Guerreiro (TIPO 0)
  strcpy(herois[0].nome, "Guerreiro");
  herois[0].custo = 50;
  herois[0].dano = 40;
  herois[0].alcance = 150;
  herois[0].texture = LoadTexture("resources/Cavaleiro.png");
  
  // Herói 2: Bardo (TIPO 1)
  strcpy(herois[1].nome, "Bardo");
  herois[1].custo = 100;
  herois[1].dano = 60;
  herois[1].alcance = 300;
  herois[1].texture = LoadTexture("resources/Bardo.png");
  
  // Herói 3: Paladino (TIPO 2)
  strcpy(herois[2].nome, "Paladino");
  herois[2].custo = 200;
  herois[2].dano = 55;
  herois[2].alcance = 200;
  herois[2].texture = LoadTexture("resources/Paladino.png");
  
  // Herói 4: Mago (TIPO 3)
  strcpy(herois[3].nome, "Mago");
  herois[3].custo = 150;
  herois[3].dano = 100;
  herois[3].alcance = 250;
  herois[3].texture = LoadTexture("resources/SapoMago.png");
}

// 💰 Função para comprar herói específico
int ComprarHeroiEspecifico(recursos *r, int tipoHeroi) {
  if (tipoHeroi >= 0 && tipoHeroi < MAX_HEROIS) {
    if (r->moedas >= herois[tipoHeroi].custo) {
      r->moedas -= herois[tipoHeroi].custo;
      // Entrar no modo de colocação
      if (placedHeroCount < MAX_HEROIS) {
        placementMode = true;
        selectedHeroType = tipoHeroi;
        menuAberto = false; // Fecha o menu
        TraceLog(LOG_INFO, "%s comprado! Clique no mapa para colocar. Moedas restantes: %d", herois[tipoHeroi].nome, r->moedas);
      } else {
        TraceLog(LOG_WARNING, "Limite de heróis atingido! Não foi possível comprar %s.", herois[tipoHeroi].nome);
      }
      return 1; // Compra realizada
    }
  }
  return 0; // Moedas insuficientes ou tipo inválido
}

// 💰 Função para desenhar o menu de heróis
void DrawMenuHerois(void) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  
  // Fundo semi-transparente para o menu
  DrawRectangle(40, 90, screenWidth - 80, screenHeight - 180, (Color){0, 0, 0, 220});
  
  // Borda do menu
  DrawRectangleLines(40, 90, screenWidth - 80, screenHeight - 180, GOLD);
  
  // Título do menu
  DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 110, 30, GOLD);
  DrawText("Pressione H para fechar", screenWidth/2 - MeasureText("Pressione M para fechar", 20)/2, 150, 20, LIGHTGRAY);
  
  // Desenha os cards dos heróis
  int cardWidth = 185; 
  int cardHeight = 245; 
  int spacing = 20;   
  int startX = (screenWidth - (MAX_HEROIS * cardWidth + (MAX_HEROIS - 1) * spacing)) / 2;
  int startY = 190;
  
  for (int i = 0; i < MAX_HEROIS; i++) {
    int cardX = startX + i * (cardWidth + spacing);
    int cardY = startY;
    
    // Card background
    Color cardColor = (Color){50, 50, 80, 255};
    DrawRectangle(cardX, cardY, cardWidth, cardHeight, cardColor);
    DrawRectangleLines(cardX, cardY, cardWidth, cardHeight, LIGHTGRAY);
    
    // Nome do herói
    DrawText(herois[i].nome, cardX + cardWidth/2 - MeasureText(herois[i].nome, 20)/2, cardY + 20, 20, YELLOW);
    
    // Ícone/textura do herói
    int textureSize = 95; 
    int textureX = cardX + (cardWidth - textureSize) / 2;
    int textureY = cardY + 50;
    DrawTexturePro(herois[i].texture,
           (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
           (Rectangle){textureX, textureY, textureSize, textureSize},
           (Vector2){0, 0}, 0.0f, WHITE);
    
    // Estatísticas
    DrawText(TextFormat("Custo: %d$", herois[i].custo), cardX + 20, cardY + 160, 17, GOLD);
    DrawText(TextFormat("Dano: %d", herois[i].dano), cardX + 20, cardY + 180, 17, RED);
    DrawText(TextFormat("Alcance: %d", herois[i].alcance), cardX + 20, cardY + 200, 17, BLUE);
    
    // Botão de compra
    Color btnColor = (gameRecursos.moedas >= herois[i].custo) ? GREEN : RED;
    DrawRectangle(cardX + 20, cardY + cardHeight - 40, cardWidth - 40, 30, btnColor);
    DrawText("COMPRAR", cardX + cardWidth/2 - MeasureText("COMPRAR", 17)/2, cardY + cardHeight - 35, 17, WHITE);
    
    // Número da tecla para comprar rápido
    DrawText(TextFormat("[%d]", i + 1), cardX + cardWidth - 25, cardY + cardHeight - 35, 17, YELLOW);
  }
  
  // Instruções no rodapé
  DrawText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 
       screenWidth/2 - MeasureText("Use 1, 2, 3, 4 para comprar rapidamente ou clique nos botões", 17)/2, 
       startY + cardHeight + 25, 17, LIGHTGRAY);
}

// 💰 Função para verificar clique nos botões do menu
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
      
      // Verifica clique no botão de compra (coordenadas atualizadas)
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
// Inicialização
void InitGame(void) {

    // ======================
    // TEXTURAS
    // ======================
    background = LoadTexture("resources/background_novo.jpg");
    towerTexture = LoadTexture("resources/tower.png");
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");

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

            // Preenche a struct PlacedHero com as estatísticas do herói
            placedHeroes[placedHeroCount].x = mousePos.x;
            placedHeroes[placedHeroCount].y = mousePos.y;
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
        // --- SPAWN DE INIMIGOS (COM PROBABILIDADE) ---
        if (towerHealth > 0 && enemyCount < MAX_ENEMIES) {
            spawnTimer += dt;
            if (spawnTimer >= SPAWN_INTERVAL) {
                // ⭐️ VERIFICA SE AINDA PODE SPAWNAR
                int activeEnemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) activeEnemies++;
                }
                
                if (activeEnemies < MAX_ENEMIES) {
                    EnemyType newEnemyType = GetRandomEnemyType(); 
                    enemies[enemyCount] = InitEnemy(path[0].x, path[0].y, newEnemyType);
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

    // 🎯 DEBUG: Desenha o caminho dos inimigos
    for (int i = 0; i < MAX_WAYPOINTS - 1; i++) {
        DrawLineEx(path[i], path[i + 1], 3.0f, (Color){255, 255, 0, 128});
    }
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        DrawCircle(path[i].x, path[i].y, 5.0f, BLUE);
        DrawText(TextFormat("%d", i), path[i].x + 10, path[i].y - 10, 10, WHITE);
    }

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

            DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 20, heroColor);
            DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});

            // Indicador de Queima no Herói
            if (placedHeroes[i].is_burning) {
                DrawCircle(placedHeroes[i].x + 15, placedHeroes[i].y - 15, 5, ORANGE);
            }

            // Barra de vida
            int barWidth = 40;
            int barHeight = 5;
            int barX = placedHeroes[i].x - barWidth / 2;
            int barY = placedHeroes[i].y - 30;

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
    // ➤ TELA DE VITÓRIA
    // =======================================================
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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), button_rect)) {
             IniciarFase2();
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
