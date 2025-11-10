// game.c

#include "raylib.h"
#include "enemy.h"
#include "game.h"
#include "recursos.h"
#include <stdio.h>
#include <string.h>

#define MAX_ENEMIES 5

Vector2 path[MAX_WAYPOINTS] = {
    {10, 440},      // 0: Início
    {120, 440},     // 1: Início da curva para baixo
    {150, 520},     // 2: NOVO PONTO
    {400, 520},     // 3: Seção horizontal inferior
    {400, 450},     // 4: Final da curva inferior
    {400, 200},     // 5: Subida
    {650, 200},     // 6: Curva superior
    {700, 180},     // 7: Preparação para ponte
    {700, 130},     // 8: Ponte
    {750, 100},     // 9: Torre (FIM)
};

static Enemy enemies[MAX_ENEMIES];
static int enemyCount = 0;
static Texture2D background;
static Texture2D towerTexture;
static int towerHealth = 100;
static recursos gameRecursos;

// 💰 Variáveis do sistema de menu
static bool menuAberto = false;
static Heroi herois[MAX_HEROIS];

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
    background = LoadTexture("resources/background.jpg");
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
    
    // Atualização normal do jogo (quando menu fechado)
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