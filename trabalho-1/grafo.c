#include "grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

typedef struct adj {
    unsigned int dest;       // índice do vértice vizinho
    int peso;                // peso da aresta ( >= 1 )
    struct adj *prox;        // próximo nó da lista
} adj_t;

typedef struct vert {
    char *nome;              // nome do vértice
    adj_t *cab;              // cabeça da lista de adjacência
} vert_t;

struct grafo {
    char *nome;              // nome do grafo
    unsigned int n_vertices; // número de vértices
    unsigned int n_arestas;  // número de arestas
    unsigned int cap;        // capacidade alocada para o vetor de vértices
    vert_t *v;               // vetor de vértices
};

static char *strdup(const char *s) {
    char *d = malloc(strlen(s) + 1);
    if (!d) {
        fprintf(stderr, "strdup failed\n");
        exit(1);
    }
    return strcpy(d, s);
}

unsigned int busca_vertice(const grafo *g, const char *nome) {
    for (unsigned int i = 0; i < g->n_vertices; ++i)
        if (strcmp(g->v[i].nome, nome) == 0)
            return i;
    return UINT_MAX;
}


unsigned int cria_vertice(grafo *g, const char *nome)
{
    unsigned int idx = busca_vertice(g, nome);
    if (idx != UINT_MAX)
        return idx;

    /* crescer vetor se necessário */
    if (g->n_vertices == g->cap) {
        g->cap = g->cap ? g->cap + 1 : 1;
        g->v = realloc(g->v, g->cap * sizeof(vert_t));
        if (!g->v) {
            fprintf(stderr, "Erro: memória insuficiente (realloc)\n");
            exit(EXIT_FAILURE);
        }
    }
    idx = g->n_vertices++;
    g->v[idx].nome = strdup(nome);
    g->v[idx].cab = NULL;
    return idx;
}

void adiciona_aresta(grafo *g, unsigned int u, unsigned int v, int p)
{
    adj_t *a = malloc(sizeof(adj_t));
    a->dest = v;
    a->peso = p;
    a->prox = g->v[u].cab;
    g->v[u].cab = a;

    adj_t *b = malloc(sizeof(adj_t));
    b->dest = u;
    b->peso = p;
    b->prox = g->v[v].cab;
    g->v[v].cab = b;

    g->n_arestas++;
}

grafo *le_grafo(FILE *f)
{
    if (!f) return NULL;

    grafo *g = malloc(sizeof(grafo));
    g->nome = NULL;
    g->n_vertices = g->n_arestas = 0;
    g->cap = 0;
    g->v = NULL;

    char linha[2048];
    int linha_lida = 0;

    while (fgets(linha, sizeof(linha), f)) {
        char *p = linha;

        // ignora espaços à esquerda
        while (*p && (*p == ' ' || *p == '\t')) p++;
        if (*p == '\0' || *p == '\n') continue; //linha vazia
        if (p[0] == '/' && p[1] == '/') continue; //comentário

        //remove \n final
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';

        if (!linha_lida) {  // primeira linha relevante = nome do grafo
            g->nome = strdup(p);
            linha_lida = 1;
            continue;
        }

        // procura marcador "--" para ver se vertice ou aresta
        char *marc = strstr(p, "--");
        if (!marc) { // vertice isolado
            cria_vertice(g, p);
            continue;
        }

        // aresta
        char v1[2048], v2[2048];
        int peso = 1;
        // split em 3 partes: v1, v2, peso
        if (sscanf(p, " %2047s -- %2047s %d", v1, v2, &peso) < 2)
            continue;   // entrada garantida bem formada ⇒ não deve ocorrer


        unsigned int u = cria_vertice(g, v1);
        unsigned int v = cria_vertice(g, v2);
        adiciona_aresta(g, u, v, peso);
    }
    return g;
}

unsigned int destroi_grafo(grafo *g)
{
    if (!g) return 0;

    for (unsigned int i = 0; i < g->n_vertices; ++i) {
        adj_t *a = g->v[i].cab;
        while (a) {
            adj_t *tmp = a->prox;
            free(a);
            a = tmp;
        }
        free(g->v[i].nome);
    }
    free(g->v);
    free(g->nome);
    free(g);
    return 1;
}


char *nome(grafo *g) { return g ? g->nome : NULL; }

unsigned int n_vertices(grafo *g) { return g ? g->n_vertices : 0; }

unsigned int n_arestas(grafo *g)  { return g ? g->n_arestas  : 0; }

void bfs(const grafo *g, unsigned int origem,
                int *dist, int *componente, int comp_id)
{
    unsigned int n = g->n_vertices;
    unsigned int *q = malloc(n * sizeof(unsigned int));
    unsigned int ini = 0, fim = 0;

    dist[origem] = 0;
    componente[origem] = comp_id;
    q[fim] = origem; fim++;

    while (ini < fim) {
        unsigned int u = q[ini]; ini++;
        for (adj_t *a = g->v[u].cab; a; a = a->prox) {
            unsigned int v = a->dest;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1; // dist sem peso
                componente[v] = comp_id;
                q[fim] = v; fim++;
            }
        }
    }
    free(q);
}

