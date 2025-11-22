#include "../include/recursos.h"

// Define o valor inicial de moedas
void inicializar_recursos(recursos *r) {
    r->moedas = 300;
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

// 💰 NOVA FUNÇÃO: Adiciona uma quantidade variável de moedas.
// Substitui a antiga 'inimigo_morto' e atende à necessidade em game.c.
void adicionar_moedas(recursos *r, int valor) {
    if (valor > 0) {
        r->moedas += valor;
    }
}