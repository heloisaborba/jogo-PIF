#ifndef recursos_h
#define recursos_h

// Estrutura que representa os recursos do jogador
typedef struct {
    int moedas;
} recursos;

// Inicializa os recursos com o valor inicial (200 moedas)
void inicializar_recursos(recursos *r);

// Retorna a quantidade atual de moedas
int get_moedas(recursos *r);

// Tenta comprar um herói (custa 100 moedas)
// Retorna 1 se a compra for bem-sucedida, 0 caso contrário
int comprar_heroi(recursos *r);

#endif
