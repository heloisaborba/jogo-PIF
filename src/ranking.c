
#include "ranking.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static ScoreNode *head = NULL;
static char ranking_filename[260] = "ranking.txt";

static ScoreNode *create_node(double timeSeconds, const char *name, const char *when) {
    ScoreNode *n = (ScoreNode*)malloc(sizeof(ScoreNode));
    if (!n) return NULL;
    n->timeSeconds = timeSeconds;
    if (name) strncpy(n->name, name, sizeof(n->name)-1);
    n->name[sizeof(n->name)-1] = '\0';
    if (when) strncpy(n->when, when, sizeof(n->when)-1);
    n->when[sizeof(n->when)-1] = '\0';
    n->next = NULL;
    return n;
}

static void insert_sorted(ScoreNode *node) {
    if (!node) return;
    if (!head || node->timeSeconds >= head->timeSeconds) {
        node->next = head;
        head = node;
        return;
    }
    ScoreNode *cur = head;
    while (cur->next && cur->next->timeSeconds > node->timeSeconds) cur = cur->next;
    node->next = cur->next;
    cur->next = node;
}

void Ranking_Init(const char *filename) {
    if (filename && filename[0]) strncpy(ranking_filename, filename, sizeof(ranking_filename)-1);
    ranking_filename[sizeof(ranking_filename)-1] = '\0';

    Ranking_Free();

    FILE *f = fopen(ranking_filename, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        double t = 0.0;
        if (sscanf(line, "%lf", &t) != 1) continue;

        char *firstSpace = strchr(line, ' ');
        if (!firstSpace) continue;

        char *lastSpace = strrchr(line, ' ');
        if (!lastSpace || lastSpace == firstSpace) continue;

        size_t nameLen = (size_t)(lastSpace - (firstSpace + 1));
        char nameBuf[64] = "";
        if (nameLen > 0) {
            if (nameLen >= sizeof(nameBuf)) nameLen = sizeof(nameBuf) - 1;
            memcpy(nameBuf, firstSpace + 1, nameLen);
            nameBuf[nameLen] = '\0';
            while (nameLen > 0 && nameBuf[nameLen-1] == ' ') { nameBuf[nameLen-1] = '\0'; nameLen--; }
        }

        char whenBuf[32] = "";
        size_t whenLen = strlen(lastSpace + 1);
        while (whenLen > 0 && (lastSpace[1 + whenLen - 1] == '\n' || lastSpace[1 + whenLen - 1] == '\r' || lastSpace[1 + whenLen - 1] == ' ')) whenLen--;
        if (whenLen > 0) {
            if (whenLen >= sizeof(whenBuf)) whenLen = sizeof(whenBuf) - 1;
            memcpy(whenBuf, lastSpace + 1, whenLen);
            whenBuf[whenLen] = '\0';
        }

        ScoreNode *n = create_node(t, nameBuf, whenBuf);
        insert_sorted(n);
    }
    fclose(f);
}

void Ranking_Add(double timeSeconds, const char *name) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char when[32];
    if (tm) strftime(when, sizeof(when), "%Y-%m-%d_%H:%M:%S", tm);
    else strncpy(when, "unknown", sizeof(when)-1);
    ScoreNode *n = create_node(timeSeconds, name ? name : "Anon", when);
    insert_sorted(n);
}

void Ranking_Save(void) {
    FILE *f = fopen(ranking_filename, "w");
    if (!f) return;
    ScoreNode *cur = head;
    while (cur) {
        fprintf(f, "%lf %s %s\n", cur->timeSeconds, cur->name, cur->when);
        cur = cur->next;
    }
    fclose(f);
}

void Ranking_Free(void) {
    ScoreNode *cur = head;
    while (cur) {
        ScoreNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    head = NULL;
}

ScoreNode *Ranking_GetHead(void) {
    return head;
}

void Ranking_Draw(int x, int y, int topN) {
    if (topN <= 0) return;
    ScoreNode *cur = head;
    int idx = 1;
    int lineHeight = 28;
    int drawY = y;
    while (cur && idx <= topN) {
        char line[256];
        snprintf(line, sizeof(line), "%d. %s - %.2f s", idx, cur->name, cur->timeSeconds);
        DrawText(line, x, drawY, 22, RAYWHITE);
        drawY += lineHeight;
        cur = cur->next;
        idx++;
    }
}