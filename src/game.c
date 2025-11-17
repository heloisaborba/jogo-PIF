// game.c

#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_ENEMIES 20

#define NUM_WAYPOINTS 84 // TOTAL DE PONTOS
#define ENEMY_DAMAGE_TO_CASTLE 20 // Dano de 20 por inimigo na torre

// ⭐️ NOVAS CONSTANTES PARA O COMBATE INIMIGO VS HERoI
#define ENEMY_ATTACK_RANGE 75.0f // Alcance de ataque dos inimigos (pixels)
#define ENEMY_DAMAGE_TO_HERO 5 // Dano que o inimigo causa a um heroi
#define ENEMY_ATTACK_INTERVAL 1.5f // Intervalo de ataque do inimigo (segundos)

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
// ⭐️ VARIÁVEIS DE ESTADO DO INIMIGO (SIMULANDO CAMPOS EM ENEMY STRUCT)
// Usamos arrays paralelos, pois não temos acesso direto à struct Enemy
static float enemyLastAttackTime[MAX_ENEMIES] = {0.0f}; // Tempo desde o último ataque
static int enemyTargetHero[MAX_ENEMIES] = {-1}; // Índice do herói alvo (-1 se não estiver atacando)


static Enemy enemies[MAX_ENEMIES];
static int enemyCount = 0;
static Texture2D background;
static Texture2D towerTexture;
// ✨ ADIÇÃO 1: Variável para a textura da Fase 2
static Texture2D backgroundFase2;
static recursos gameRecursos;
static float spawnTimer = 0.0f;
static const float SPAWN_INTERVAL = 2.0f; // Spawn a new enemy every 2 seconds


// Variáveis GLOBAIS de Estado (Removidas as duplicações)
GameState current_game_state = PLAYING; 
int enemies_defeated_count = 0; 
int towerHealth = CASTLE_MAX_HEALTH; // Usa a constante definida em game.h

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
  // Heroi 1: Guerreiro
  strcpy(herois[0].nome, "Guerreiro");
  herois[0].custo = 50;
  herois[0].dano = 10;
  herois[0].alcance = 150;
    // ✨ Descomentar carregamento (Se não estiver assim, ele não funciona)
    herois[0].texture = LoadTexture("resources/Cavaleiro.png");
  
  // Herói 2: Bardo
  strcpy(herois[1].nome, "Bardo");
  herois[1].custo = 100;
  herois[1].dano = 20;
  herois[1].alcance = 300;
    // ✨ Descomentar carregamento
    herois[1].texture = LoadTexture("resources/Bardo.png");
  
  // Herói 3: Paladino
  strcpy(herois[2].nome, "Paladino");
  herois[2].custo = 200;
  herois[2].dano = 15;
  herois[2].alcance = 200;
    // ✨ Descomentar carregamento
    herois[2].texture = LoadTexture("resources/Paladino.png");
  
  // Herói 4: Mago
  strcpy(herois[3].nome, "Mago");
  herois[3].custo = 150;
  herois[3].dano = 25;
  herois[3].alcance = 250;
    // ✨ Descomentar carregamento
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
  
  // Fundo semi-transparente para o menu (mais largo para 4 heróis)
  DrawRectangle(40, 90, screenWidth - 80, screenHeight - 180, (Color){0, 0, 0, 220});
  
  // Borda do menu
  DrawRectangleLines(40, 90, screenWidth - 80, screenHeight - 180, GOLD);
  
  // Título do menu
  DrawText("LOJA DE HERÓIS", screenWidth/2 - MeasureText("LOJA DE HERÓIS", 30)/2, 110, 30, GOLD);
  DrawText("Pressione M para fechar", screenWidth/2 - MeasureText("Pressione M para fechar", 20)/2, 150, 20, LIGHTGRAY);
  
  // Desenha os cards dos heróis - UM POUQUINHO MAIOR
  int cardWidth = 185; // Aumentado de 170 para 185
  int cardHeight = 245; // Aumentado de 230 para 245
  int spacing = 20;   // Reduzido um pouco o espaçamento
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