unsigned int calcula_componentes(const grafo *g, int *comps)
{
    unsigned int n = g->n_vertices;
    int *dist = malloc(n * sizeof(int));
    for (unsigned int i = 0; i < n; ++i) dist[i] = -1;

    unsigned int comp = 0;
    for (unsigned int i = 0; i < n; ++i) {
        if (dist[i] == -1) {
            bfs(g, i, dist, comps ? comps : dist, comp);
            comp++;
        }
    }
    free(dist);
    return comp;
}

unsigned int n_componentes(grafo *g)
{
    if (!g) return 0;
    return calcula_componentes(g, NULL);
}

unsigned int bipartido(grafo *g)
{
    if (!g) return 0;
    unsigned int n = g->n_vertices;
    int *cor = malloc(n * sizeof(int));
    for (unsigned int i = 0; i < n; i++) cor[i] = -1;

    int is_bip = 1;
    unsigned int *q = malloc(n * sizeof(unsigned int));

    for (unsigned int s = 0; s < n && is_bip; s++) {
        if (cor[s] != -1) continue;
        unsigned int ini = 0, fim = 0;
        cor[s] = 0;
        q[fim++] = s;

        while (ini < fim && is_bip) {
            unsigned int u = q[ini++];
            for (adj_t *a = g->v[u].cab; a; a = a->prox) {
                unsigned int v = a->dest;
                if (cor[v] == -1) {
                    cor[v] = 1 - cor[u];
                    q[fim++] = v;
                } else if (cor[v] == cor[u]) {
                    is_bip = 0;
                    break;
                }
            }
        }
    }
    free(cor);
    free(q);
    return is_bip;
}

static void dijkstra(const grafo *g, unsigned int s, int *dist)
{
    unsigned int n = g->n_vertices;
    bool *vis = malloc(n * sizeof(bool));
    for (int i = 0; i < n; i++) vis[i] = false;

    for (unsigned i = 0; i < n; ++i) dist[i] = INT_MAX;
    dist[s] = 0;

    for (unsigned iter = 0; iter < n; ++iter) {
        /* pega o vértice não visitado com menor dist */
        int best = -1, bestd = INT_MAX;
        for (unsigned v = 0; v < n; ++v)
            if (!vis[v] && dist[v] < bestd) { bestd = dist[v]; best = v; }
        if (best == -1) break;          /* resto é inalcançável            */

        vis[best] = true;
        for (adj_t *a = g->v[best].cab; a; a = a->prox) {
            unsigned v = a->dest;
            int w = a->peso > 0 ? a->peso : 1;      /* peso 1 default */
            if (!vis[v] && dist[best] + w < dist[v])
                dist[v] = dist[best] + w;
        }
    }
    free(vis);
}


unsigned int diametro_componente(const grafo *g,
                                 const unsigned int *lista,
                                 unsigned int tam)
{
    unsigned int n = g->n_vertices;
    int *dist = malloc(n * sizeof(int));
    unsigned int diam = 0;

    for (unsigned idx = 0; idx < tam; ++idx) {
        unsigned s = lista[idx];
        dijkstra(g, s, dist);

        for (unsigned v = 0; v < n; ++v)
            if (dist[v] != INT_MAX && (unsigned)dist[v] > diam)
                diam = dist[v];
    }
    free(dist);
    return diam;
}


/* ------------------------------------------------------------------ */
/* comparador para ordenar diâmetros em ordem não-decrescente --------*/
static int cmp_uint(const void *a, const void *b)
{
    unsigned int ua = *(const unsigned int *)a;
    unsigned int ub = *(const unsigned int *)b;
    if (ua < ub) return -1;
    if (ua > ub) return  1;
    return 0;
}


unsigned int componentes(const grafo *g,
                                int ign_v,              /*  vértice a ignorar ou -1   */
                                int ign_u, int ign_w)   /*  aresta (u,w) a ignorar ou -1 */
{
    unsigned int n = g->n_vertices;
    bool *vis = malloc(n * sizeof(bool));
    for (unsigned int i = 0; i < n; i++) vis[i] = false;
    unsigned int comps = 0, *fila = malloc(n * sizeof(unsigned int));

    for (unsigned int s = 0; s < n; ++s) {
        if (s == (unsigned)ign_v || vis[s]) continue;
        /* nova componente */
        comps++;
        unsigned int ini = 0, fim = 0;
        fila[fim++] = s;  vis[s] = true;

        while (ini < fim) {
            unsigned int u = fila[ini++];
            for (adj_t *a = g->v[u].cab; a; a = a->prox) {
                unsigned int v = a->dest;
                if ((int)v == ign_v) continue; /*ignora vértice*/
                /* ignora aresta específica*/
                if ((int)u == ign_u && (int)v == ign_w) continue;
                if ((int)u == ign_w && (int)v == ign_u) continue;
                if (!vis[v]) { vis[v] = 1; fila[fim++] = v; }
            }
        }
    }
    free(vis); free(fila);
    return comps;
}

