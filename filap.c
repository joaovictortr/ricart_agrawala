#include <stdio.h>
#include <malloc.h>
#include "filap.h"

void fixup_heap(filap fila);
void fixdown_heap(filap fila, int k);

int rotulo_menor(item_fila a, item_fila b)
{
    return a->pri < b->pri;
}

filap cria_filap(int capacidade)
{
    filap nova_filap = malloc(sizeof(struct fila_prioridade));
    if (nova_filap == NULL)
        return NULL;

    nova_filap->tamanho = 0;
    nova_filap->capacidade = capacidade;
    nova_filap->conteudo = malloc(sizeof(struct item_fila) * (capacidade+1));

    if (nova_filap->conteudo == NULL) {
        free(nova_filap);
        return NULL;
    }

    return nova_filap;
}

int destroi_filap(filap fila)
{
    if (fila == NULL) return 0;

    if (fila->conteudo != NULL) {
        free(fila->conteudo);
    } else {
        fprintf(stderr, "[%s:%d] Conteúdo da fila de prioridades não estava alocado!\n", __FUNCTION__, __LINE__);
    }
    free(fila);

    return 1;
}

int vazia_filap(filap fila)
{
    return fila->tamanho == 0;
}

/* Conserta o heap de baixo para cima
 */
void fixup_heap(filap fila)
{
    int k = fila->tamanho;
    int pai = k / 2;
    while(k > 1 && rotulo_menor(fila->conteudo[pai], fila->conteudo[k])) {
        // rotulo(pai(k) < k), então troca(pai(k), k)
        item_fila t = fila->conteudo[pai];
        fila->conteudo[pai] = fila->conteudo[k];
        fila->conteudo[k] = t;
        // sobe um nível na árvore porque k foi trocado com pai(k)
        k = pai;
        // atualiza o nodo pai de k
        pai = k/2;
    }
}

/**
 * Conserta o heap de cima para baixo a partir de k
 */
void fixdown_heap(filap fila, int k)
{
    int j, n = fila->tamanho-1;

    while(2*k <= n) { // enquanto esq(k) estiver contido no heap
        j = 2*k; // esq(k)

        // j está dentro do heap e rotulo(esq(k)) = rotulo(j) < rotulo(j+1) = rotulo(dir(k)),
        // isto é, o rótulo do filho de esquerdo de k é menor que o do direito (propriedade
        // max-heap não violada), então passamos ao filho direito de k (j = j+1 = dir(k))
        if (j < n && rotulo_menor(fila->conteudo[j], fila->conteudo[j+1]))
            j++;
        // se j = esq(k)
        //   se rotulo(k) > rotulo(esq(k))
        //     propriedade max-heap mantida, chave do pai é maior que a do filho esquerdo
        // senao (se j = dir(k))
        //   se rotulo(k) > rotulo(dir(k))
        //     propriedade max-heap mantida, chave do pai é maior que a do filho direito
        if (!rotulo_menor(fila->conteudo[k], fila->conteudo[j]))
            break;

        // propriedade max-heap violada, nodo com chave k tem que ser demovido do heap,
        // ou seja, k vira filho e o filho de k (seja esquerdo ou direito, dependendo de j)
        // vira pai:
        //     troca(k, j)
        //  se j = esq(k)
        //     troca(k, esq(k))
        //  se j = dir(k)
        //     troca(k, dir(k))
        item_fila t = fila->conteudo[k];
        fila->conteudo[k] = fila->conteudo[j];
        fila->conteudo[j] = t;
        k = j;
    }
}

int insere_filap(filap f, int pid, int timestamp, int pri, int tipo)
{
    if (f->tamanho == f->capacidade) {
        // capacidade da fila foi esgotada, não é possível inserir
        fprintf(stderr, "Capacidade da fila de prioridades esgotada!\n");
        return 0;
    }

    item_fila novo_item = malloc(sizeof(struct item_fila));

    novo_item->pid = pid;
    novo_item->timestamp = timestamp;
    novo_item->pri = pri;
    novo_item->tipo = tipo;

    f->conteudo[++f->tamanho] = novo_item;
    fixup_heap(f);
    return 1;
}


item_fila remove_max_filap(filap f)
{
    if (f == NULL) return NULL;

    if (vazia_filap(f)) return NULL;

    item_fila t = f->conteudo[1];
    f->conteudo[1] = f->conteudo[f->tamanho];
    f->conteudo[f->tamanho] = t;

    fixdown_heap(f, 1);
    return f->conteudo[f->tamanho--];
}
