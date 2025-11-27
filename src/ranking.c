// ranking.c
#include "ranking.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static ScoreNode *head = NULL;
static char ranking_filename[260] = "ranking.txt";

// Helper: create node
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

// Inserts sorted descending by time (longer time first)
static void insert_sorted(ScoreNode *node) {
    if (!node) return;
    if (!head || node->timeSeconds >= head->timeSeconds) {
        node->next = head;
        head = node;
        return;
    }
    ScoreNode *cur = head;
    // Percorre até achar o nó cuja pontuação é menor ou até o fim da lista
    while (cur->next && cur->next->timeSeconds > node->timeSeconds) cur = cur->next;
    node->next = cur->next;
    cur->next = node;
}

void Ranking_Init(const char *filename) {
    if (filename && filename[0]) strncpy(ranking_filename, filename, sizeof(ranking_filename)-1);
    ranking_filename[sizeof(ranking_filename)-1] = '\0';

    // Clear existing list to avoid duplicates on multiple inits
    Ranking_Free();

    // load file if exists
    FILE *f = fopen(ranking_filename, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        // Expect a line like: <time> <name (may contain spaces)> <when>
        // where <when> has no spaces (we use strftime with underscore)
        double t = 0.0;
        // parse leading time
        if (sscanf(line, "%lf", &t) != 1) continue;

        // find first space after time
        char *firstSpace = strchr(line, ' ');
        if (!firstSpace) continue;

        // find last space (before the timestamp)
        char *lastSpace = strrchr(line, ' ');
        if (!lastSpace || lastSpace == firstSpace) continue;

        // extract name between firstSpace+1 and lastSpace-1
        size_t nameLen = (size_t)(lastSpace - (firstSpace + 1));
        char nameBuf[64] = "";
        if (nameLen > 0) {
            if (nameLen >= sizeof(nameBuf)) nameLen = sizeof(nameBuf) - 1;
            memcpy(nameBuf, firstSpace + 1, nameLen);
            nameBuf[nameLen] = '\0';
            // trim trailing spaces
            while (nameLen > 0 && nameBuf[nameLen-1] == ' ') { nameBuf[nameLen-1] = '\0'; nameLen--; }
        }

        // extract when (timestamp) from lastSpace+1 up to newline
        char whenBuf[32] = "";
        size_t whenLen = strlen(lastSpace + 1);
        // remove trailing newline or spaces
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
        // write: time name when
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
        // Format: 1. Name - 123.45 s
        snprintf(line, sizeof(line), "%d. %s - %.2f s", idx, cur->name, cur->timeSeconds);
        DrawText(line, x, drawY, 22, RAYWHITE);
        drawY += lineHeight;
        cur = cur->next;
        idx++;
    }
}