#include "raylib.h"
#include "menu.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Texture2D RedimensionarTextureMantendoProporcao(Texture2D textureOriginal, int novaLargura, int novaAltura) {
    Image imagemOriginal = LoadImageFromTexture(textureOriginal);
    
    float ratioOriginal = (float)imagemOriginal.width / imagemOriginal.height;
    float ratioNova = (float)novaLargura / novaAltura;
    
    int larguraFinal, alturaFinal;
    
    if (ratioOriginal > ratioNova) {
        larguraFinal = novaLargura;
        alturaFinal = novaLargura / ratioOriginal;
    } else {
        alturaFinal = novaAltura;
        larguraFinal = novaAltura * ratioOriginal;
    }
    
    ImageResize(&imagemOriginal, larguraFinal, alturaFinal);
    Texture2D textureRedimensionada = LoadTextureFromImage(imagemOriginal);
    UnloadImage(imagemOriginal);
    
    return textureRedimensionada;
}

Texture2D RedimensionarTexture(Texture2D textureOriginal, int novaLargura, int novaAltura) {
    
    RenderTexture2D target = LoadRenderTexture(novaLargura, novaAltura);
    
    BeginTextureMode(target);
    ClearBackground(BLANK);
    
    float escalaX = (float)novaLargura / textureOriginal.width;
    float escalaY = (float)novaAltura / textureOriginal.height;
    float escala = (escalaX < escalaY) ? escalaX : escalaY;
    
    Vector2 posicao = {
        (novaLargura - textureOriginal.width * escala) / 2,
        (novaAltura - textureOriginal.height * escala) / 2
    };
    
    DrawTextureEx(textureOriginal, posicao, 0, escala, WHITE);
    
    EndTextureMode();
    
    Texture2D textureRedimensionada = target.texture;

    Image imagemRedimensionada = LoadImageFromTexture(textureRedimensionada);
    ImageFlipVertical(&imagemRedimensionada); 
    
    Texture2D textureFinal = LoadTextureFromImage(imagemRedimensionada);
    
    UnloadRenderTexture(target);
    UnloadImage(imagemRedimensionada);
    
    return textureFinal;
}

