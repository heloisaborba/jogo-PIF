// ranking.c
#include "ranking.h"
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
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        double t = 0.0;
        char name[64] = "";
        char when[32] = "";
        // File format: <time> <name> <when>
        if (sscanf(line, "%lf %63s %31s", &t, name, when) >= 1) {
            ScoreNode *n = create_node(t, name, when);
            insert_sorted(n);
        }
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
    // This function is intentionally left minimal because drawing needs raylib.
    // The game will call Ranking_GetHead() and draw entries from there using DrawText.
    (void)x; (void)y; (void)topN;
}
