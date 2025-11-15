// game.c

#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_ENEMIES 5

#define NUM_WAYPOINTS 84 // TOTAL DE PONTOS

Vector2 path[NUM_WAYPOINTS] = {
    // ... Pontos 0 a 82 (sem alteração) ...
    { 50, 565 },  { 65, 560 },  { 80, 555 },  { 95, 550 },  { 110, 545 },
    { 125, 540 }, { 140, 535 }, { 155, 530 }, { 170, 525 }, { 185, 520 }, 
    { 200, 515 }, { 215, 510 }, { 230, 505 }, { 245, 500 }, { 270, 495 }, 
    { 300, 488 }, { 330, 480 }, { 360, 473 }, { 390, 465 }, { 420, 458 }, 
    { 430, 435 }, { 450, 435 }, { 470, 435 }, { 490, 435 }, { 490, 420 }, 
    { 490, 405 }, { 490, 390 }, { 480, 385 }, { 465, 380 }, { 450, 375 }, 
    { 435, 370 }, { 420, 365 }, { 405, 360 }, { 390, 360 }, { 375, 366 }, 
    { 360, 372 }, { 345, 378 }, { 330, 384 }, { 315, 390 }, { 300, 396 }, 
    { 285, 402 }, { 270, 408 }, { 250, 420 }, { 230, 435 }, 
    { 215, 427 }, { 200, 419 }, { 185, 411 }, { 170, 403 }, 
    { 155, 395 }, { 140, 387 }, // Ponto 49
    { 140, 372 }, // Ponto 50 
    { 155, 364 }, { 170, 356 }, { 185, 348 }, { 200, 340 }, 
    { 215, 332 }, { 230, 324 }, { 245, 316 }, // Ponto 57 
    { 260, 316 }, { 245, 308 }, { 230, 300 }, { 215, 292 }, { 200, 284 }, 
    { 185, 276 }, // Ponto 63 
    { 205, 271 }, { 225, 266 }, { 245, 261 }, { 265, 256 }, 
    { 280, 249 }, { 295, 242 }, { 310, 235 }, // Ponto 70
    { 325, 228 }, { 340, 221 }, { 360, 231 }, { 380, 241 }, { 400, 251 }, 
    { 420, 261 }, { 440, 271 }, 
    { 460, 251 }, // Ponto 78 
    { 497, 239 }, // Ponto 79 
    { 534, 227 }, // Ponto 80 
    { 571, 215 }, // Ponto 81 
    { 608, 203 }, // Ponto 82 
    
    // CORREÇÃO APLICADA: Ponto 83 sobe 15 pixels.
    { 645, 176 }, // Ponto 83 (Y: 191 - 15 = 176)

    
};

static Enemy enemies[MAX_ENEMIES];
static int enemyCount = 0;
static Texture2D background;
static Texture2D towerTexture;
static int towerHealth = 100;
static recursos gameRecursos;
static float spawnTimer = 0.0f;
static const float SPAWN_INTERVAL = 2.0f; // Spawn a new enemy every 2 seconds

// 💰 Variáveis do sistema de menu
static bool menuAberto = false;
static Heroi herois[MAX_HEROIS];

// Heróis colocados no mapa
static PlacedHero placedHeroes[MAX_HEROIS];
static int placedHeroCount = 0;
static bool placementMode = false;
static int selectedHeroType = -1; // Tipo do herói a ser colocado