void ShowHeroesMenu(PersonagemInfo personagens[]) { 
    double t = GetTime();
    while (GetTime() - t < 0.15) { GetKeyPressed(); }

    int currentPersonagem = 0;

    Rectangle leftArrowRect, rightArrowRect, backButtonRect;

    while (!WindowShouldClose()) {

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        float fontSmall  = sh * 0.024f;  
        float fontMedium = sh * 0.034f;
        float fontLarge  = sh * 0.055f;

        leftArrowRect  = (Rectangle){ 60, sh/2 - 25, 50, 50 };
        rightArrowRect = (Rectangle){ sw - 110, sh/2 - 25, 50, 50 };
        backButtonRect = (Rectangle){ 40, sh - 80, 140, 50 };

        Vector2 mousePos = GetMousePosition();
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE))
            return;

        if (IsKeyPressed(KEY_LEFT) || (mouseReleased && CheckCollisionPointRec(mousePos, leftArrowRect)))
            currentPersonagem = (currentPersonagem - 1 + 4) % 4;

        if (IsKeyPressed(KEY_RIGHT) || (mouseReleased && CheckCollisionPointRec(mousePos, rightArrowRect)))
            currentPersonagem = (currentPersonagem + 1) % 4;

        if (mouseReleased && CheckCollisionPointRec(mousePos, backButtonRect))
            return;

        BeginDrawing();
        ClearBackground((Color){ 25, 20, 15, 255 }); 

        DrawRectangle(30, 30, sw - 60, sh - 60, (Color){ 45, 35, 25, 240 });
        DrawRectangleLinesEx((Rectangle){30, 30, sw - 60, sh - 60}, 4, (Color){180, 140, 80, 255});

        const char *title = "GRANDES HERÓIS DO REINO";
        int titleWidth = MeasureText(title, fontLarge);
        DrawText(title, 
            sw/2 - titleWidth/2, 
            50, 
            fontLarge, 
            (Color){ 220, 180, 80, 255 }
        );

        DrawRectangleRec(leftArrowRect, (Color){ 100, 75, 50, 255 });
        DrawRectangleRec(rightArrowRect, (Color){ 100, 75, 50, 255 });
        DrawRectangleLinesEx(leftArrowRect, 2, (Color){200, 160, 100, 255});
        DrawRectangleLinesEx(rightArrowRect, 2, (Color){200, 160, 100, 255});

        int arrowTextWidth = MeasureText("<", fontMedium);
        DrawText("<", leftArrowRect.x + (50 - arrowTextWidth)/2, leftArrowRect.y + 12, fontMedium, WHITE);
        DrawText(">", rightArrowRect.x + (50 - arrowTextWidth)/2, rightArrowRect.y + 12, fontMedium, WHITE);

        char pageText[20];
        sprintf(pageText, "%d / 4", currentPersonagem + 1);
        int pageWidth = MeasureText(pageText, fontMedium);
        DrawText(pageText, sw/2 - pageWidth/2, sh - 66, fontMedium, (Color){220, 200, 150, 255});

        const char *nome;
        const char *descricao[5]; 
        Color nomeCor;

        switch (currentPersonagem) {
            case 0:
                nome = "O CAVALEIRO";
                nomeCor = (Color){255, 215, 0, 255}; 
                descricao[0] = "Espada de aço templário";
                descricao[1] = "Armadura de placas reais";
                descricao[2] = "Proteção da linha de frente";
                descricao[3] = "uramento de honra e lealdad";
                descricao[4] = "Pouco dano, atacando apenas Dragão e Goblim";
                break;

            case 1:
                nome = "O MAGO";
                nomeCor = (Color){100, 200, 100, 255}; 
                descricao[0] = "Orbe dos elementos antigos";
                descricao[1] = "Conhecimento arcano proibido";
                descricao[2] = "Feitiços de eras esquecidas";
                descricao[3] = "Visão além do véu mortal";
                descricao[4] = "Poder sobre vida e morte";
                break;

            case 2:
                nome = "O BARDO";
                nomeCor = (Color){100, 150, 255, 255}; 
                descricao[0] = "Canções de batalha ancestral";
                descricao[1] = "Alaúde das terras distantes";
                descricao[2] = "Inspira coragem nos aliados";
                descricao[3] = "Melodias que curam feridas";
                descricao[4] = "Crônicas dos grandes feitos";
                break;

            default:
                nome = "O PALADINO";
                nomeCor = (Color){180, 100, 220, 255}; 
                descricao[0] = "Escudo sagrado abençoado";
                descricao[1] = "Cura pelas mãos divinas";
                descricao[2] = "Fé inabalável no combate";
                descricao[3] = "Proteção aos mais fracos";
                descricao[4] = "Luz que afasta as trevas";
                break;
        }

        int nomeWidth = MeasureText(nome, fontMedium);
        DrawText(nome, sw/2 - nomeWidth/2, 110, fontMedium, nomeCor);

        Rectangle imgBox = { sw/2 - 100, 150, 200, 200 };
        DrawRectangleRec(imgBox, (Color){ 80, 65, 45, 240 });
        DrawRectangleLinesEx(imgBox, 3, (Color){ 180, 150, 100, 255 });

        if (personagens[currentPersonagem].img.id != 0) {
            Texture2D texture = personagens[currentPersonagem].img;
            float scale = 0.4f;
            Vector2 position = {
                imgBox.x + (imgBox.width - texture.width * scale) / 2,
                imgBox.y + (imgBox.height - texture.height * scale) / 2
            };
            DrawTextureEx(texture, position, 0, scale, WHITE);
        } else {
            const char *placeholder = "⚔️";
            int placeholderWidth = MeasureText(placeholder, 40);
            DrawText(placeholder, 
                    imgBox.x + (imgBox.width - placeholderWidth)/2, 
                    imgBox.y + 80, 
                    40, 
                    (Color){160, 140, 100, 255});
        }

        Rectangle descBox = { 80, 350, sw - 160, sh - 430 };
        DrawRectangleRec(descBox, (Color){ 70, 55, 40, 230 });
        DrawRectangleLinesEx(descBox, 3, (Color){ 170, 140, 90, 255 });

        const char *descTitle = "HABILIDADES E CARACTERÍSTICAS";
        int descTitleWidth = MeasureText(descTitle, fontSmall * 1.1f);
        DrawText(descTitle, 
                sw/2 - descTitleWidth/2, 
                descBox.y + 15, 
                fontSmall * 1.1f, 
                (Color){220, 190, 120, 255});

        float lineSpacing = fontSmall * 1.6f;
        float startY = descBox.y + 50;
        
        for (int i = 0; i < 5; i++) {
            int lineWidth = MeasureText(descricao[i], fontSmall);
            DrawText(descricao[i], 
                    sw/2 - lineWidth/2, 
                    startY + (i * lineSpacing), 
                    fontSmall, 
                    (Color){240, 230, 210, 255});
        }

        Color backColor = CheckCollisionPointRec(mousePos, backButtonRect)
            ? (Color){ 140, 100, 60, 255 }
            : (Color){ 110, 80, 50, 255 };

        DrawRectangleRec(backButtonRect, backColor);
        DrawRectangleLinesEx(backButtonRect, 2, (Color){200, 170, 120, 255});
        
        const char *backText = "VOLTAR";
        int backTextWidth = MeasureText(backText, fontSmall * 1.1f);
        DrawText(backText, 
                backButtonRect.x + (backButtonRect.width - backTextWidth)/2, 
                backButtonRect.y + 15, 
                fontSmall * 1.1f, 
                WHITE);

            EndDrawing();
        }
    }