// ✨ ADIÇÃO 2: Função para iniciar a Fase 2 (transição)
void IniciarFase2(void) {
    TraceLog(LOG_INFO, "Iniciando Fase 2...");
    
    // 1. Troca a textura de fundo. Agora 'background' aponta para a textura da Fase 2.
    background = backgroundFase2; 
    
    // 2. Resetar o estado do jogo para a próxima onda
    enemyCount = 0;
    enemies_defeated_count = 0; 
    towerHealth = CASTLE_MAX_HEALTH; // Restaura a vida da torre
    current_game_state = PLAYING; // Retorna ao estado de jogo

    placedHeroCount = 0;

    // 3. Resetar variáveis de spawn e inimigos
    spawnTimer = 0.0f; 
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemyLastAttackTime[i] = 0.0f;
        enemyTargetHero[i] = -1;
    }
    
    // Aqui você também poderia redefinir o array 'path' se o caminho da Fase 2 fosse diferente.
    // Ex: path = novo_caminho_fase2;
}

// Inicialização
void InitGame(void) {
  // Carrega as texturas
  background = LoadTexture("resources/background_novo.jpg");
  towerTexture = LoadTexture("resources/tower.png");
    // ✨ ADIÇÃO 3: Carregar a nova textura da Fase 2
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");

  inicializar_recursos(&gameRecursos);
  InicializarHerois(); // 💰 Inicializa o sistema de heróis
  
  towerHealth = CASTLE_MAX_HEALTH; // NOVO: Usa a constante de vida máxima
  current_game_state = PLAYING; // NOVO: Inicia no estado PLAYING
  enemies_defeated_count = 0; // NOVO: Zera o contador de vitória
  enemyCount = 0; // Zera a contagem para iniciar o spawn

  // Limpa os tempos de ataque dos inimigos
  for(int i = 0; i < MAX_ENEMIES; i++) {
    enemyLastAttackTime[i] = 0.0f;
    enemyTargetHero[i] = -1;
  }
}