// 💰 Inicializa os heróis disponíveis
void InicializarHerois(void) {
    // Herói 1: Guerreiro
    strcpy(herois[0].nome, "Guerreiro");
    herois[0].custo = 50;
    herois[0].dano = 10;
    herois[0].alcance = 150;
    //herois[0].texture = LoadTexture("resources/perfil_guerreiro.png");
    
    // Herói 2: Bardo
    strcpy(herois[1].nome, "Bardo");
    herois[1].custo = 100;
    herois[1].dano = 20;
    herois[1].alcance = 300;
    //herois[1].texture = LoadTexture("resources/perfil_bardo.png");
    
    // Herói 3: Paladino
    strcpy(herois[2].nome, "Paladino");
    herois[2].custo = 200;
    herois[2].dano = 15;
    herois[2].alcance = 200;
    //herois[2].texture = LoadTexture("resources/perfil_paladino.png");
    
    // Herói 4: Mago
    strcpy(herois[3].nome, "Mago");
    herois[3].custo = 150;
    herois[3].dano = 25;
    herois[3].alcance = 250;
    //herois[3].texture = LoadTexture("resources/perfil_mago.png");
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
// 💰 Função para desenhar o menu de heróis
// 💰 Função para desenhar o menu de heróis
void DrawMenuHerois(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Fundo semi-transparente para o menu (mais largo para 4 heróis)
    DrawRectangle(40, 90, screenWidth - 80, screenHeight - 180, (Color){0, 0, 0, 220});
    
    // Borda do menu
    DrawRectangleLines(40, 90, screenWidth - 80, screenHeight - 180, GOLD);
    
    // Título do menu
    DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 110, 30, GOLD);
    DrawText("Pressione M para fechar", screenWidth/2 - MeasureText("Pressione M para fechar", 20)/2, 150, 20, LIGHTGRAY);
    
    // Desenha os cards dos heróis - UM POUQUINHO MAIOR
    int cardWidth = 185;  // Aumentado de 170 para 185
    int cardHeight = 245; // Aumentado de 230 para 245
    int spacing = 20;     // Reduzido um pouco o espaçamento
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
        
        // Ícone/textura do herói (um pouquinho maior)
        int textureSize = 95; // Aumentado de 90 para 95
        int textureX = cardX + (cardWidth - textureSize) / 2;
        int textureY = cardY + 50;
        DrawTexturePro(herois[i].texture,
                      (Rectangle){0, 0, herois[i].texture.width, herois[i].texture.height},
                      (Rectangle){textureX, textureY, textureSize, textureSize},
                      (Vector2){0, 0}, 0.0f, WHITE);
        
        // Estatísticas (texto um pouquinho maior)
        DrawText(TextFormat("Custo: %d$", herois[i].custo), cardX + 20, cardY + 160, 17, GOLD);
        DrawText(TextFormat("Dano: %d", herois[i].dano), cardX + 20, cardY + 180, 17, RED);
        DrawText(TextFormat("Alcance: %d", herois[i].alcance), cardX + 20, cardY + 200, 17, BLUE);
        
        // Botão de compra (um pouquinho maior)
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

// Inicialização
void InitGame(void) {
    // Carrega as texturas
    background = LoadTexture("resources/background_novo.jpg");
    towerTexture = LoadTexture("resources/tower.png");

    inicializar_recursos(&gameRecursos);
    InicializarHerois(); // 💰 Inicializa o sistema de heróis
    
    towerHealth = 100;
    enemyCount = 1;

    for (int i = 0; i < enemyCount; i++) {
        enemies[i] = InitEnemy((int)path[0].x, (int)path[0].y); 
    }
}

// Atualização
void UpdateGame(void) {
    // 💰 Alterna o menu com a tecla M
    if (IsKeyPressed(KEY_M)) {
        menuAberto = !menuAberto;
    }
    
    // Se o menu estiver aberto, processa apenas inputs do menu
    if (menuAberto) {
        VerificarCliqueMenu();

        // 💰 Compras rápidas com teclas numéricas
        if (IsKeyPressed(KEY_ONE)) {
            if (ComprarHeroiEspecifico(&gameRecursos, 0)) {
                TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[0].nome, gameRecursos.moedas);
            }
        }
        if (IsKeyPressed(KEY_TWO)) {
            if (ComprarHeroiEspecifico(&gameRecursos, 1)) {
                TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[1].nome, gameRecursos.moedas);
            }
        }
        if (IsKeyPressed(KEY_THREE)) {
            if (ComprarHeroiEspecifico(&gameRecursos, 2)) {
                TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[2].nome, gameRecursos.moedas);
            }
        }
        if (IsKeyPressed(KEY_FOUR)) {
            if (ComprarHeroiEspecifico(&gameRecursos, 3)) {
                TraceLog(LOG_INFO, "%s comprado! Moedas restantes: %d", herois[3].nome, gameRecursos.moedas);
            }
        }

        return; // Não atualiza o jogo enquanto o menu está aberto
    }

    // Se estiver no modo de colocação, processa clique no mapa
    if (placementMode) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            // Coloca o herói na posição clicada
            placedHeroes[placedHeroCount].x = (int)mousePos.x;
            placedHeroes[placedHeroCount].y = (int)mousePos.y;
            placedHeroes[placedHeroCount].tipo = selectedHeroType;
            placedHeroes[placedHeroCount].dano = herois[selectedHeroType].dano;
            placedHeroes[placedHeroCount].alcance = herois[selectedHeroType].alcance;
            placedHeroes[placedHeroCount].lastAttackTime = 0.0f;
            placedHeroes[placedHeroCount].texture = herois[selectedHeroType].texture;
            placedHeroCount++;
            TraceLog(LOG_INFO, "%s colocado no mapa!", herois[selectedHeroType].nome);
            // Sai do modo de colocação
            placementMode = false;
            selectedHeroType = -1;
        }
        return; // Não atualiza o jogo enquanto no modo de colocação
    }
    


    // Atualizar heróis colocados
    for (int i = 0; i < placedHeroCount; i++) {
        // Encontrar inimigo mais próximo no alcance
        int targetEnemy = -1;
        float minDist = placedHeroes[i].alcance;
        for (int j = 0; j < enemyCount; j++) {
            if (enemies[j].active) {
                float dx = placedHeroes[i].x - enemies[j].x;
                float dy = placedHeroes[i].y - enemies[j].y;
                float dist = sqrt(dx*dx + dy*dy);
                if (dist < minDist) {
                    minDist = dist;
                    targetEnemy = j;
                }
            }
        }

        // Atacar inimigo se encontrado
        if (targetEnemy != -1) {
            placedHeroes[i].lastAttackTime += GetFrameTime();
            if (placedHeroes[i].lastAttackTime >= 1.0f) { // Ataque a cada 1 segundo
                enemies[targetEnemy].health -= placedHeroes[i].dano;
                if (enemies[targetEnemy].health <= 0) {
                    enemies[targetEnemy].active = 0;
                }
                placedHeroes[i].lastAttackTime = 0.0f;
            }
        }
    }

    // Spawn new enemies if tower is alive and not at max enemies
    if (towerHealth > 0 && enemyCount < MAX_ENEMIES) {
        spawnTimer += GetFrameTime();
        if (spawnTimer >= SPAWN_INTERVAL) {
            enemies[enemyCount] = InitEnemy((int)path[0].x, (int)path[0].y);
            enemyCount++;
            spawnTimer = 0.0f;
        }
    }

    for (int i = 0; i < enemyCount; i++) {
        UpdateEnemy(&enemies[i]);

        if (EnemyReachedTower(enemies[i]) && enemies[i].active) {
            towerHealth -= 1;
            enemies[i].active = 0;
        }
    }

    if (towerHealth <= 0) {
        towerHealth = 0;
    }
}