void ShowHowToPlayMenu() { 
    double t = GetTime();
    while (GetTime() - t < 0.15) { GetKeyPressed(); }

    Rectangle backButtonRect;

    while (!WindowShouldClose()) {

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        float fontSmall  = sh * 0.022f;  
        float fontMedium = sh * 0.028f;  
        float fontLarge  = sh * 0.050f;

        backButtonRect = (Rectangle){ 40, sh - 80, 180, 50 };

        Vector2 mousePos = GetMousePosition();
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE) ||
            (mouseReleased && CheckCollisionPointRec(mousePos, backButtonRect))) {
            return;
        }

        BeginDrawing();
        ClearBackground((Color){ 20, 15, 10, 255 });

        DrawRectangle(25, 25, sw - 50, sh - 50, (Color){ 45, 35, 25, 245 });
        DrawRectangleLinesEx((Rectangle){25, 25, sw - 50, sh - 50}, 4, (Color){180, 140, 80, 255});

        const char *title = "COMO JOGAR:";
        int titleWidth = MeasureText(title, fontLarge);
        DrawText(title,
            sw/2 - titleWidth/2,
            45,
            fontLarge,
            (Color){ 220, 180, 80, 255 }
        );

        Rectangle textBox = { 60, 140, sw - 120, sh - 250 };
        DrawRectangleRec(textBox, (Color){ 65, 50, 35, 235 });
        DrawRectangleLinesEx(textBox, 3, (Color){170, 140, 90, 255});

        struct Section {
            const char *title;
            const char *lines[6];
            int lineCount;
        };

        struct Section sections[] = {
            {
                "COMO JOGAR",
                {"Posicione herois nas rotas para bloquear inimigos", "", NULL},
                2
            },
            {
                "SISTEMA DE MOEDAS",
                {
                    "Derrote inimigos para ganhar moedas", 
                    "Use moedas para comprar/evoluir herois",
                    "", NULL
                },
                3
            },
            {
                "DEFESA DA TORRE",
                {
                    "Se os inimigos chegarem a torre, ela toma dano",
                    "Salve a torre ate o fim das ordas para vencer!",
                    "", NULL
                },
                3
            }
        };

        int numSections = sizeof(sections) / sizeof(sections[0]);
        float currentY = textBox.y + 25;
        float sectionSpacing = 25;
        float lineSpacing = fontSmall * 1.4f;

        for (int s = 0; s < numSections; s++) {
           
            int titleWidth = MeasureText(sections[s].title, fontMedium);
            DrawText(sections[s].title, 
                    textBox.x + (textBox.width - titleWidth)/2, 
                    currentY, 
                    fontMedium, 
                    (Color){220, 190, 120, 255});
            
            currentY += lineSpacing * 1.2f;

            
            for (int l = 0; l < sections[s].lineCount; l++) {
                if (sections[s].lines[l][0] != '\0') { 
                    int lineWidth = MeasureText(sections[s].lines[l], fontSmall);
                    DrawText(sections[s].lines[l], 
                            textBox.x + (textBox.width - lineWidth)/2, 
                            currentY, 
                            fontSmall, 
                            (Color){240, 230, 210, 255});
                }
                currentY += lineSpacing;
            }
            
            currentY += sectionSpacing;
            
            if (s < numSections - 1) {
                DrawRectangle(textBox.x + 40, currentY - sectionSpacing/2, textBox.width - 80, 2, (Color){150, 120, 80, 255});
            }
        }

        const char *proverbio = "Um bom estrategista vence batalhas, mas um grande líder conquista reinos!";
        int proverbioWidth = MeasureText(proverbio, fontSmall * 0.9f);
        DrawText(proverbio,
                textBox.x + (textBox.width - proverbioWidth)/2,
                textBox.y + textBox.height - 30,
                fontSmall * 0.9f,
                (Color){180, 160, 120, 255});

        Color backColor = CheckCollisionPointRec(mousePos, backButtonRect)
            ? (Color){ 140, 100, 60, 255 }
            : (Color){ 110, 80, 50, 255 };

        DrawRectangleRec(backButtonRect, backColor);
        DrawRectangleLinesEx(backButtonRect, 2, (Color){200, 170, 120, 255});
        
        const char *backText = "VOLTAR";
        int backTextWidth = MeasureText(backText, fontSmall * 1.1f);
        DrawText(backText, 
                backButtonRect.x + (backButtonRect.width - backTextWidth)/2, 
                backButtonRect.y + 15, 
                fontSmall * 1.1f, 
                WHITE);

        EndDrawing();
    }
}

