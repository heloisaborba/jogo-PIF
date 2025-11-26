// ranking.h
#ifndef RANKING_H
#define RANKING_H

#include <stdbool.h>

// Nó para lista encadeada de pontuações (tempo em segundos)
typedef struct ScoreNode {
    double timeSeconds;
    char name[64];
    char when[32]; // timestamp string
    struct ScoreNode *next;
} ScoreNode;

// Inicializa o sistema de ranking, lê `filename` se existir
void Ranking_Init(const char *filename);

// Adiciona um novo registro (tempo em segundos) com nome do jogador
void Ranking_Add(double timeSeconds, const char *name);

// Salva o ranking atual no arquivo (sobrescreve)
void Ranking_Save(void);

// Libera memória alocada pela lista
void Ranking_Free(void);

// Desenha os top N do ranking na tela (opcional)
void Ranking_Draw(int x, int y, int topN);

// Retorna ponteiro para o primeiro nó (apenas leitura)
ScoreNode *Ranking_GetHead(void);

#endif // RANKING_H
