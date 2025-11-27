
#ifndef RANKING_H
#define RANKING_H

#include <stdbool.h>

typedef struct ScoreNode {
    double timeSeconds;
    char name[64];
    char when[32]; 
    struct ScoreNode *next;
} ScoreNode;

void Ranking_Init(const char *filename);

void Ranking_Add(double timeSeconds, const char *name);

void Ranking_Save(void);

void Ranking_Free(void);

void Ranking_Draw(int x, int y, int topN);

ScoreNode *Ranking_GetHead(void);

#endif 