void ShowRankingMenu() {
    double t = GetTime();
    while (GetTime() - t < 0.15) { GetKeyPressed(); }

    Rectangle backButtonRect;

    while (!WindowShouldClose()) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        
        float fontSmall = sh * 0.024f;
        float fontMedium = sh * 0.032f;
        float fontLarge = sh * 0.052f;
        
        backButtonRect = (Rectangle){ 50, sh - 90, 200, 55 };

        Vector2 mousePos = GetMousePosition();
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE) || 
            (mouseReleased && CheckCollisionPointRec(mousePos, backButtonRect))) {
            break;
        }

        BeginDrawing();
        ClearBackground((Color){ 20, 15, 10, 255 });

        DrawRectangle(30, 30, sw - 60, sh - 60, (Color){ 45, 35, 25, 240 });
        DrawRectangleLinesEx((Rectangle){30, 30, sw - 60, sh - 60}, 4, (Color){180, 140, 80, 255});

        const char *title = "SALÃO DA FAMA DOS CAMPEÕES";
        int titleWidth = MeasureText(title, fontLarge);
        DrawText(title, 
                sw/2 - titleWidth/2, 
                55, 
                fontLarge, 
                (Color){ 220, 180, 80, 255 });
        
        const char* ranking[] = {
            "⚔️ GRANDES FEITOS DOS HERÓIS ⚔️",
            "",
            "🥇 SIR GALAHAD - 15.000 honras",
            "🥇 DAMA ELENA - 12.500 honras", 
            "🥉 BARÃO KARL - 10.200 honras",
            "🎖️ LORENZO - 8.700 honras",
            "🎖️ MERLIN - 7.800 honras",
            "",
            "💎 SABEDORIA DO CONSELHO:",
            "Cada invasor derrotado aumenta tua glória",
            "",
            "🎯 HONRAS ATUAIS: A conquistar...",
            "📜 TEU MAIOR FEITO: A ser escrito..."
        };
        
        float startY = 130;
        float lineHeight = fontSmall * 1.7f;
        
        for (int i = 0; i < 12; i++) {
            Color textColor = (i == 0) ? (Color){220, 190, 130, 255} : 
                            (i >= 2 && i <= 6) ? (Color){230, 210, 190, 255} : 
                            (Color){190, 170, 150, 255};
            int fontSize = (i == 0) ? fontMedium : fontSmall;
            
            int textWidth = MeasureText(ranking[i], fontSize);
            DrawText(ranking[i], 
                    sw/2 - textWidth/2, 
                    startY + (i * lineHeight), 
                    fontSize, 
                    textColor);
        }
        
        Color backColor = CheckCollisionPointRec(mousePos, backButtonRect) ? 
                        (Color){ 140, 100, 60, 255 } : (Color){ 110, 80, 50, 255 };
        
        DrawRectangleRec(backButtonRect, backColor);
        DrawRectangleLinesEx(backButtonRect, 3, (Color){200, 170, 120, 255});
        
        const char *backText = "VOLTAR";
        int backTextWidth = MeasureText(backText, fontSmall * 1.1f);
        DrawText(backText, 
                backButtonRect.x + (backButtonRect.width - backTextWidth)/2, 
                backButtonRect.y + 17, 
                fontSmall * 1.1f, 
                (Color){240, 220, 160, 255});

        EndDrawing();
    }
}

