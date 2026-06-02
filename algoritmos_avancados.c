/*
 * ============================================================
 *  Detective Quest — Nível Mestre
 *  Mapa + BST de Pistas + Tabela Hash (Pista → Suspeito)
 * ============================================================
 *  Autor  : Paulo Giovani dos Santos - Enigma Studios
 *  Versão : 4.0  (estende os Níveis Novato, Aventureiro e Mestre 3.0)
 *
 *  Novidades em relação à Versão 3.0:
 *    • Função encontrarSuspeito() — consulta pista → suspeito na Hash.
 *    • Função verificarSuspeitoFinal() — fase de julgamento interativa:
 *        o jogador acusa um suspeito e o sistema verifica se há pelo
 *        menos duas pistas que apontem para ele.
 *    • Ao final da exploração, o jogo entra automaticamente na fase
 *      de julgamento antes de exibir o relatório completo.
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── Constantes ─────────────────────────────────────────── */
#define MAX_NOME      64
#define MAX_PISTA    128
#define MAX_SUSPEITO  64
#define TAM_HASH      17   /* tamanho primo → menos colisões   */

/* ================================================================
 *  MÓDULO 1 — Árvore Binária de Navegação (Mapa da Mansão)
 * ================================================================ */

/**
 * Sala — nó da árvore binária do mapa.
 *
 * @campo nome       Nome identificador do cômodo.
 * @campo pista      Pista estática associada ao cômodo (pode ser "").
 * @campo suspeito   Suspeito vinculado à pista deste cômodo.
 * @campo esquerda   Ponteiro para o cômodo à esquerda na árvore.
 * @campo direita    Ponteiro para o cômodo à direita na árvore.
 */
