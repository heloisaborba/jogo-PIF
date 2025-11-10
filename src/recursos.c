#include "../include/recursos.h"

// Define o valor inicial de moedas
void inicializar_recursos(recursos *r) {
    r->moedas = 200;
}

// Retorna o total de moedas do jogador
int get_moedas(recursos *r) {
    return r->moedas;
}

// Compra de herói: custa 100 moedas se houver saldo suficiente
int comprar_heroi(recursos *r) {
    if (r->moedas >= 100) {
        r->moedas -= 100;
        return 1; // compra realizada
    }
    return 0; // moedas insuficientes
}

// Recompensa por inimigo derrotado: adiciona 5 moedas
void inimigo_morto(recursos *r) {
    r->moedas += 5;
}
