/*
 * ============================================================
 *  Detective Quest — Nível Aventureiro
 *  Mapa da Mansão + Organização de Pistas com BST
 * ============================================================
 *  Autor  : Paulo Giovani dos Santos - Enigma Studios
 *  Versão : 2.0  (estende o Nível Novato)
 *
 *  Novidades em relação ao Nível Novato:
 *    • Algumas salas possuem pistas associadas.
 *    • Ao visitar uma sala com pista, ela é inserida numa
 *      Árvore Binária de Busca (BST) de strings.
 *    • Ao sair, o jogador pode listar todas as pistas
 *      encontradas em ordem alfabética.
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Constantes ─────────────────────────────────────────── */
#define MAX_NOME  64
#define MAX_PISTA 128

/* ================================================================
 *  MÓDULO 1 — Árvore Binária de Navegação (Mapa da Mansão)
 * ================================================================ */

/**
 * Sala — nó da árvore binária do mapa.
 *
 * @campo nome      Nome do cômodo.
 * @campo pista     Pista encontrada na sala (vazio = sem pista).
 * @campo esquerda  Caminho à esquerda.
 * @campo direita   Caminho à direita.
 */
typedef struct Sala {
    char         nome[MAX_NOME];
    char         pista[MAX_PISTA]; /* "" significa sem pista */
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

/* ================================================================
 *  MÓDULO 2 — Árvore Binária de Busca (BST) de Pistas
 * ================================================================ */

/**
 * NoPista — nó da BST que armazena as pistas coletadas.
 *
 * @campo pista     String com a pista.
 * @campo esquerda  Sub-árvore com pistas lexicograficamente menores.
 * @campo direita   Sub-árvore com pistas lexicograficamente maiores.
 */
typedef struct NoPista {
    char           pista[MAX_PISTA];
    struct NoPista *esquerda;
    struct NoPista *direita;
} NoPista;

/* ─── Protótipos ──────────────────────────────────────────── */

/* Mapa */
Sala   *criarSala(const char *nome, const char *pista);
void    explorarSalas(Sala *atual, NoPista **bst);
void    liberarArvore(Sala *raiz);

/* BST de pistas */
NoPista *inserirPista(NoPista *raiz, const char *pista);
void     emOrdem(NoPista *raiz);
void     liberarBST(NoPista *raiz);

/* ================================================================
 *  criarSala
 *  ---------
 *  Aloca um novo nó do mapa com nome e pista associada.
 *
 *  @param nome   Nome do cômodo.
 *  @param pista  Pista da sala ("" para nenhuma).
 *  @return       Ponteiro para a Sala criada.
 * ================================================================ */
Sala *criarSala(const char *nome, const char *pista) {
    Sala *nova = (Sala *)malloc(sizeof(Sala));
    if (!nova) {
        fprintf(stderr, "[ERRO] Falha ao alocar memória para sala '%s'.\n", nome);
        exit(EXIT_FAILURE);
    }
    strncpy(nova->nome,  nome,  MAX_NOME  - 1);
    strncpy(nova->pista, pista, MAX_PISTA - 1);
    nova->nome[MAX_NOME   - 1] = '\0';
    nova->pista[MAX_PISTA - 1] = '\0';
    nova->esquerda = NULL;
    nova->direita  = NULL;
    return nova;
}

/* ================================================================
 *  inserirPista
 *  ------------
 *  Insere uma string na BST de pistas mantendo a ordem alfabética.
 *  Pistas duplicadas são ignoradas.
 *
 *  @param raiz   Raiz atual da BST.
 *  @param pista  String a inserir.
 *  @return       Nova raiz da BST (pode mudar apenas na 1ª inserção).
 * ================================================================ */
NoPista *inserirPista(NoPista *raiz, const char *pista) {
    if (!raiz) {
        /* Cria novo nó */
        NoPista *novo = (NoPista *)malloc(sizeof(NoPista));
        if (!novo) {
            fprintf(stderr, "[ERRO] Falha ao alocar memória para pista.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(novo->pista, pista, MAX_PISTA - 1);
        novo->pista[MAX_PISTA - 1] = '\0';
        novo->esquerda = NULL;
        novo->direita  = NULL;
        return novo;
    }

    int cmp = strcmp(pista, raiz->pista);

    if (cmp < 0)
        raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0)
        raiz->direita  = inserirPista(raiz->direita,  pista);
    /* cmp == 0: pista duplicada, ignora */

    return raiz;
}

/* ================================================================
 *  emOrdem
 *  -------
 *  Percorre a BST em ordem simétrica (in-order) e imprime
 *  todas as pistas em ordem alfabética.
 *
 *  @param raiz  Raiz (ou sub-raiz) da BST.
 * ================================================================ */
void emOrdem(NoPista *raiz) {
    if (!raiz) return;
    emOrdem(raiz->esquerda);
    printf("    📄 %s\n", raiz->pista);
    emOrdem(raiz->direita);
}

/* ================================================================
 *  explorarSalas
 *  -------------
 *  Navega pela árvore do mapa; ao entrar numa sala com pista,
 *  insere a pista na BST automaticamente.
 *
 *  @param atual  Sala corrente do mapa.
 *  @param bst    Ponteiro para a raiz da BST de pistas.
 * ================================================================ */
void explorarSalas(Sala *atual, NoPista **bst) {
    if (!atual) return;

    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  🕵️  Você está em: %-21s║\n", atual->nome);
    printf("╚══════════════════════════════════════════╝\n");

    /* Verifica e registra pista da sala */
    if (strlen(atual->pista) > 0) {
        printf("  🔎 Pista encontrada: \"%s\"\n", atual->pista);
        *bst = inserirPista(*bst, atual->pista);
        printf("  ✅ Pista adicionada ao seu caderno!\n");
    }

    /* Nó-folha */
    if (!atual->esquerda && !atual->direita) {
        printf("  ➤ Beco sem saída! Não há mais caminhos a explorar.\n");
        return;
    }

    /* Opções de navegação */
    printf("  Caminhos disponíveis:\n");
    if (atual->esquerda)
        printf("    [e] Ir para a ESQUERDA → %s\n", atual->esquerda->nome);
    if (atual->direita)
        printf("    [d] Ir para a DIREITA  → %s\n", atual->direita->nome);
    printf("    [s] Sair da exploração\n");
    printf("  Sua escolha: ");

    char opcao;
    scanf(" %c", &opcao);

    switch (opcao) {
        case 'e': case 'E':
            if (atual->esquerda)
                explorarSalas(atual->esquerda, bst);
            else
                printf("  ⚠️  Não há caminho à esquerda!\n");
            break;

        case 'd': case 'D':
            if (atual->direita)
                explorarSalas(atual->direita, bst);
            else
                printf("  ⚠️  Não há caminho à direita!\n");
            break;

        case 's': case 'S':
            printf("\n  🚪 Você encerrou a exploração.\n");
            break;

        default:
            printf("  ❌ Opção inválida.\n");
            explorarSalas(atual, bst);
    }
}

/* ================================================================
 *  liberarArvore / liberarBST
 *  --------------------------
 *  Liberam, respectivamente, o mapa e a BST de pistas (pós-ordem).
 * ================================================================ */
void liberarArvore(Sala *raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz);
}

void liberarBST(NoPista *raiz) {
    if (!raiz) return;
    liberarBST(raiz->esquerda);
    liberarBST(raiz->direita);
    free(raiz);
}

/* ================================================================
 *  main
 *  ----
 *  Monta o mapa com pistas em certas salas e inicia o jogo.
 *
 *  Layout:
 *                   Hall de Entrada
 *                  /               \
 *           Sala de Estar        Biblioteca  [luva ensanguentada]
 *           /         \          /        \
 *       Cozinha      Jardim  Escritório  Porão [carta anônima]
 *      [faca]      [pegadas]  [veneno]
 *        /    \
 *   Despensa  Varanda
 *  [ricino]
 * ================================================================ */
int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   🔍 DETECTIVE QUEST — Nível Aventureiro      ║\n");
    printf("║      Mansão + Caderno de Pistas (BST)         ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /* ── Mapa da mansão (2º parâmetro = pista da sala) ── */
    Sala *raiz = criarSala("Hall de Entrada", "");

    raiz->esquerda = criarSala("Sala de Estar", "");
    raiz->direita  = criarSala("Biblioteca",    "Luva ensanguentada");

    raiz->esquerda->esquerda = criarSala("Cozinha", "Faca com impressão digital");
    raiz->esquerda->direita  = criarSala("Jardim",  "Pegadas na lama");

    raiz->direita->esquerda = criarSala("Escritório", "Frasco de veneno");
    raiz->direita->direita  = criarSala("Porão",      "Carta anônima");

    raiz->esquerda->esquerda->esquerda = criarSala("Despensa", "Extrato de ricino");
    raiz->esquerda->esquerda->direita  = criarSala("Varanda",  "");

    /* ── BST de pistas (inicialmente vazia) ── */
    NoPista *bst = NULL;

    /* ── Exploração ── */
    explorarSalas(raiz, &bst);

    /* ── Relatório de pistas coletadas ── */
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║        📓 CADERNO DE PISTAS COLETADAS        ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    if (!bst) {
        printf("  Nenhuma pista encontrada nesta exploração.\n");
    } else {
        printf("  Pistas em ordem alfabética:\n");
        emOrdem(bst);
    }

    /* ── Limpeza de memória ── */
    liberarArvore(raiz);
    liberarBST(bst);

    printf("\n  🏁 Investigação encerrada. Bom trabalho, detetive!\n\n");
    return 0;
}
