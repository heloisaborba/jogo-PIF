// enemy.c

#include "raylib.h"
#include "enemy.h"
#include "game.h" // Inclui game.h para ter acesso a MAX_WAYPOINTS e path
#include <math.h> // Necessário para sqrtf e roundf

#define ENEMY_SPEED 1.0f // Velocidade de movimento (pode ser float)

// Declaração externa correta: path usa o tamanho MAX_WAYPOINTS de game.h
extern Vector2 path[MAX_WAYPOINTS]; 

Enemy InitEnemy(int x, int y) {
    // Inicializa o inimigo com o ponto inicial (x, y) e mirando o Próximo Waypoint (índice 1)
    Enemy e = { x, y, 50, 1, 1 }; 
    return e;
}

void UpdateEnemy(Enemy *e) {
    if (!e->active) return;
    
    // Se o inimigo atingiu ou passou o último Waypoint, ele chegou ao destino.
    if (e->currentWaypoint >= MAX_WAYPOINTS) {
        return; 
    }

    Vector2 target = path[e->currentWaypoint];
    
    // 1. Calcular a diferença (vetor) para o Waypoint
    float dx = target.x - e->x;
    float dy = target.y - e->y;
    float distance = sqrtf(dx*dx + dy*dy); // Distância Euclidiana

    // 2. Mover em direção ao Waypoint
    if (distance > ENEMY_SPEED) { // Mover se estiver longe o suficiente
        
        // Normalizar o vetor de direção e aplicar a velocidade
        float factor = ENEMY_SPEED / distance;
        
        // Aplicar o movimento. Usamos roundf para suavizar a conversão para int.
        e->x += (int)roundf(dx * factor);
        e->y += (int)roundf(dy * factor);
    } 
    
    // 3. Se o inimigo atingiu o Waypoint (ou está muito próximo), avançar
    else { 
        // Snap para o Waypoint antes de mudar
        e->x = (int)target.x;
        e->y = (int)target.y;
        
        e->currentWaypoint++; // Vai para o próximo ponto
    }
}

void DrawEnemy(Enemy e) {
    if (!e.active) return;
    // Desenha o inimigo no centro de suas coordenadas
    DrawRectangle(e.x - 15, e.y - 15, 30, 30, RED); 
}

// Função para verificar se chegou ao fim do caminho (no último Waypoint)
int EnemyReachedTower(Enemy e) {
    // Se o índice do Waypoint atual for maior ou igual ao máximo, ele chegou
    return e.currentWaypoint >= MAX_WAYPOINTS;
}