typedef struct Sala {
    char         nome[MAX_NOME];
    char         pista[MAX_PISTA];
    char         suspeito[MAX_SUSPEITO];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

/* ================================================================
 *  MÓDULO 2 — Árvore Binária de Busca (BST) de Pistas
 * ================================================================ */

/**
 * NoPista — nó da BST de pistas coletadas.
 *
 * @campo pista      String da pista coletada (chave de ordenação).
 * @campo esquerda   Subárvore com pistas lexicograficamente menores.
 * @campo direita    Subárvore com pistas lexicograficamente maiores.
 */
typedef struct NoPista {
    char           pista[MAX_PISTA];
    struct NoPista *esquerda;
    struct NoPista *direita;
} NoPista;

/* ================================================================
 *  MÓDULO 3 — Tabela Hash (Pista → Suspeito)
 *
 *  Estratégia de colisão: encadeamento separado (linked list).
 *  Cada bucket armazena uma lista de EntradaHash.
 * ================================================================ */

/**
 * EntradaHash — célula da lista encadeada dentro de um bucket.
 *
 * @campo pista     Chave de busca.
 * @campo suspeito  Valor associado.
 * @campo prox      Próximo elemento na lista (colisão).
 */
typedef struct EntradaHash {
    char              pista[MAX_PISTA];
    char              suspeito[MAX_SUSPEITO];
    struct EntradaHash *prox;
} EntradaHash;

/** Tabela Hash: vetor de ponteiros para listas encadeadas. */
typedef EntradaHash *TabelaHash[TAM_HASH];

/* ─── Protótipos ──────────────────────────────────────────── */

/* Mapa */
Sala   *criarSala(const char *nome, const char *pista, const char *suspeito);
void    explorarSalas(Sala *atual, NoPista **bst, TabelaHash tabela);
void    liberarArvore(Sala *raiz);

/* BST */
NoPista *inserirPista(NoPista *raiz, const char *pista);
void     emOrdem(NoPista *raiz);
void     liberarBST(NoPista *raiz);

/* Hash */
unsigned int funcaoHash(const char *chave);
void         inserirNaHash(TabelaHash tabela, const char *pista, const char *suspeito);
const char  *encontrarSuspeito(TabelaHash tabela, const char *pista);
void         exibirHash(TabelaHash tabela);
void         suspeitorMaisRelatado(TabelaHash tabela);
void         verificarSuspeitoFinal(TabelaHash tabela);
void         liberarHash(TabelaHash tabela);

/* Auxiliar */
int          contarPistasPorSuspeito(TabelaHash tabela, const char *suspeito);

/* ================================================================
 *  funcaoHash
 *  ----------
 *  Soma os valores ASCII de todos os caracteres da chave e
 *  retorna o índice (módulo TAM_HASH).
 *
 *  @param chave  String usada como chave.
 *  @return       Índice no vetor [0, TAM_HASH).
 * ================================================================ */
unsigned int funcaoHash(const char *chave) {
    unsigned int soma = 0;
    while (*chave) {
        soma += (unsigned char)*chave;
        chave++;
    }
    return soma % TAM_HASH;
}

/* ================================================================
 *  inserirNaHash
 *  -------------
 *  Insere o par (pista → suspeito) na tabela hash.
 *  Se a pista já existir no bucket, atualiza o suspeito.
 *  Caso contrário, aloca uma nova EntradaHash e a insere no
 *  início da lista encadeada do bucket (custo O(1)).
 *
 *  Estratégia de colisão: encadeamento separado (chaining).
 *  A função de hash soma os valores ASCII da chave e aplica
 *  módulo TAM_HASH para determinar o bucket.
 *
 *  @param tabela    A tabela hash (vetor de listas encadeadas).
 *  @param pista     Chave de busca (string da pista).
 *  @param suspeito  Valor associado (nome do suspeito).
 * ================================================================ */
void inserirNaHash(TabelaHash tabela, const char *pista, const char *suspeito) {
    unsigned int idx = funcaoHash(pista);
    EntradaHash *atual = tabela[idx];

    /* Verifica se a chave já existe no bucket — atualiza se sim */
    while (atual) {
        if (strcmp(atual->pista, pista) == 0) {
            strncpy(atual->suspeito, suspeito, MAX_SUSPEITO - 1);
            atual->suspeito[MAX_SUSPEITO - 1] = '\0';
            return;
        }
        atual = atual->prox;
    }

    /* Cria nova entrada e insere no início da lista (O(1)) */
    EntradaHash *nova = (EntradaHash *)malloc(sizeof(EntradaHash));
    if (!nova) {
        fprintf(stderr, "[ERRO] Falha ao alocar entrada na tabela hash.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nova->pista,    pista,    MAX_PISTA    - 1);
    strncpy(nova->suspeito, suspeito, MAX_SUSPEITO - 1);
    nova->pista[MAX_PISTA       - 1] = '\0';
    nova->suspeito[MAX_SUSPEITO - 1] = '\0';
    nova->prox    = tabela[idx];
    tabela[idx]   = nova;
}

/* ================================================================
 *  encontrarSuspeito
 *  -----------------
 *  Consulta a tabela hash e retorna o suspeito associado a uma
 *  pista específica. Percorre a lista encadeada do bucket
 *  correspondente até encontrar a chave ou esgotar a lista.
 *
 *  Complexidade média: O(1) — depende da distribuição da hash.
 *  Pior caso (todas as chaves no mesmo bucket): O(n).
 *
 *  @param tabela  A tabela hash (vetor de listas encadeadas).
 *  @param pista   Chave de busca (string da pista coletada).
 *  @return        Ponteiro para a string do suspeito, ou NULL
 *                 se a pista não estiver registrada na tabela.
 * ================================================================ */
const char *encontrarSuspeito(TabelaHash tabela, const char *pista) {
    unsigned int idx   = funcaoHash(pista);
    EntradaHash *atual = tabela[idx];
    while (atual) {
        if (strcmp(atual->pista, pista) == 0)
            return atual->suspeito;
        atual = atual->prox;
    }
    return NULL;   /* pista não encontrada */
}

/* ================================================================
 *  exibirHash
 *  ----------
 *  Percorre todos os buckets e imprime as associações pista→suspeito.
 *
 *  @param tabela  A tabela hash.
 * ================================================================ */
void exibirHash(TabelaHash tabela) {
    int encontrou = 0;
    for (int i = 0; i < TAM_HASH; i++) {
        EntradaHash *atual = tabela[i];
        while (atual) {
            printf("    🔗 %-35s → %s\n", atual->pista, atual->suspeito);
            encontrou = 1;
            atual = atual->prox;
        }
    }
    if (!encontrou)
        printf("    Nenhuma associação registrada.\n");
}

/* ================================================================
 *  contarPistasPorSuspeito
 *  -----------------------
 *  Função auxiliar recursiva — percorre toda a tabela hash e
 *  conta quantas pistas estão associadas ao suspeito informado.
 *  Usa comparação case-sensitive entre os nomes.
 *
 *  Usada internamente por verificarSuspeitoFinal() e
 *  suspeitorMaisRelatado() para calcular o total de evidências.
 *
 *  @param tabela    A tabela hash.
 *  @param suspeito  Nome do suspeito a ser contado.
 *  @return          Número de pistas que apontam para esse suspeito.
 * ================================================================ */
int contarPistasPorSuspeito(TabelaHash tabela, const char *suspeito) {
    int total = 0;
    for (int i = 0; i < TAM_HASH; i++) {
        EntradaHash *atual = tabela[i];
        while (atual) {
            if (strcmp(atual->suspeito, suspeito) == 0)
                total++;
            atual = atual->prox;
        }
    }
    return total;
}

/* ================================================================
 *  suspeitorMaisRelatado
 *  ---------------------
 *  Conta quantas pistas cada suspeito acumulou e exibe o campeão.
 *  Usa um array auxiliar de contagem (máximo 16 suspeitos distintos).
 *
 *  @param tabela  A tabela hash.
 * ================================================================ */
void suspeitorMaisRelatado(TabelaHash tabela) {
#define MAX_SUSPEITOS_DISTINTOS 16
    char  nomes[MAX_SUSPEITOS_DISTINTOS][MAX_SUSPEITO];
    int   contagem[MAX_SUSPEITOS_DISTINTOS];
    int   total = 0;

    memset(contagem, 0, sizeof(contagem));

    /* Varre todos os buckets */
    for (int i = 0; i < TAM_HASH; i++) {
        EntradaHash *atual = tabela[i];
        while (atual) {
            int encontrado = 0;
            for (int j = 0; j < total; j++) {
                if (strcmp(nomes[j], atual->suspeito) == 0) {
                    contagem[j]++;
                    encontrado = 1;
                    break;
                }
            }
            if (!encontrado && total < MAX_SUSPEITOS_DISTINTOS) {
                strncpy(nomes[total], atual->suspeito, MAX_SUSPEITO - 1);
                nomes[total][MAX_SUSPEITO - 1] = '\0';
                contagem[total] = 1;
                total++;
            }
            atual = atual->prox;
        }
    }

    if (total == 0) {
        printf("  Nenhum suspeito registrado.\n");
        return;
    }

    /* Encontra o máximo */
    int maiorIdx = 0;
    for (int j = 1; j < total; j++) {
        if (contagem[j] > contagem[maiorIdx])
            maiorIdx = j;
    }

    /* Exibe ranking */
    printf("  Suspeitos e suas pistas:\n");
    for (int j = 0; j < total; j++) {
        printf("    %-25s : %d pista(s)\n", nomes[j], contagem[j]);
    }

    printf("\n  🏆 Suspeito mais relatado: %s (%d pista(s))\n",
           nomes[maiorIdx], contagem[maiorIdx]);
}

/* ================================================================
 *  verificarSuspeitoFinal
 *  ----------------------
 *  Conduz a fase de julgamento final do Detective Quest.
 *
 *  Fluxo:
 *    1. Exibe todos os suspeitos registrados na tabela hash,
 *       listando quantas pistas cada um acumulou.
 *    2. Solicita ao jogador que digite o nome do suspeito acusado.
 *    3. Usa contarPistasPorSuspeito() para verificar o total de
 *       evidências contra o acusado na tabela hash.
 *    4. Se o acusado tiver pelo menos 2 pistas → CULPADO CONFIRMADO.
 *       Caso contrário → ACUSAÇÃO REJEITADA por falta de evidências.
 *
 *  Critério de condenação: mínimo de 2 pistas associadas ao suspeito.
 *  O nome informado deve corresponder exatamente ao cadastrado
 *  (comparação case-sensitive via strcmp).
 *
 *  @param tabela  A tabela hash com as evidências coletadas.
 * ================================================================ */
void verificarSuspeitoFinal(TabelaHash tabela) {
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║           ⚖️  FASE DE JULGAMENTO                  ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");

    /* ── Lista suspeitos e contagens disponíveis ── */
    printf("  Evidências registradas por suspeito:\n");
    suspeitorMaisRelatado(tabela);

    /* ── Leitura da acusação do jogador ── */
    char acusado[MAX_SUSPEITO];
    printf("\n  👮 Digite o nome do suspeito que você acusa: ");

    /* Limpa buffer antes de fgets */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (!fgets(acusado, sizeof(acusado), stdin)) {
        printf("  ❌ Falha ao ler entrada.\n");
        return;
    }

    /* Remove newline final caso presente */
    acusado[strcspn(acusado, "\n")] = '\0';

    if (strlen(acusado) == 0) {
        printf("  ⚠️  Nenhum suspeito informado. Julgamento cancelado.\n");
        return;
    }

    /* ── Contagem de pistas que apontam para o acusado ── */
    int pistasCont = contarPistasPorSuspeito(tabela, acusado);

    printf("\n  🔍 Verificando evidências contra \"%s\"...\n", acusado);
    printf("     → %d pista(s) encontrada(s).\n", pistasCont);

    /* ── Veredicto: mínimo de 2 pistas para condenação ── */
    if (pistasCont >= 2) {
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║  🔨 CULPADO CONFIRMADO!                  ║\n");
        printf("  ║  %s foi condenado(a) com %d evidência(s). ║\n",
               acusado, pistasCont);
        printf("  ╚══════════════════════════════════════════╝\n");
        printf("  🎉 Parabéns, detetive! Caso resolvido!\n");
    } else if (pistasCont == 1) {
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║  ⚠️  ACUSAÇÃO REJEITADA                   ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        printf("  Apenas 1 pista contra \"%s\".\n", acusado);
        printf("  São necessárias pelo menos 2 evidências para condenar.\n");
        printf("  Continue investigando antes de acusar!\n");
    } else {
        printf("\n  ╔══════════════════════════════════════════╗\n");
        printf("  ║  ❌ ACUSAÇÃO REJEITADA                    ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        if (encontrarSuspeito(tabela, acusado) == NULL &&
            contarPistasPorSuspeito(tabela, acusado) == 0) {
            printf("  Suspeito \"%s\" não consta nas evidências coletadas.\n", acusado);
        }
        printf("  Nenhuma pista encontrada contra \"%s\".\n", acusado);
        printf("  Explore mais a mansão antes de acusar!\n");
    }
}

/* ================================================================
 *  liberarHash
 *  -----------
 *  Libera toda a memória das listas encadeadas da tabela hash.
 *
 *  @param tabela  A tabela hash.
 * ================================================================ */
void liberarHash(TabelaHash tabela) {
    for (int i = 0; i < TAM_HASH; i++) {
        EntradaHash *atual = tabela[i];
        while (atual) {
            EntradaHash *prox = atual->prox;
            free(atual);
            atual = prox;
        }
        tabela[i] = NULL;
    }
}

/* ================================================================
 *  criarSala
 *  ---------
 *  Aloca dinamicamente um novo nó Sala e inicializa seus campos.
 *
 *  Copia com segurança os três campos de texto (nome, pista e
 *  suspeito) usando strncpy com limite de buffer, garantindo
 *  terminação em '\0'. Os ponteiros filhos são iniciados como NULL,
 *  tornando o nó uma folha pronta para ser inserida na árvore.
 *
 *  @param nome      Nome identificador do cômodo (ex: "Biblioteca").
 *  @param pista     Pista estática do cômodo (pode ser "" se vazio).
 *  @param suspeito  Suspeito associado à pista (pode ser "" se vazio).
 *  @return          Ponteiro para a nova Sala alocada.
 *                   Encerra o programa com EXIT_FAILURE se malloc falhar.
 * ================================================================ */
Sala *criarSala(const char *nome, const char *pista, const char *suspeito) {
    Sala *nova = (Sala *)malloc(sizeof(Sala));
    if (!nova) {
        fprintf(stderr, "[ERRO] Falha ao alocar sala '%s'.\n", nome);
        exit(EXIT_FAILURE);
    }
    strncpy(nova->nome,     nome,     MAX_NOME     - 1);
    strncpy(nova->pista,    pista,    MAX_PISTA    - 1);
    strncpy(nova->suspeito, suspeito, MAX_SUSPEITO - 1);
    nova->nome[MAX_NOME         - 1] = '\0';
    nova->pista[MAX_PISTA       - 1] = '\0';
    nova->suspeito[MAX_SUSPEITO - 1] = '\0';
    nova->esquerda = NULL;
    nova->direita  = NULL;
    return nova;
}

/* ================================================================
 *  BST — inserirPista
 *  ------------------
 *  Insere uma pista coletada na árvore binária de busca (BST),
 *  mantendo a ordenação lexicográfica dos nós.
 *
 *  A inserção é feita recursivamente:
 *    • Se a raiz for NULL, cria um novo nó folha com a pista.
 *    • Se a pista for menor (strcmp < 0), desce à subárvore esquerda.
 *    • Se a pista for maior (strcmp > 0), desce à subárvore direita.
 *    • Duplicatas são ignoradas (strcmp == 0 não insere novo nó).
 *
 *  Complexidade: O(h), onde h é a altura da árvore.
 *
 *  @param raiz   Raiz da BST (ou NULL para árvore vazia).
 *  @param pista  String da pista a ser inserida.
 *  @return       Nova raiz da BST após a inserção.
 * ================================================================ */
NoPista *inserirPista(NoPista *raiz, const char *pista) {
    if (!raiz) {
        NoPista *novo = (NoPista *)malloc(sizeof(NoPista));
        if (!novo) {
            fprintf(stderr, "[ERRO] BST: sem memória.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(novo->pista, pista, MAX_PISTA - 1);
        novo->pista[MAX_PISTA - 1] = '\0';
        novo->esquerda = NULL;
        novo->direita  = NULL;
        return novo;
    }
    int cmp = strcmp(pista, raiz->pista);
    if      (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0) raiz->direita  = inserirPista(raiz->direita,  pista);
    /* cmp == 0: pista duplicada — ignora */
    return raiz;
}

/* ================================================================
 *  emOrdem / liberarBST / liberarArvore  (sem alterações)
 * ================================================================ */

void emOrdem(NoPista *raiz) {
    if (!raiz) return;
    emOrdem(raiz->esquerda);
    printf("    📄 %s\n", raiz->pista);
    emOrdem(raiz->direita);
}

void liberarBST(NoPista *raiz) {
    if (!raiz) return;
    liberarBST(raiz->esquerda);
    liberarBST(raiz->direita);
    free(raiz);
}

void liberarArvore(Sala *raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    free(raiz);
}

/* ================================================================
 *  explorarSalas
 *  -------------
 *  Navega recursivamente pela árvore binária do mapa da mansão,
 *  exibindo o cômodo atual e ativando o sistema de coleta de pistas.
 *
 *  A cada visita a um cômodo com pista não vazia:
 *    1. Exibe o nome da pista e o suspeito vinculado a ela.
 *    2. Insere a pista na BST (inserirPista), mantendo ordenação.
 *    3. Registra o par (pista → suspeito) na tabela hash (inserirNaHash).
 *
 *  Ao chegar em uma folha (sem filhos), informa que não há mais
 *  caminhos disponíveis e encerra o ramo de exploração.
 *
 *  O menu interativo oferece três opções ao jogador:
 *    [e] — avança para o cômodo à esquerda na árvore.
 *    [d] — avança para o cômodo à direita na árvore.
 *    [s] — encerra a exploração imediatamente.
 *  Entrada inválida repete o menu do cômodo atual (recursão).
 *
 *  @param atual   Ponteiro para o nó (Sala) corrente na árvore.
 *  @param bst     Ponteiro para a raiz da BST de pistas (atualizada).
 *  @param tabela  Tabela hash de evidências (pista → suspeito).
 * ================================================================ */
void explorarSalas(Sala *atual, NoPista **bst, TabelaHash tabela) {
    if (!atual) return;

    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║  🕵️  Você está em: %-22s║\n", atual->nome);
    printf("╚═══════════════════════════════════════════╝\n");

    if (strlen(atual->pista) > 0) {
        printf("  🔎 Pista encontrada : \"%s\"\n", atual->pista);
        printf("  👤 Suspeito ligado  : %s\n",
               strlen(atual->suspeito) > 0 ? atual->suspeito : "(sem suspeito)");

        /* 1. Registra pista na BST (ordenação lexicográfica) */
        *bst = inserirPista(*bst, atual->pista);

        /* 2. Registra na Hash apenas se houver suspeito associado */
        if (strlen(atual->suspeito) > 0)
            inserirNaHash(tabela, atual->pista, atual->suspeito);

        printf("  ✅ Evidência registrada!\n");
    }

    /* Cômodo folha — sem filhos */
    if (!atual->esquerda && !atual->direita) {
        printf("  ➤ Beco sem saída. Não há mais caminhos.\n");
        return;
    }

    /* Menu de navegação interativo */
    printf("  Caminhos disponíveis:\n");
    if (atual->esquerda)
        printf("    [e] ESQUERDA → %s\n", atual->esquerda->nome);
    if (atual->direita)
        printf("    [d] DIREITA  → %s\n", atual->direita->nome);
    printf("    [s] Sair da exploração\n");
    printf("  Sua escolha: ");

    char opcao;
    scanf(" %c", &opcao);

    switch (opcao) {
        case 'e': case 'E':
            if (atual->esquerda) explorarSalas(atual->esquerda, bst, tabela);
            else printf("  ⚠️  Sem caminho à esquerda!\n");
            break;
        case 'd': case 'D':
            if (atual->direita)  explorarSalas(atual->direita,  bst, tabela);
            else printf("  ⚠️  Sem caminho à direita!\n");
            break;
        case 's': case 'S':
            printf("\n  🚪 Exploração encerrada pelo detetive.\n");
            break;
        default:
            printf("  ❌ Opção inválida. Tente novamente.\n");
            explorarSalas(atual, bst, tabela);
    }
}

/* ================================================================
 *  main
 *  ----
 *  Monta o mapa, a BST e a tabela hash; inicia o jogo;
 *  conduz o julgamento final e exibe o relatório de investigação.
 *
 *  Suspeitos do caso:
 *    - Coronel Mustard
 *    - Srta. Scarlet
 *    - Prof. Plum
 *    - Sra. Peacock
 *
 *  Layout da árvore binária da mansão:
 *
 *                   Hall de Entrada
 *                  /               \
 *           Sala de Estar        Biblioteca  [luva / Scarlet]
 *           /         \          /        \
 *       Cozinha      Jardim  Escritório  Porão [carta / Peacock]
 *   [faca/Mustard] [pegadas/Plum] [veneno/Scarlet]
 *        /    \
 *   Despensa   Varanda
 * [ricino/Mustard]
 * ================================================================ */
int main(void) {
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║      🔍 DETECTIVE QUEST — Nível Mestre           ║\n");
    printf("║   Mansão + Pistas (BST) + Suspeitos (Hash)       ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");

    /* ── Tabela Hash: inicializa todos os buckets com NULL ── */
    TabelaHash tabela;
    for (int i = 0; i < TAM_HASH; i++) tabela[i] = NULL;

    /* ── Mapa da mansão (nome, pista, suspeito) ── */
    Sala *raiz = criarSala("Hall de Entrada", "", "");

    raiz->esquerda = criarSala("Sala de Estar", "", "");
    raiz->direita  = criarSala("Biblioteca",    "Luva ensanguentada",         "Srta. Scarlet");

    raiz->esquerda->esquerda = criarSala("Cozinha",    "Faca com impressao digital", "Coronel Mustard");
    raiz->esquerda->direita  = criarSala("Jardim",     "Pegadas na lama",            "Prof. Plum");

    raiz->direita->esquerda  = criarSala("Escritorio", "Frasco de veneno",           "Srta. Scarlet");
    raiz->direita->direita   = criarSala("Porao",      "Carta anonima",              "Sra. Peacock");

    raiz->esquerda->esquerda->esquerda = criarSala("Despensa", "Extrato de ricino", "Coronel Mustard");
    raiz->esquerda->esquerda->direita  = criarSala("Varanda",  "", "");

    /* ── BST vazia ── */
    NoPista *bst = NULL;

    /* ── Exploração interativa ── */
    explorarSalas(raiz, &bst, tabela);

    /* ── Fase de Julgamento Final ── */
    verificarSuspeitoFinal(tabela);

    /* ── Relatório: Caderno de pistas (BST em ordem) ── */
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║           📓 CADERNO DE PISTAS (BST)             ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    if (!bst)
        printf("  Nenhuma pista coletada.\n");
    else
        emOrdem(bst);

    /* ── Relatório: Associações na Hash ── */
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║        🗂️  EVIDÊNCIAS → SUSPEITOS (Hash)          ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    exibirHash(tabela);

    /* ── Relatório: Suspeito mais relatado ── */
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║           🎯 ANÁLISE FINAL DO CASO               ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    suspeitorMaisRelatado(tabela);

    /* ── Limpeza ── */
    liberarArvore(raiz);
    liberarBST(bst);
    liberarHash(tabela);

    printf("\n  🏁 Caso encerrado. Bom trabalho, detetive!\n\n");
    return 0;
}
