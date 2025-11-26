#include "enemy.h"
#include "game.h"    // Para acessar currentWave, pathInferior, pathSuperior, etc.
#include <stdlib.h>
#include <math.h>

// Tabela de estatísticas dos inimigos (deve corresponder ao enum EnemyType)
const EnemyConfig ENEMY_STATS[NUM_ENEMY_TYPES] = {
    // INIMIGO_GOBLIN - Pode ser atingido por TODOS EXCETO Paladino
    {
        .damage = 10,
        .speed = 2.0f,
        .maxHealth = 100,
        .recompensa = 10,
        .range = 50,
        .resistance = 0x04, // bit 2 ativado = Paladino (tipo 2) NÃO pode atacar
        .special_ability = false
    },
    // INIMIGO_SPECTRO - Só pode ser atingido pelo MAGO
    {
        .damage = 8,
        .speed = 2.5f,
        .maxHealth = 80,
        .recompensa = 15,
        .range = 60,
        .resistance = 0x07, // bits 0,1,2 ativados = Guerreiro, Bardo, Paladino resistem
        .special_ability = false
    },
    // INIMIGO_NECROMANTE - Só pode ser atingido pelo PALADINO
    {
        .damage = 15,
        .speed = 1.5f,
        .maxHealth = 150,
        .recompensa = 25,
        .range = 70,
        .resistance = 0x0B, // bits 0,1,3 ativados = Guerreiro, Bardo, Mago resistem (só Paladino ataca)
        .special_ability = true
    },
    // INIMIGO_DRAGAO - Pode ser atingido por TODOS EXCETO Paladino
    {
        .damage = 25,
        .speed = 1.8f,
        .maxHealth = 300,
        .recompensa = 50,
        .range = 80,
        .resistance = 0x04, // bit 2 ativado = Paladino (tipo 2) NÃO pode atacar
        .special_ability = true
    }
};

// Em enemy.c:
Vector2* GetEnemyCurrentPath(Enemy *e, int *pathLength) {
    extern Vector2 pathInferior[];
    extern Vector2 pathSuperior[];
    extern int currentWave;
    
    // ⭐️⭐️⭐️ IMPORTANTE: Fase 1 SEMPRE retorna caminho inferior
    if (currentWave == 1) {
        *pathLength = 84;
        return pathInferior; // ⭐️ SEMPRE inferior na fase 1
    } else {
        // Fase 2+: respeita o pathIndex
        if (e->pathIndex == 0) {
            *pathLength = 84;
            return pathInferior;
        } else {
            *pathLength = 96;
            return pathSuperior;
        }
    }
}

// ⭐️ NOVA FUNÇÃO: Obter waypoint final (torre) baseado no caminho
Vector2 GetEnemyFinalWaypoint(Enemy *e) {
    int pathLength;
    Vector2* path = GetEnemyCurrentPath(e, &pathLength);
    return path[pathLength - 1];
}

Enemy InitEnemy(float x, float y, EnemyType type) {
    Enemy enemy;
    
    // Inicialização básica
    enemy.active = true;
    enemy.type = type;
    enemy.x = x;
    enemy.y = y;
    enemy.currentWaypoint = 0;

    // ⭐️ INICIALIZAÇÃO DO CAMINHO
    enemy.pathIndex = 0;

    // Estatísticas base
    enemy.damage = ENEMY_STATS[type].damage;
    enemy.speed = ENEMY_STATS[type].speed;
    enemy.maxHealth = ENEMY_STATS[type].maxHealth;
    enemy.health = enemy.maxHealth;
    enemy.recompensa_moedas = ENEMY_STATS[type].recompensa;
    enemy.range = ENEMY_STATS[type].range;
    enemy.resistance = ENEMY_STATS[type].resistance;

    // ====== AUMENTO DE VELOCIDADE POR FASE ======
    if (currentWave == 2) {
        enemy.speed *= 1.35f;   // +35% mais rápido
    }
    else if (currentWave == 3) {
        enemy.speed *= 1.85f;   // +75% mais rápido
    }
    // ============================================

    // ====== AUMENTO DE DANO POR FASE ======
    if (currentWave == 2) {
        enemy.damage *= 1.40f;  // +40% dano
    }
    else if (currentWave == 3) {
        enemy.damage *= 1.80f;  // +80% dano
    }
    // ======================================

    // Status especiais
    enemy.is_burning = false;
    enemy.burning_timer = 0.0f;
    enemy.necromante_heal_timer = 0.0f;

    return enemy;
}


