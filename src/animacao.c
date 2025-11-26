#include "raylib.h"
#include "animacao.h"

Texture2D backgroundFase1;
Texture2D backgroundFase2;
Texture2D backgroundFase3;
Texture2D fundoMenu;

static void ZerarAnimacoes(Personagem *p) {
    for (int i = 0; i < ESTADO_MAX; i++) {
        p->anim[i].textura.id = 0;
        p->anim[i].colunas = 1;
        p->anim[i].linhas = 1;
        p->anim[i].totalFrames = 1;
        p->anim[i].frameAtual = 0;
        p->anim[i].tempoPorFrame = 0.15f;
        p->anim[i].acumulado = 0.0f;
        p->anim[i].larguraFrame = 0;
        p->anim[i].alturaFrame = 0;
    }
}

static void CarregarAnimacao(Animacao *a, const char *arquivo, int colunas, int linhas, int totalFrames, float tempoPorFrame) {
    a->textura = LoadTexture(arquivo);
    a->colunas = colunas;
    a->linhas = linhas;
    a->totalFrames = totalFrames;
    a->frameAtual = 0;
    a->tempoPorFrame = tempoPorFrame;
    a->acumulado = 0.0f;

    if (colunas <= 0) colunas = 1;
    if (linhas <= 0) linhas = 1;
    a->larguraFrame = a->textura.width / colunas;
    a->alturaFrame = a->textura.height / linhas;
}

void CarregarTexturasGlobais(void) {
    backgroundFase1 = LoadTexture("resources/background_novo.jpg");
    backgroundFase2 = LoadTexture("resources/backgroundFase2.jpg");
    backgroundFase3 = LoadTexture("resources/backgroundFase3.jpg");
    fundoMenu = LoadTexture("resources/fundoMenu.png");
}

void DescarregarTexturasGlobais(void) {
    UnloadTexture(backgroundFase1);
    UnloadTexture(backgroundFase2);
    UnloadTexture(backgroundFase3);
    UnloadTexture(fundoMenu);
}

static void ConfigurarCavaleiro(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_IDLE], "resources/cavaleiro idle.png", 1, 2, 2, 0.15f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/cavaleiro ataque.png", 1, 2, 2, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/cavaleiro morte.png", 1, 1, 1, 0.15f);
}

static void ConfigurarMago(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_IDLE], "resources/idle mago.png", 1, 2, 2, 0.15f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/ataque mago.png", 1, 3, 3, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/morte Mago.png", 1, 1, 1, 0.15f);
}

static void ConfigurarPaladino(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_IDLE], "resources/idle paladino.png", 1, 2, 2, 0.15f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/ataque paladino.png", 1, 3, 3, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_CURA], "resources/cura paladino.png", 1, 2, 2, 0.12f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/morte paladino.png", 1, 1, 1, 0.15f);
}

static void ConfigurarBardo(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_IDLE], "resources/idle barod.png", 1, 2, 2, 0.15f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/ataque bardo.png", 2, 2, 4, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_BUFF], "resources/buff bardo.png", 1, 3, 3, 0.12f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/morte bardo.png", 1, 1, 1, 0.15f);
}

static void ConfigurarGoblin(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_ANDANDO], "resources/goblin direita.png", 1, 3, 3, 0.12f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/ataque goblin direita.png", 1, 3, 3, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/morte goblin.png", 1, 1, 1, 0.15f);
}

static void ConfigurarFantasma(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_ANDANDO], "resources/fantasma de frente.png", 1, 3, 3, 0.12f);
}

static void ConfigurarNecromante(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_ANDANDO], "resources/necro andando direita.png", 1, 3, 3, 0.12f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/necro ataque.png", 1, 3, 3, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_CURA], "resources/necro cura.png", 1, 2, 2, 0.12f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/necro morte .png", 1, 1, 1, 0.15f);
}

static void ConfigurarDragao(Personagem *p) {
    CarregarAnimacao(&p->anim[ESTADO_ANDANDO], "resources/dragao andando direita.png", 1, 3, 3, 0.10f);
    CarregarAnimacao(&p->anim[ESTADO_ATAQUE], "resources/dragao ataque direita.png", 1, 3, 3, 0.08f);
    CarregarAnimacao(&p->anim[ESTADO_MORTE], "resources/dragao morte.png", 1, 1, 1, 0.15f);
}

Personagem CriarPersonagem(TipoPersonagem tipo, Vector2 pos, float escala) {
    Personagem p;
    p.tipo = tipo;
    p.posicao = pos;
    p.escala = escala;
    p.estado = ESTADO_IDLE;
    p.ativo = 1;
    p.vida = 100;
    ZerarAnimacoes(&p);

    switch (tipo) {
        case TIPO_CAVALEIRO:
            ConfigurarCavaleiro(&p);
            break;
        case TIPO_MAGO:
            ConfigurarMago(&p);
            break;
        case TIPO_PALADINO:
            ConfigurarPaladino(&p);
            break;
        case TIPO_BARDO:
            ConfigurarBardo(&p);
            break;
        case TIPO_GOBLIN:
            ConfigurarGoblin(&p);
            break;
        case TIPO_FANTASMA:
            ConfigurarFantasma(&p);
            break;
        case TIPO_NECROMANTE:
            ConfigurarNecromante(&p);
            break;
        case TIPO_DRAGAO:
            ConfigurarDragao(&p);
            break;
        default:
            break;
    }

    return p;
}

void MudarEstado(Personagem *p, EstadoAnimacao estado) {
    if (!p) return;
    if (estado < 0 || estado >= ESTADO_MAX) return;
    if (p->estado == estado) return;
    p->estado = estado;
    Animacao *a = &p->anim[p->estado];
    a->frameAtual = 0;
    a->acumulado = 0.0f;
}

void AtualizarPersonagem(Personagem *p) {
    if (!p) return;
    Animacao *a = &p->anim[p->estado];
    if (a->textura.id == 0) return;
    if (a->totalFrames <= 1) return;

    a->acumulado += GetFrameTime();
    if (a->acumulado >= a->tempoPorFrame) {
        a->acumulado -= a->tempoPorFrame;
        a->frameAtual++;
        if (a->frameAtual >= a->totalFrames) {
            a->frameAtual = 0;
        }
    }
}

void DesenharPersonagem(const Personagem *p) {
    if (!p) return;
    const Animacao *a = &p->anim[p->estado];
    if (a->textura.id == 0) return;

    int coluna = 0;
    int linha = 0;

    if (a->totalFrames > 1 && a->colunas > 0) {
        coluna = a->frameAtual % a->colunas;
        linha = a->frameAtual / a->colunas;
    }

    Rectangle src = { coluna * a->larguraFrame, linha * a->alturaFrame, a->larguraFrame, a->alturaFrame };
    Rectangle dst = { p->posicao.x, p->posicao.y, a->larguraFrame * p->escala, a->alturaFrame * p->escala };
    Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(a->textura, src, dst, origin, 0.0f, WHITE);
}