#ifndef __FILAP_H__
#define __FILAP_H_
struct item_fila {
    int pri;
    int pid;
    int timestamp;
    int tipo;
};
typedef struct item_fila *item_fila;

struct fila_prioridade {
    int tamanho;
    int capacidade;
    item_fila *conteudo;
};

typedef struct fila_prioridade* filap;

int rotulo_menor(item_fila a, item_fila b);
filap cria_filap(int tamanho);
int vazia_filap();
int insere_filap(filap f, int pid, int timestamp, int pri, int tipo);
item_fila remove_max_filap(filap f);
int destroi_filap(filap f);
#endif
