/*
 * ============================================================
 *  Detective Quest — Nível Novato
 *  Mapa da Mansão com Árvore Binária
 * ============================================================
 *  Autor  : Paulo Giovani dos Santos - Enigma Studios
 *  Versão : 1.0
 *
 *  Descrição:
 *    Representa o mapa de uma mansão como uma árvore binária.
 *    Cada nó é uma sala (cômodo). O jogador explora a mansão
 *    escolhendo ir à esquerda (e) ou à direita (d) a partir
 *    do Hall de Entrada, até atingir um nó-folha.
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Constantes ─────────────────────────────────────────── */
#define MAX_NOME 64

/* ─── Estrutura de dados ──────────────────────────────────── */

/**
 * Sala — nó da árvore binária.
 *
 * @campo nome      Nome descritivo do cômodo.
 * @campo esquerda  Ponteiro para o cômodo à esquerda.
 * @campo direita   Ponteiro para o cômodo à direita.
 */
typedef struct Sala {
    char         nome[MAX_NOME];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

/* ─── Protótipos ──────────────────────────────────────────── */
Sala *criarSala(const char *nome);
void  explorarSalas(Sala *atual);
void  liberarArvore(Sala *raiz);

/* ================================================================
 *  criarSala
 *  ---------
 *  Aloca dinamicamente um novo nó (sala) e inicializa seus campos.
 *
 *  @param nome  String com o nome do cômodo.
 *  @return      Ponteiro para a sala criada, ou NULL em falha.
 * ================================================================ */
Sala *criarSala(const char *nome) {
    Sala *nova = (Sala *)malloc(sizeof(Sala));
    if (!nova) {
        fprintf(stderr, "[ERRO] Falha ao alocar memória para sala '%s'.\n", nome);
        return NULL;
    }
    strncpy(nova->nome, nome, MAX_NOME - 1);
    nova->nome[MAX_NOME - 1] = '\0'; /* garante terminador nulo */
    nova->esquerda = NULL;
    nova->direita  = NULL;
    return nova;
}

/* ================================================================
 *  explorarSalas
 *  -------------
 *  Permite ao jogador navegar pela árvore binária interativamente.
 *  A cada sala, o jogador escolhe:
 *    e → ir para o cômodo à esquerda
 *    d → ir para o cômodo à direita
 *    s → encerrar a exploração
 *  A função termina ao atingir um nó-folha (sem filhos).
 *
 *  @param atual  Ponteiro para a sala corrente.
 * ================================================================ */
void explorarSalas(Sala *atual) {
    if (!atual) return;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("  🕵️Você está em: %-18s║\n", atual->nome);
    printf("╚══════════════════════════════════════╝\n");

    /* Nó-folha: sem caminhos disponíveis */
    if (!atual->esquerda && !atual->direita) {
        printf("  ➤ Beco sem saída! Não há mais caminhos a explorar.\n");
        printf("  ➤ Sua exploração terminou aqui.\n");
        return;
    }

    /* Exibe as opções disponíveis conforme os filhos existentes */
    printf("  Caminhos disponíveis:\n");
    if (atual->esquerda)
        printf("    [e] Ir para a ESQUERDA → %s\n", atual->esquerda->nome);
    if (atual->direita)
        printf("    [d] Ir para a DIREITA  → %s\n", atual->direita->nome);
    printf("    [s] Sair da exploração\n");
    printf("  Sua escolha: ");

    char opcao;
    scanf(" %c", &opcao);   /* espaço antes de %c descarta '\n' residual */

    switch (opcao) {
        case 'e':
        case 'E':
            if (atual->esquerda)
                explorarSalas(atual->esquerda);
            else
                printf("  ⚠️  Não há caminho à esquerda aqui!\n");
            break;

        case 'd':
        case 'D':
            if (atual->direita)
                explorarSalas(atual->direita);
            else
                printf("  ⚠️  Não há caminho à direita aqui!\n");
            break;

        case 's':
        case 'S':
            printf("\n  🚪 Você decidiu encerrar a exploração. Até a próxima!\n");
            break;

        default:
            printf("  ❌ Opção inválida. Tente novamente.\n");
            explorarSalas(atual); /* repete a sala atual */
    }
}

/* ================================================================
 *  liberarArvore
 *  -------------
 *  Percorre a árvore em pós-ordem e libera toda memória alocada.
 *
 *  @param raiz  Raiz (ou sub-raiz) da árvore a ser liberada.
 * ================================================================ */
void liberarArvore(Sala *raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz);
}

/* ================================================================
 *  main
 *  ----
 *  Monta o mapa da mansão como uma árvore binária estática
 *  e inicia a exploração a partir do Hall de Entrada.
 *
 *  Layout da mansão:
 *
 *                   Hall de Entrada
 *                  /               \
 *           Sala de Estar        Biblioteca
 *           /         \          /        \
 *        Cozinha    Jardim   Escritório  Porão
 *        /    \
 *    Despensa  Varanda
 * ================================================================ */
int main(void) {
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║       🔍 DETECTIVE QUEST — Nível Novato   ║\n");
    printf("║         Exploração da Mansão Misteriosa   ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    printf("\n  Construindo o mapa da mansão...\n");

    /* ── Nível 0 (raiz) ── */
    Sala *raiz = criarSala("Hall de Entrada");

    /* ── Nível 1 ── */
    raiz->esquerda = criarSala("Sala de Estar");
    raiz->direita  = criarSala("Biblioteca");

    /* ── Nível 2 — ramo esquerdo ── */
    raiz->esquerda->esquerda = criarSala("Cozinha");
    raiz->esquerda->direita  = criarSala("Jardim");

    /* ── Nível 2 — ramo direito ── */
    raiz->direita->esquerda = criarSala("Escritório");
    raiz->direita->direita  = criarSala("Porão");

    /* ── Nível 3 — folhas do ramo Cozinha ── */
    raiz->esquerda->esquerda->esquerda = criarSala("Despensa");
    raiz->esquerda->esquerda->direita  = criarSala("Varanda");

    printf("  Mapa carregado! Boa investigação, detetive.\n");

    /* Inicia a exploração a partir da raiz */
    explorarSalas(raiz);

    /* Libera toda a memória alocada */
    liberarArvore(raiz);

    printf("\n  🏁 Programa encerrado. Até a próxima aventura!\n\n");
    return 0;
}