void UpdateEnemy(Enemy *e) {
    if (!e->active) return;

    // ⭐️ ATUALIZADO: Obter caminho específico deste inimigo
    int pathLength;
    Vector2* currentPath = GetEnemyCurrentPath(e, &pathLength);
    
    // Movimento pelos waypoints
    if (e->currentWaypoint < pathLength - 1) {
        Vector2 target = currentPath[e->currentWaypoint + 1];
        Vector2 direction = {
            target.x - e->x,
            target.y - e->y
        };
        
        float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
        
        if (distance < e->speed) {
            // Chegou no waypoint
            e->currentWaypoint++;
            e->x = target.x;
            e->y = target.y;
        } else {
            // Move em direção ao waypoint
            direction.x /= distance;
            direction.y /= distance;
            
            e->x += direction.x * e->speed;
            e->y += direction.y * e->speed;
        }
    }
    
    // Atualiza status especiais
    if (e->is_burning) {
        e->burning_timer -= GetFrameTime();
        if (e->burning_timer <= 0) {
            e->is_burning = false;
        }
    }
    
    if (e->type == INIMIGO_NECROMANTE) {
        e->necromante_heal_timer -= GetFrameTime();
    }
}

void DrawEnemy(Enemy e) {
    if (!e.active) return;
    
    Color color;
    switch(e.type) {
        case INIMIGO_GOBLIN: color = GREEN; break;
        case INIMIGO_SPECTRO: color = GRAY; break;
        case INIMIGO_NECROMANTE: color = PURPLE; break;
        case INIMIGO_DRAGAO: color = RED; break;
        default: color = WHITE;
    }
    
    // ⭐️ ADICIONADO: Indicador visual do caminho
    if (currentWave > 1) {
        // Desenha pequeno indicador do caminho
        Color pathColor = (e.pathIndex == 0) ? BLUE : SKYBLUE;
        DrawCircle(e.x, e.y - 20, 3, pathColor);
    }
    
    // Desenha inimigo como círculo (substitua por sprites depois)
    DrawCircle(e.x, e.y, 15, color);
    
    // Desenha barra de vida
    float healthPercent = (float)e.health / (float)e.maxHealth;
    DrawRectangle(e.x - 20, e.y - 25, 40, 5, GRAY);
    DrawRectangle(e.x - 20, e.y - 25, 40 * healthPercent, 5, GREEN);
    
    // Indicador de queima
    if (e.is_burning) {
        DrawCircle(e.x + 15, e.y - 15, 5, ORANGE);
    }
}

int EnemyReachedTower(Enemy e) {
    // ⭐️ ATUALIZADO: Verifica se chegou no último waypoint do seu caminho específico
    int pathLength;
    Vector2* currentPath = GetEnemyCurrentPath((Enemy*)&e, &pathLength);
    
    Vector2 finalWaypoint = currentPath[pathLength - 1];
    float distanceToFinal = Vector2Distance((Vector2){e.x, e.y}, finalWaypoint);
    
    return (e.currentWaypoint >= pathLength - 1) || 
           (distanceToFinal < 20.0f);
}

// ⭐️ NOVA FUNÇÃO: Para debug - mostra informações do caminho do inimigo
void DrawEnemyDebugInfo(Enemy e, int index) {
    if (!e.active) return;
    
    const char* pathName = (e.pathIndex == 0) ? "INFERIOR" : "SUPERIOR";
    int pathLength;
    
    DrawText(TextFormat("Inimigo %d: %s (WP: %d/%d)", 
                       index, pathName, e.currentWaypoint, pathLength - 1),
            10, 150 + (index * 15), 10, WHITE);
}