char *diametros(grafo *g)
{
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (n == 0) return strdup("");

    /* agrupa vértices por componente */
    int *comp = malloc(n * sizeof(int));
    unsigned int ncomp = calcula_componentes(g, comp);

    unsigned int *tam = malloc(ncomp * sizeof(unsigned int));
    memset(tam, 0, ncomp * sizeof(unsigned int));
    for (unsigned int i = 0; i < n; ++i) tam[comp[i]]++;

    unsigned int **lista = malloc(ncomp * sizeof(unsigned int *));
    for (unsigned int c = 0; c < ncomp; ++c)
        lista[c] = malloc(tam[c] * sizeof(unsigned int));

    for (unsigned int c = 0; c < ncomp; ++c) tam[c] = 0;
    for (unsigned int i = 0; i < n; ++i)
        lista[comp[i]][tam[comp[i]]++] = i;

    /* calcula diâmetros */
    unsigned int *d = malloc(ncomp * sizeof(unsigned int));
    for (unsigned int c = 0; c < ncomp; ++c)
        d[c] = diametro_componente(g, lista[c], tam[c]);

    
    /* ordena em ordem não-decrescente usando qsort */
    qsort(d, ncomp, sizeof(unsigned int), cmp_uint);

    /* monta string */
    size_t len = 0;
    for (unsigned int c = 0; c < ncomp; ++c) {
        char buf[32]; sprintf(buf, "%u", d[c]);
        len += strlen(buf) + 1;
    }
    char *out = malloc(len ? len : 1); out[0] = '\0';
    for (unsigned int c = 0; c < ncomp; ++c) {
        char buf[32]; sprintf(buf, "%u", d[c]);
        strcat(out, buf); if (c + 1 < ncomp) strcat(out, " ");
    }

    /* libera */
    for (unsigned int c = 0; c < ncomp; ++c) free(lista[c]);
    free(lista); free(tam); free(comp); free(d);
    return out;
}

char *vertices_corte(grafo *g)
{
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (n == 0) return strdup("");

    const unsigned int comps0 = componentes(g, -1, -1, -1);
    char **arts = malloc(n * sizeof(char *));
    unsigned int qtd = 0;

    for (unsigned int v = 0; v < n; ++v) {
        if (componentes(g, (int)v, -1, -1) > comps0)
            arts[qtd++] = g->v[v].nome;
    }
    if (qtd == 0) { free(arts); return strdup(""); }

    qsort(arts, qtd, sizeof(char*), (int(*)(const void*,const void*)) strcmp);

    size_t len = 0;
    for (unsigned i = 0; i < qtd; ++i) len += strlen(arts[i]) + 1;
    char *out = malloc(len); out[0] = '\0';
    for (unsigned i = 0; i < qtd; ++i) {
        strcat(out, arts[i]); if (i + 1 < qtd) strcat(out, " ");
    }
    free(arts); return out;
}

/* comparador para qsort de char* */
int cmp_str(const void *a, const void *b)
{
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcmp(sa, sb);
}


char *arestas_corte(grafo *g)
{
    if (!g) return NULL;
    unsigned int n = g->n_vertices;
    if (n == 0) return strdup("");

    const unsigned int comps0 = componentes(g, -1, -1, -1);

    /* no pior caso, toda aresta é ponte */
    char **edges = malloc(g->n_arestas * sizeof(char *));
    unsigned int qtd = 0;

    for (unsigned int u = 0; u < n; ++u) {
        for (adj_t *a = g->v[u].cab; a; a = a->prox) {
            unsigned int v = a->dest;
            if (u < v) { // evita duplicar arestas
                if (componentes(g, -1, (int)u, (int)v) > comps0) {
                    const char *s1 = g->v[u].nome;
                    const char *s2 = g->v[v].nome;
                    if (strcmp(s1, s2) > 0) { const char *tmp = s1; s1 = s2; s2 = tmp; }

                    size_t len = strlen(s1) + strlen(s2) + 2;
                    char *str = malloc(len);
                    snprintf(str, len, "%s %s", s1, s2);
                    edges[qtd++] = str;
                }
            }
        }
    }

        if (qtd == 0) {
        free(edges);
        return strdup("");
    }

    /* ordena as strings alfabeticamente */
    qsort(edges, qtd, sizeof(char *), cmp_str);

    /* monta string final */
    size_t len = 0;
    for (unsigned i = 0; i < qtd; ++i) len += strlen(edges[i]) + 1;

    char *out = malloc(len);
    out[0] = '\0';

    for (unsigned i = 0; i < qtd; ++i) {
        strcat(out, edges[i]);
        if (i + 1 < qtd) strcat(out, " ");
        free(edges[i]);          /* libera string individual          */
    }
    free(edges);
    return out;

}
