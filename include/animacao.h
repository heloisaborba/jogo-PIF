#ifndef ANIMACAO_H
#define ANIMACAO_H

#include "raylib.h"

typedef enum {
    ESTADO_IDLE,
    ESTADO_ANDANDO,
    ESTADO_ATAQUE,
    ESTADO_BUFF,
    ESTADO_CURA,
    ESTADO_MORTE,
    ESTADO_MAX
} EstadoAnimacao;

typedef enum {
    TIPO_CAVALEIRO,
    TIPO_MAGO,
    TIPO_PALADINO,
    TIPO_BARDO,
    TIPO_GOBLIN,
    TIPO_FANTASMA,
    TIPO_NECROMANTE,
    TIPO_DRAGAO,
    TIPO_MAX
} TipoPersonagem;

typedef struct {
    Texture2D textura;
    int colunas;
    int linhas;
    int totalFrames;
    int frameAtual;
    float tempoPorFrame;
    float acumulado;
    int larguraFrame;
    int alturaFrame;
} Animacao;

typedef struct {
    TipoPersonagem tipo;
    Vector2 posicao;
    float escala;
    EstadoAnimacao estado;
    Animacao anim[ESTADO_MAX];
    int ativo;
    int vida;
} Personagem;

extern Texture2D backgroundFase1;
extern Texture2D backgroundFase2;
extern Texture2D backgroundFase3;
extern Texture2D fundoMenu;

void CarregarTexturasGlobais(void);
void DescarregarTexturasGlobais(void);

Personagem CriarPersonagem(TipoPersonagem tipo, Vector2 pos, float escala);
void MudarEstado(Personagem *p, EstadoAnimacao estado);
void AtualizarPersonagem(Personagem *p);
void DesenharPersonagem(const Personagem *p);

#endif