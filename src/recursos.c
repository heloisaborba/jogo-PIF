#include "recursos.h"

// Define o valor inicial de moedas
void inicializar_recursos(recursos *r) {
    r->moedas = 200;
}

// Retorna o total de moedas do jogador
int get_moedas(recursos *r) {
    return r->moedas;
}