// Atualização
void UpdateGame(void) {
  // 💰 Alterna o menu com a tecla M
  if (IsKeyPressed(KEY_M)) {
    menuAberto = !menuAberto;
  }

  // NOVO: Se o jogo terminou ou está na tela de vitória/derrota, bloqueia a maioria das ações
  if (current_game_state != PLAYING) {
    
        // ✨ ADIÇÃO 4: Lógica de clique do botão "CONTINUAR" na tela de vitória
        if (current_game_state == WAVE_WON && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int screenWidth = GetScreenWidth();
            // Coordenadas do botão (DEVE ser as mesmas usadas em DrawGame)
            Rectangle button_rect = { (screenWidth / 2) - 100, GetScreenHeight() / 2 + 50, 200, 50 };
            
            if (CheckCollisionPointRec(GetMousePosition(), button_rect)) {
                IniciarFase2(); // Chama a função que inicia a Fase 2 e troca o fundo
                return; // Sai da atualização para evitar processar o frame no estado antigo
            }
        }
        
    // Se menu estiver aberto, permite fechar (e processa compra se for rápida)
    if (menuAberto) {
      VerificarCliqueMenu();
      // ... (Lógica de Compras rápidas) ...
    }
    
    return; // Sai se o jogo não está em andamento
  }
  
  // Se o menu estiver aberto, processa apenas inputs do menu
  if (menuAberto) {
    VerificarCliqueMenu();
    // ... (Lógica de Compras rápidas) ...
    return; // Não atualiza o jogo enquanto o menu está aberto
  }

// ... (Resto da função UpdateGame inalterado: colocação de heróis, ataque de heróis, spawn de inimigos, movimento/ataque de inimigos) ...
    
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
      placedHeroes[placedHeroCount].health = 100; // Vida inicial dos heróis
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
  
  
  // 1. Atualizar heróis colocados (Lógica de Ataque do Herói)
  for (int i = 0; i < placedHeroCount; i++) {
    if (placedHeroes[i].health <= 0) continue; // Herói morto não ataca

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
        
        // NOVO: Lógica de morte do inimigo pelo herói
        if (enemies[targetEnemy].health <= 0) {
          
          // ✨ ADICIONA AS MOEDAS AO JOGADOR AQUI
          // Requer: o campo `recompensa_moedas` na struct Enemy
          // Requer: a função `adicionar_moedas(&gameRecursos, valor)` em recursos.c/h
          adicionar_moedas(&gameRecursos, enemies[targetEnemy].recompensa_moedas); 
          
          enemies[targetEnemy].active = 0;
          enemies_defeated_count++; // Incrementa o contador de vitória
        }
        placedHeroes[i].lastAttackTime = 0.0f;
      }
    }
  }

  // 2. Spawn new enemies if tower is alive and not at max enemies
  if (towerHealth > 0 && enemyCount < MAX_ENEMIES) {
    spawnTimer += GetFrameTime();
    if (spawnTimer >= SPAWN_INTERVAL) {
      enemies[enemyCount] = InitEnemy((int)path[0].x, (int)path[0].y);
      // ⭐️ Inicializa as variáveis de estado de ataque para o novo inimigo
      enemyLastAttackTime[enemyCount] = 0.0f;
      enemyTargetHero[enemyCount] = -1;
      
      enemyCount++;
      spawnTimer = 0.0f;
    }
  }

  // 3. Atualiza movimento e checa chegada à torre (e adiciona ataque a heróis)
  for (int i = 0; i < enemyCount; i++) {
    if (!enemies[i].active) continue;

    // ⭐️ LÓGICA DE ATAQUE DO INIMIGO AO HERÓI
    int targetHeroIndex = enemyTargetHero[i];
    
    // Se o inimigo não tem um alvo OU o alvo morreu/foi removido
    if (targetHeroIndex == -1 || placedHeroes[targetHeroIndex].health <= 0) {
      targetHeroIndex = -1; // Resetar o alvo

      // Tenta encontrar um herói no alcance
      float minDist = ENEMY_ATTACK_RANGE;
      for (int j = 0; j < placedHeroCount; j++) {
        if (placedHeroes[j].health > 0) { // Só mira em heróis vivos
          float dx = enemies[i].x - placedHeroes[j].x;
          float dy = enemies[i].y - placedHeroes[j].y;
          float dist = sqrt(dx*dx + dy*dy);
          if (dist <= minDist) { // Usa <= para mirar no herói que está no alcance
            minDist = dist;
            targetHeroIndex = j;
            break; // Alvo encontrado! (Poderia ser o mais próximo, mas o primeiro no alcance é mais simples)
          }
        }
      }
      enemyTargetHero[i] = targetHeroIndex; // Define o novo alvo
    }

    if (targetHeroIndex != -1) {
      // ⭐️ O inimigo PARA e ataca o herói!
      
      enemyLastAttackTime[i] += GetFrameTime();
      if (enemyLastAttackTime[i] >= ENEMY_ATTACK_INTERVAL) {
        // Ataca o herói
        placedHeroes[targetHeroIndex].health -= ENEMY_DAMAGE_TO_HERO;
        TraceLog(LOG_INFO, "Inimigo %d atacou Herói %d. Vida Heroi: %d", i, targetHeroIndex, placedHeroes[targetHeroIndex].health);
        
        // Checa se o herói morreu
        if (placedHeroes[targetHeroIndex].health <= 0) {
          placedHeroes[targetHeroIndex].health = 0;
          // O herói morto será "removido" na próxima iteração do loop dos inimigos
          // ou por uma função de limpeza (que não está implementada).
          // Por enquanto, apenas zeramos a vida para que ele seja ignorado.
        }
        enemyLastAttackTime[i] = 0.0f;
      }
      // Não chama UpdateEnemy, pois o inimigo está atacando
    } else {
      // Se não há herói no alcance, o inimigo se move em direção à torre
      UpdateEnemy(&enemies[i]);
      enemyTargetHero[i] = -1; // Garante que o alvo foi resetado
    }
    
    // Checa se chegou na torre (Lógica existente)
    if (EnemyReachedTower(enemies[i]) && enemies[i].active) {
      towerHealth -= ENEMY_DAMAGE_TO_CASTLE; 
      enemies[i].active = 0; // Desativa o inimigo após causar dano

      if (towerHealth <= 0) {
        towerHealth = 0;
        current_game_state = GAME_OVER; // MUDANÇA DE ESTADO: PERDEU
      }
    }
  }

  // 4. NOVO: Lógica de Vitória
  if (enemyCount >= MAX_ENEMIES && enemies_defeated_count >= MAX_ENEMIES) {
    current_game_state = WAVE_WON; // MUDANÇA DE ESTADO: VENCEU
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

// ... (Restante de DrawGame inalterado) ...
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
    for (int i = 0; i < enemyCount; i++) {
      DrawEnemy(enemies[i]);
    }

    // Desenhar heróis colocados
    for (int i = 0; i < placedHeroCount; i++) {
      if (placedHeroes[i].health <= 0) continue; // Não desenha heróis mortos

      Color heroColor;
      switch (placedHeroes[i].tipo) {
        case 0: heroColor = BLUE; break;
        case 1: heroColor = GREEN; break;
        case 2: heroColor = YELLOW; break;
        case 3: heroColor = PURPLE; break;
        default: heroColor = WHITE; break;
      }
      DrawCircle(placedHeroes[i].x, placedHeroes[i].y, 20, heroColor);
      // Desenhar alcance (círculo semi-transparente)
      DrawCircleLines(placedHeroes[i].x, placedHeroes[i].y, placedHeroes[i].alcance, (Color){heroColor.r, heroColor.g, heroColor.b, 100});
      // ⭐️ Desenhar alcance de ataque do inimigo (DEBUG)
      // DrawCircleLines(enemies[i].x, enemies[i].y, ENEMY_ATTACK_RANGE, (Color){255, 0, 0, 100});
      
      // Desenhar barra de vida dos heróis
      int barWidth = 40;
      int barHeight = 5;
      int barX = placedHeroes[i].x - barWidth / 2;
      int barY = placedHeroes[i].y - 30;
      DrawRectangle(barX, barY, barWidth, barHeight, RED);
      DrawRectangle(barX, barY, (int)(barWidth * (placedHeroes[i].health / 100.0f)), barHeight, GREEN);
    }
  }


  // 💰 Desenha o menu se estiver aberto
  if (menuAberto) {
    DrawMenuHerois();
  }
  
  // =======================================================
  // NOVO: TELA DE ESTADO FINAL (VITÓRIA OU DERROTA)
  // =======================================================

  if (current_game_state == GAME_OVER) {
    // TELA DE DERROTA
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RED, 0.8f)); 
    const char *message = "VOCÊ PERDEU! A TORRE FOI DESTRUÍDA.";
    int message_len = MeasureText(message, 40);
    DrawText(message, (GetScreenWidth() / 2) - (message_len / 2), GetScreenHeight() / 2 - 50, 40, WHITE);
    
  } else if (current_game_state == WAVE_WON) {
    // TELA DE VITÓRIA
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(DARKGREEN, 0.8f)); 
    
    const char *message = "VOCÊ GANHOU! IR PARA A FASE 2";
    int message_len = MeasureText(message, 40);
    DrawText(message, (GetScreenWidth() / 2) - (message_len / 2), GetScreenHeight() / 2 - 50, 40, GOLD);
    
    // Exemplo de botão
    Rectangle button_rect = { (GetScreenWidth() / 2) - 100, GetScreenHeight() / 2 + 50, 200, 50 };
    DrawRectangleRec(button_rect, YELLOW);
    DrawText("CONTINUAR", button_rect.x + 50, button_rect.y + 15, 20, BLACK);
  }

  EndDrawing();
}

// Finalização
void CloseGame(void) {
  // ✨ MODIFICAÇÃO 5: Libera a memória de ambas as texturas de fundo
    UnloadTexture(background);
    UnloadTexture(backgroundFase2);
    
  UnloadTexture(towerTexture);
  
  // 💰 Descarrega texturas dos heróis
  for (int i = 0; i < MAX_HEROIS; i++) {
    UnloadTexture(herois[i].texture);
  }
}