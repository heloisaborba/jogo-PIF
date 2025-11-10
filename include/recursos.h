#ifndef recursos_h
#define recursos_h

typedef struct {
    int moedas;
} recursos;

// Inicializa os recursos com o valor inicial (200 moedas)
void inicializar_recursos(recursos *r);

// Retorna a quantidade atual de moedas
int get_moedas(recursos *r);

#endif