MenuOption ShowMenu() {
    const char *options[MENU_TOTAL] = {
        "Start Game",
        "Heroes",
        "How to Play",
        "Ranking",
        "Exit"
    };

    int selected = 0;

    Texture2D background = LoadTexture("resources/fundoMenu.png");

    PersonagemInfo personagens[4];

    typedef struct {
        const char* arquivo;
        int largura;
        int altura;
    } ConfigPersonagem;
    
    ConfigPersonagem configs[] = {
        {"resources/Cavaleiro.png", 550, 550},   
        {"resources/SapoMago.png",  530, 530},   
        {"resources/Bardo.png",     540, 540},   
        {"resources/Paladino.png",  560, 560}    
    };
    
    
    printf("=== CARREGANDO E REDIMENSIONANDO IMAGENS ===\n");
    
    for (int i = 0; i < 4; i++) {
        printf("Carregando: %s\n", configs[i].arquivo);
        
        Texture2D texOriginal = LoadTexture(configs[i].arquivo);
        
        if (texOriginal.id == 0) {
            printf("ERRO: Não foi possível carregar %s\n", configs[i].arquivo);
            
            Image placeholder = GenImageColor(configs[i].largura, configs[i].altura, (Color){100, 100, 100, 255});
            personagens[i].img = LoadTextureFromImage(placeholder);
            UnloadImage(placeholder);
            
            printf("Usando placeholder para %s\n", configs[i].arquivo);
        } else {
            printf("Sucesso: %s - Original: %dx%d -> Novo: %dx%d\n", 
                   configs[i].arquivo, texOriginal.width, texOriginal.height, 
                   configs[i].largura, configs[i].altura);
            
            personagens[i].img = RedimensionarTexture(texOriginal, configs[i].largura, configs[i].altura);
            UnloadTexture(texOriginal);
            
            printf("Redimensionado para: %dx%d\n", personagens[i].img.width, personagens[i].img.height);
        }
        
        switch(i) {
            case 0:
                personagens[i].nome = "Cavaleiro";
                personagens[i].descricao = "Um combatente forte que protege a linha de frente.";
                break;
            case 1:
                personagens[i].nome = "Mago";
                personagens[i].descricao = "Ataca com magia poderosa e dano em área.";
                break;
            case 2:
                personagens[i].nome = "Bardo";
                personagens[i].descricao = "Toca músicas para atacar e buffar aliados.";
                break;
            case 3:
                personagens[i].nome = "Paladino";
                personagens[i].descricao = "Protege aliados com escudo e cura.";
                break;
        }
    }
    
    printf("=== TODAS AS IMAGENS CARREGADAS ===\n");

    void ShowNamePromptMenu(void) {
        char buffer[64] = "";
        int len = 0;
        bool done = false;

        while (!WindowShouldClose() && !done) {
            int c = GetCharPressed();
            while (c > 0) {
                if (c >= 32 && c < 128 && len < (int)sizeof(buffer)-1) {
                    buffer[len++] = (char)c; buffer[len] = '\0';
                }
                c = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) { if (len>0) { len--; buffer[len]='\0'; } }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) { done = true; }
            if (IsKeyPressed(KEY_ESCAPE)) { buffer[0]='\0'; done = true; }

            BeginDrawing();
            DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
            const char *title = "DIGITE SEU NOME";
            int tw = MeasureText(title, 28);
            DrawText(title, GetScreenWidth()/2 - tw/2, 140, 28, GOLD);

            int boxW=520, boxH=64;
            int bx = GetScreenWidth()/2 - boxW/2;
            int by = GetScreenHeight()/2 - boxH/2;
            DrawRectangle(bx, by, boxW, boxH, LIGHTGRAY);
            DrawRectangleLines(bx, by, boxW, boxH, DARKGRAY);
            DrawText(buffer, bx+12, by+14, 28, BLACK);

            DrawText("Enter confirmar, Esc = Anon", GetScreenWidth()/2 - 160, by + boxH + 12, 14, GRAY);
            EndDrawing();
        }

        if (buffer[0] != '\0') {
            strncpy(playerName, buffer, sizeof(playerName)-1);
            playerName[sizeof(playerName)-1] = '\0';
        }
    }


    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_UP)) {
            selected--;
            if (selected < 0) selected = MENU_TOTAL - 1;
        }

        if (IsKeyPressed(KEY_DOWN)) {
            selected++;
            if (selected >= MENU_TOTAL) selected = 0;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (selected == MENU_HEROES) {
                ShowHeroesMenu(personagens);
                GetKeyPressed();
                WaitTime(0.15f);
                selected = 1;
                continue;
            } else if (selected == MENU_HOW_TO_PLAY) {
                ShowHowToPlayMenu();
                GetKeyPressed();
                WaitTime(0.15f);
                selected = 2;
                continue;
            } else if (selected == MENU_RANKING) {
                ShowRankingMenu();
                GetKeyPressed();
                WaitTime(0.15f);
                selected = 3;
                continue;
            } else if (selected == MENU_EXIT) {
                
                CloseWindow();
                exit(0);
            } else {
                
                ShowNamePromptMenu();
                break;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        float sw = GetScreenWidth();
        float sh = GetScreenHeight();

        float ratioImg = (float)background.width / background.height;
        float ratioScreen = sw / sh;

        float scale = (ratioScreen < ratioImg ? sw / background.width : sh / background.height);
        float w = background.width * scale;
        float h = background.height * scale;

        Rectangle src = {0, 0, background.width, background.height};
        Rectangle dst = {(sw - w) / 2, (sh - h) / 2, w, h};

        DrawTexturePro(background, src, dst, (Vector2){0, 0}, 0, WHITE);

        DrawText("=== TOWER DEFENSE ===", sw * 0.22, sh * 0.15, 40 * (sh / 720.0f), WHITE);

        int menuFont = (int)(30 * (sh / 720.0f));
        float yStart = sh * 0.40;
        float spacing = sh * 0.07;

        for (int i = 0; i < MENU_TOTAL; i++) {
            Color color = (i == selected) ? GREEN : WHITE;
            const char *txt = TextFormat("%s%s", i == selected ? "> " : "  ", options[i]);

            Vector2 s = MeasureTextEx(GetFontDefault(), txt, menuFont, 0);
            DrawText(txt, (sw - s.x) / 2.2, yStart + i * spacing, menuFont, color);
        }

        DrawText("Use ARROWS + ENTER", sw * 0.35, sh * 0.8, 20, GRAY);

        EndDrawing();
    }

    for (int i = 0; i < 4; i++) {
        UnloadTexture(personagens[i].img);
    }
    UnloadTexture(background);

    return (MenuOption)selected;
}