// 🔹 Função para desenhar UI normal
void DrawGameUI(void) {
    // Fundo semi-transparente para as informações
    DrawRectangle(10, 10, 280, 90, (Color){0, 0, 0, 128});
    
    // Torre HP
    DrawText(TextFormat("Torre HP: %d", towerHealth), 20, 20, 20, RED);
    
    // 💰 Moedas
    DrawText(TextFormat("Moedas: %d", get_moedas(&gameRecursos)), 20, 50, 20, GOLD);
    
    // Instruções para abrir menu
    DrawText("M - Abrir loja de herois", 20, 80, 15, LIGHTGRAY);

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

    // 🔹 Inimigos
    for (int i = 0; i < enemyCount; i++) {
        DrawEnemy(enemies[i]);
    }

    // Desenhar heróis colocados
    for (int i = 0; i < placedHeroCount; i++) {
        Color heroColor;
        switch (placedHeroes[i].tipo) {
            case 0: heroColor = BLUE; break;   // Guerreiro
            case 1: heroColor = GREEN; break;  // Bardo
            case 2: heroColor = YELLOW; break; // Paladino
            case 3: heroColor = PURPLE; break; // Mago
            default: heroColor = WHITE; break;
        }
        DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 20, heroColor);
        // Desenhar alcance (círculo semi-transparente)
        DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});
    }



    // 💰 Desenha o menu se estiver aberto
    if (menuAberto) {
        DrawMenuHerois();
    }

    EndDrawing();
}

// Finalização
void CloseGame(void) {
    UnloadTexture(background);
    UnloadTexture(towerTexture);
    
    // 💰 Descarrega texturas dos heróis
    for (int i = 0; i < MAX_HEROIS; i++) {
        UnloadTexture(herois[i].texture);
    }
}