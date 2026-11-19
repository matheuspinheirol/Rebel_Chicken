#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define LARGURA       800
#define ALTURA        450
#define CHAO_Y        420

#define VEL_X_INICIAL 250.0f
#define VEL_X_MAXIMA  750.0f

#define SOBE         -1300.0f
#define DESCE         900.0f
#define VEL_MAXIMA    550.0f

#define SPRITE_LARG   12
#define SPRITE_ALT    12
#define ESCALA_SPRITE 3

#define MILHO_LARG 16
#define MILHO_ALT 24
#define PASSO_MILHO 30
#define PONTOS_POR_MILHO 10

#define FALCAO_LARG 56
#define FALCAO_ALT  28
#define FALCAO_VEL_EXTRA 200.0f
#define AVISO_TEMPO      1.2f
#define GAP_FALCAO         8
#define MAX_FALCOES        6
#define SEGUNDOS_POR_NIVEL 20.0f
#define TEMPO_INICIO_FALCAO 10.0f

#define VEL_GIRO          180.0f

#define MAX_SCORES     5
#define ARQUIVO_SCORES "top_scores.txt"

#define QTD_ARVORES  3
#define PERIODO      1000

#define QTD_NUVENS 5

typedef enum {
    OBSTACULO,
    OBS_HORIZONTAL,
    OBS_INCLINADO,
    OBS_GIRATORIO,
    MILHO,
    FALCAO
} TipoElemento;

typedef struct Elemento {
    TipoElemento tipo;
    Rectangle caixa;
    float angulo;
    struct Elemento *prox;
} Elemento;

typedef enum {
    TELA_INICIAL,
    JOGANDO
} EstadoJogo;

const float nuvem_x[QTD_NUVENS]   = { 80, 280, 480, 700, 900 };
const float nuvem_y[QTD_NUVENS]   = { 60, 120,  45, 100, 150 };
const float nuvem_esc[QTD_NUVENS] = { 1.0f, 0.7f, 1.3f, 0.9f, 0.8f };

const float arvore_x[QTD_ARVORES]   = { 100, 450, 780 };
const float arvore_esc[QTD_ARVORES] = { 1.0f, 0.8f, 1.2f };

int carregar_scores(int scores[])
{
    FILE *arq = fopen(ARQUIVO_SCORES, "r");
    int qtd = 0;

    if (arq == NULL) return 0;

    while (qtd < MAX_SCORES && fscanf(arq, "%d", &scores[qtd]) == 1) {
        qtd++;
    }

    fclose(arq);
    return qtd;
}

void salvar_scores(int scores[], int qtd)
{
    FILE *arq = fopen(ARQUIVO_SCORES, "w");
    if (arq == NULL) return;

    for (int i = 0; i < qtd; i++) {
        fprintf(arq, "%d\n", scores[i]);
    }

    fclose(arq);
}

void registrar_score(int scores[], int *qtd, int pontos)
{
    int pos = 0;
    while (pos < *qtd && scores[pos] >= pontos) pos++;

    if (pos >= MAX_SCORES) return;

    int fim = (*qtd < MAX_SCORES) ? *qtd : MAX_SCORES - 1;
    for (int i = fim; i > pos; i--) {
        scores[i] = scores[i - 1];
    }

    scores[pos] = pontos;
    if (*qtd < MAX_SCORES) (*qtd)++;

    salvar_scores(scores, *qtd);
}

int sprite_galinha[SPRITE_ALT][SPRITE_LARG] = {
    {0,0,0,0,0,0,2,2,0,0,0,0},
    {0,0,0,0,0,2,2,2,2,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,0,0},
    {0,0,0,0,0,1,1,4,1,3,3,0},
    {0,0,0,0,0,1,1,1,2,3,0,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,1,0,0},
    {1,1,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,3,0,3,0,0,0,0},
    {0,0,0,0,0,3,3,3,3,0,0,0}
};

void desenhar_galinha(int x, int y)
{
    Color cores[5] = { BLANK, WHITE, RED, ORANGE, BLACK };

    for (int linha = 0; linha < SPRITE_ALT; linha++) {
        for (int coluna = 0; coluna < SPRITE_LARG; coluna++) {
            int valor = sprite_galinha[linha][coluna];
            if (valor != 0) {
                DrawRectangle(x + coluna * ESCALA_SPRITE,
                              y + linha * ESCALA_SPRITE,
                              ESCALA_SPRITE, ESCALA_SPRITE,
                              cores[valor]);
            }
        }
    }
}

void desenhar_triangulo(Vector2 a, Vector2 b, Vector2 c, Color cor)
{
    float cruz = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cruz < 0) DrawTriangle(a, b, c, cor);
    else          DrawTriangle(a, c, b, cor);
}

void desenhar_milho(Rectangle r)
{
    int x = (int) r.x, y = (int) r.y;
    int cx = x + MILHO_LARG / 2;
    Color grao = (Color){ 205, 133, 0, 255 };

    DrawEllipse(cx, y + 10, 6, 10, GOLD);
    for (int i = 0; i < 3; i++) {
        DrawLine(cx - 5, y + 5 + i * 5, cx + 5, y + 5 + i * 5, grao);
    }
    DrawLine(cx, y + 1, cx, y + 19, grao);

    desenhar_triangulo((Vector2){ cx, y + 24 },
                       (Vector2){ cx - 8, y + 9 },
                       (Vector2){ cx - 1, y + 13 }, DARKGREEN);
    desenhar_triangulo((Vector2){ cx, y + 24 },
                       (Vector2){ cx + 8, y + 9 },
                       (Vector2){ cx + 1, y + 13 }, GREEN);
}

void desenhar_falcao(Rectangle r)
{
    int x = (int) r.x, y = (int) r.y, w = (int) r.width;
    float bater = sinf((float) GetTime() * 16.0f);

    desenhar_triangulo((Vector2){ x + w - 12, y + 15 },
                       (Vector2){ x + w + 6,  y + 9 },
                       (Vector2){ x + w + 6,  y + 23 }, DARKBROWN);
    desenhar_triangulo((Vector2){ x + 22, y + 15 },
                       (Vector2){ x + 46, y + 15 },
                       (Vector2){ x + 44, y + 14 - 18 * bater }, BROWN);
    DrawEllipse(x + 32, y + 17, 24, 9, BROWN);
    DrawEllipse(x + 30, y + 20, 16, 5, BEIGE);
    DrawCircle(x + 13, y + 13, 9, DARKBROWN);
    desenhar_triangulo((Vector2){ x - 3, y + 14 },
                       (Vector2){ x + 6, y + 10 },
                       (Vector2){ x + 6, y + 18 }, GOLD);
    DrawCircle(x + 11, y + 11, 3, WHITE);
    DrawCircle(x + 10, y + 11, 1.5f, BLACK);
    DrawLine(x + 6, y + 5, x + 15, y + 9, BLACK);
}

void desenhar_alerta(float faixa_y, float faixa_alt, int qtd, float tempo)
{
    float pulso = sinf(tempo * 18.0f);
    float brilho = pulso * 0.5f + 0.5f;

    DrawRectangle(0, (int) faixa_y - 4, LARGURA, (int) faixa_alt + 8,
                  Fade(RED, 0.15f + 0.25f * brilho));

    int tam = (int)(44 + 6 * pulso);
    int larg_ex = MeasureText("!", tam);
    float cx = LARGURA - 40;
    float cy = faixa_y + faixa_alt / 2;
    if (cy < tam / 2 + 10) cy = tam / 2 + 10;
    if (cy > CHAO_Y - tam / 2 - 10) cy = CHAO_Y - tam / 2 - 10;

    int ex = (int)(cx - larg_ex / 2);
    int ey = (int)(cy - tam / 2);
    DrawText("!", ex + 2, ey + 2, tam, BLACK);
    DrawText("!", ex,     ey,     tam, RED);

    const char *texto = (qtd > 1) ? "FALCOES!" : "FALCAO!";
    int larg_txt = MeasureText(texto, 26);
    int tx = (int)(cx - larg_ex / 2 - 16 - larg_txt);
    DrawText(texto, tx + 2, (int) cy - 11, 26, BLACK);
    DrawText(texto, tx,     (int) cy - 13, 26, RED);
}

float posicao_loop(float x0, float deslocamento, float fator, float folga)
{
    float x = fmodf(x0 - deslocamento * fator, PERIODO);
    if (x < 0) x += PERIODO;
    return x - folga;
}

void desenhar_colinas(float deslocamento, float fator, int passo,
                      float rx, float ry, Color cor)
{
    float d = fmodf(deslocamento * fator, (float) passo);
    for (int i = -1; i <= LARGURA / passo + 1; i++) {
        float cx = i * passo - d + passo / 2.0f;
        DrawEllipse((int) cx, CHAO_Y, rx, ry, cor);
    }
}

void desenhar_arvore(float x, float base, float e)
{
    DrawRectangle((int)(x - 6 * e), (int)(base - 48 * e), (int)(12 * e), (int)(48 * e), BROWN);
    DrawCircle((int)(x - 20 * e), (int)(base - 52 * e), 20 * e, DARKGREEN);
    DrawCircle((int)(x + 20 * e), (int)(base - 52 * e), 20 * e, DARKGREEN);
    DrawCircle((int) x,           (int)(base - 64 * e), 28 * e, GREEN);
}

void desenhar_nuvem(float x, float y, float e)
{
    Color branco = (Color){ 255, 255, 255, 255 };

    DrawCircle((int) x,              (int) y,              25 * e, branco);
    DrawCircle((int)(x + 30 * e),    (int)(y - 15 * e),    30 * e, branco);
    DrawCircle((int)(x + 65 * e),    (int) y,              25 * e, branco);
    DrawCircle((int)(x + 30 * e),    (int)(y + 8 * e),     25 * e, branco);
}

void desenhar_sol(float cx, float cy)
{
    DrawCircle((int) cx, (int) cy, 56, Fade(YELLOW, 0.20f));
    DrawCircle((int) cx, (int) cy, 36, GOLD);
    DrawCircle((int) cx, (int) cy, 32, YELLOW);
}

void desenhar_cenario(float deslocamento)
{
    ClearBackground(SKYBLUE);
    desenhar_sol(690, 75);

    for (int i = 0; i < QTD_NUVENS; i++) {
        float x = posicao_loop(nuvem_x[i], deslocamento, 0.05f, 150);
        desenhar_nuvem(x, nuvem_y[i], nuvem_esc[i]);
    }

    desenhar_colinas(deslocamento, 0.10f, 420, 270, 85, (Color){ 150, 210, 150, 255 });
    desenhar_colinas(deslocamento, 0.25f, 300, 200, 55, (Color){ 100, 180, 100, 255 });

    DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, DARKGREEN);
    DrawRectangle(0, CHAO_Y, LARGURA, 6, (Color){ 80, 180, 70, 255 });

    for (int i = 0; i < QTD_ARVORES; i++) {
        float x = posicao_loop(arvore_x[i], deslocamento, 0.5f, 100);
        desenhar_arvore(x, CHAO_Y + 4, arvore_esc[i]);
    }

    DrawRectangle(0, CHAO_Y - 18, LARGURA, 4, BROWN);
    DrawRectangle(0, CHAO_Y - 8,  LARGURA, 4, BROWN);
    int desloc_cerca = (int) deslocamento % 100;
    for (int i = -1; i <= LARGURA / 100 + 1; i++) {
        DrawRectangle(i * 100 - desloc_cerca, CHAO_Y - 25, 8, 25, DARKBROWN);
    }
}

Elemento *inserir(Elemento *inicio, float x, float y, float larg, float alt)
{
    Elemento *novo = (Elemento *) malloc(sizeof(Elemento));
    if (novo == NULL) return inicio;

    novo->caixa = (Rectangle){ x, y, larg, alt };
    novo->tipo = OBSTACULO;
    novo->angulo = 0;
    novo->prox = inicio;
    return novo;
}

Elemento *inserir_falcao(Elemento *inicio, float x, float y)
{
    Elemento *novo = inserir(inicio, x, y, FALCAO_LARG, FALCAO_ALT);
    if (novo != inicio) novo->tipo = FALCAO;
    return novo;
}

Elemento *criar_onda_falcoes(Elemento *lista, float faixa_y, int qtd)
{
    for (int i = 0; i < qtd; i++) {
        float x = LARGURA + 20 + (i % 2) * 40;
        float y = faixa_y + i * (FALCAO_ALT + GAP_FALCAO);
        lista = inserir_falcao(lista, x, y);
    }
    return lista;
}

Elemento *atualizar(Elemento *inicio, float dt, float vel_cenario)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        float vel = vel_cenario;
        if (atual->tipo == FALCAO) vel += FALCAO_VEL_EXTRA;
        atual->caixa.x -= vel * dt;

        if (atual->tipo == OBS_GIRATORIO) {
            atual->angulo += VEL_GIRO * dt;
            if (atual->angulo >= 360.0f) atual->angulo -= 360.0f;
        }

        if (atual->caixa.x + atual->caixa.width < 0) {
            Elemento *remover = atual;
            if (anterior == NULL) inicio = atual->prox;
            else anterior->prox = atual->prox;
            atual = atual->prox;
            free(remover);
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
    return inicio;
}

void calcular_cantos(Vector2 c, float w, float h, float ang, Vector2 out[4])
{
    float a = ang * DEG2RAD;
    float cs = cosf(a), sn = sinf(a);
    float hx = w / 2, hy = h / 2;
    Vector2 loc[4] = { {-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy} };

    for (int i = 0; i < 4; i++) {
        out[i].x = c.x + loc[i].x * cs - loc[i].y * sn;
        out[i].y = c.y + loc[i].x * sn + loc[i].y * cs;
    }
}

void projetar(Vector2 p[4], Vector2 eixo, float *min, float *max)
{
    *min = *max = p[0].x * eixo.x + p[0].y * eixo.y;
    for (int i = 1; i < 4; i++) {
        float v = p[i].x * eixo.x + p[i].y * eixo.y;
        if (v < *min) *min = v;
        if (v > *max) *max = v;
    }
}

int colisao_sat(Vector2 a[4], Vector2 b[4])
{
    Vector2 *poly[2] = { a, b };
    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < 4; i++) {
            Vector2 p1 = poly[k][i];
            Vector2 p2 = poly[k][(i + 1) % 4];
            Vector2 eixo = { -(p2.y - p1.y), p2.x - p1.x };
            float minA, maxA, minB, maxB;
            projetar(a, eixo, &minA, &maxA);
            projetar(b, eixo, &minB, &maxB);
            if (maxA < minB || maxB < minA) return 0;
        }
    }
    return 1;
}

int colisao_rotacionada(Rectangle galinha, Elemento *e)
{
    Vector2 cg = { galinha.x + galinha.width / 2, galinha.y + galinha.height / 2 };
    Vector2 ce = { e->caixa.x + e->caixa.width / 2, e->caixa.y + e->caixa.height / 2 };
    Vector2 pg[4], pe[4];
    calcular_cantos(cg, galinha.width, galinha.height, 0, pg);
    calcular_cantos(ce, e->caixa.width, e->caixa.height, e->angulo, pe);
    return colisao_sat(pg, pe);
}

int colidiu(Elemento *inicio, Rectangle galinha)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if (p->tipo == MILHO) continue;

        Rectangle caixa = p->caixa;
        if (p->tipo == FALCAO) {
            caixa.y += 4;
            caixa.height -= 8;
        }

        if (p->tipo == OBS_INCLINADO || p->tipo == OBS_GIRATORIO) {
            if (colisao_rotacionada(galinha, p)) return 1;
        } else if (CheckCollisionRecs(galinha, caixa)) {
            return 1;
        }
    }
    return 0;
}

Vector2 ponto_arame(Vector2 centro, Vector2 d, Vector2 n, float t, float u)
{
    return (Vector2){ centro.x + d.x * t + n.x * u,
                      centro.y + d.y * t + n.y * u };
}

void desenhar_arame(Elemento *p)
{
    Rectangle c = p->caixa;
    Vector2 centro = { c.x + c.width / 2, c.y + c.height / 2 };

    int em_pe = c.height > c.width;
    float comp = em_pe ? c.height : c.width;
    float esp  = em_pe ? c.width  : c.height;
    float ang  = (p->angulo + (em_pe ? 90.0f : 0.0f)) * DEG2RAD;

    Vector2 d = { cosf(ang), sinf(ang) };
    Vector2 n = { -sinf(ang), cosf(ang) };
    float m = esp / 2 - 2;
    float passo = 18;

    for (int s = -1; s <= 1; s += 2) {
        DrawLineEx(ponto_arame(centro, d, n, s * comp / 2, -esp / 2 - 4),
                   ponto_arame(centro, d, n, s * comp / 2,  esp / 2 + 4),
                   6, BLACK);
        DrawLineEx(ponto_arame(centro, d, n, s * comp / 2, -esp / 2 - 3),
                   ponto_arame(centro, d, n, s * comp / 2,  esp / 2 + 3),
                   3, BROWN);
    }

    for (float t = -comp / 2; t + passo / 2 <= comp / 2; t += passo) {
        Vector2 a  = ponto_arame(centro, d, n, t,             -m);
        Vector2 b  = ponto_arame(centro, d, n, t + passo / 2, +m);
        Vector2 a2 = ponto_arame(centro, d, n, t + passo,     -m);
        DrawLineEx(a, b, 2, DARKGRAY);
        DrawLineEx(b, a2, 2, DARKGRAY);
    }

    for (int s = -1; s <= 1; s += 2) {
        Vector2 ini = ponto_arame(centro, d, n, -comp / 2, s * m);
        Vector2 fim = ponto_arame(centro, d, n,  comp / 2, s * m);
        DrawLineEx(ini, fim, 5, BLACK);
        DrawLineEx(ini, fim, 3, LIGHTGRAY);
    }

    for (float t = -comp / 2 + passo / 2; t < comp / 2 - 4; t += passo) {
        for (int s = -1; s <= 1; s += 2) {
            DrawLineEx(ponto_arame(centro, d, n, t - 5, s * m - 5),
                       ponto_arame(centro, d, n, t + 5, s * m + 5), 2, RED);
            DrawLineEx(ponto_arame(centro, d, n, t - 5, s * m + 5),
                       ponto_arame(centro, d, n, t + 5, s * m - 5), 2, RED);
        }
    }
}

void desenhar_lista(Elemento *inicio)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if (p->tipo == MILHO) {
            desenhar_milho(p->caixa);
        } else if (p->tipo != FALCAO) {
            desenhar_arame(p);
        }
    }
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if (p->tipo == FALCAO) desenhar_falcao(p->caixa);
    }
}

Elemento *liberar_lista(Elemento *inicio)
{
    while (inicio != NULL) {
        Elemento *proximo = inicio->prox;
        free(inicio);
        inicio = proximo;
    }
    return NULL;
}

Rectangle com_folga(Rectangle r)
{
    float folga = 10;
    return (Rectangle){ r.x - folga, r.y - folga,
                        r.width + 2 * folga, r.height + 2 * folga };
}

Rectangle caixa_envolvente(Elemento *e)
{
    Rectangle c = e->caixa;

    if (e->tipo == OBS_INCLINADO || e->tipo == OBS_GIRATORIO) {
        float cx = c.x + c.width / 2, cy = c.y + c.height / 2;
        float raio = sqrtf(c.width * c.width + c.height * c.height) / 2;
        return (Rectangle){ cx - raio, cy - raio, 2 * raio, 2 * raio };
    }
    return c;
}

int area_livre(Elemento *lista, Rectangle area)
{
    Rectangle a = com_folga(area);
    for (Elemento *p = lista; p != NULL; p = p->prox) {
        if (CheckCollisionRecs(a, com_folga(caixa_envolvente(p)))) return 0;
    }
    return 1;
}

Elemento *inserir_milho(Elemento *inicio, float x, float y)
{
    Elemento *nova = inserir(inicio, x, y, MILHO_LARG, MILHO_ALT);
    if (nova != inicio) nova->tipo = MILHO;
    return nova;
}

Elemento *coletar_milho(Elemento *inicio, Rectangle galinha, int *milho)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        if (atual->tipo == MILHO && CheckCollisionRecs(galinha, atual->caixa)) {
            (*milho)++;
            Elemento *remover = atual;
            if (anterior == NULL) inicio = atual->prox;
            else anterior->prox = atual->prox;
            atual = atual->prox;
            free(remover);
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
    return inicio;
}

Elemento *criar_bloco_milho(Elemento *lista, int colunas, int linhas)
{
    float larg_bloco = (float)((colunas - 1) * PASSO_MILHO + MILHO_LARG);
    float alt_bloco  = (float)((linhas  - 1) * PASSO_MILHO + MILHO_ALT);

    for (int tentativa = 0; tentativa < 10; tentativa++) {
        float y = (float) GetRandomValue(20, (int)(CHAO_Y - alt_bloco - 10));
        Rectangle area = { LARGURA, y, larg_bloco, alt_bloco };

        if (area_livre(lista, area)) {
            for (int i = 0; i < linhas; i++) {
                for (int j = 0; j < colunas; j++) {
                    lista = inserir_milho(lista,
                                          LARGURA + j * PASSO_MILHO,
                                          y + i * PASSO_MILHO);
                }
            }
            return lista;
        }
    }
    return lista;
}

Elemento *criar_grupo_milho(Elemento *lista)
{
    switch (GetRandomValue(0, 9)) {
        case 0: case 1: case 2: case 3:
            return criar_bloco_milho(lista, GetRandomValue(3, 8), 1);
        case 4: case 5: case 6: {
            int lado = GetRandomValue(2, 4);
            return criar_bloco_milho(lista, lado, lado);
        }
        case 7: case 8:
            return criar_bloco_milho(lista, 1, GetRandomValue(3, 5));
        default:
            return criar_bloco_milho(lista, GetRandomValue(1, 2), 1);
    }
}

Elemento *inserir_obstaculo(Elemento *inicio, TipoElemento tipo,
                            float x, float y, float larg, float alt, float angulo)
{
    Elemento *novo = inserir(inicio, x, y, larg, alt);
    if (novo != inicio) {
        novo->tipo = tipo;
        novo->angulo = angulo;
    }
    return novo;
}

Elemento *inserir_se_livre(Elemento *lista, TipoElemento tipo,
                           float x, float y, float larg, float alt, float ang)
{
    Elemento tmp = { tipo, { x, y, larg, alt }, ang, NULL };
    if (!area_livre(lista, caixa_envolvente(&tmp))) return lista;
    return inserir_obstaculo(lista, tipo, x, y, larg, alt, ang);
}

Elemento *criar_obstaculo_vertical(Elemento *lista)
{
    float alt = (float) GetRandomValue(70, 150);
    float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
    return inserir_se_livre(lista, OBSTACULO, LARGURA, y, 22, alt, 0);
}

Elemento *criar_obstaculo_horizontal(Elemento *lista)
{
    float alt = 22;
    float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
    return inserir_se_livre(lista, OBS_HORIZONTAL, LARGURA, y, 130, alt, 0);
}

Elemento *criar_obstaculo_inclinado(Elemento *lista)
{
    float larg = 150, alt = 22, margem = 80;
    float cy = (float) GetRandomValue((int) margem, (int)(CHAO_Y - margem));
    float ang = (float) GetRandomValue(30, 45);
    return inserir_se_livre(lista, OBS_INCLINADO, LARGURA, cy - alt / 2, larg, alt, ang);
}

Elemento *criar_obstaculo_giratorio(Elemento *lista)
{
    float larg = 110, alt = 20, margem = 60;
    float cy = (float) GetRandomValue((int) margem, (int)(CHAO_Y - margem));
    return inserir_se_livre(lista, OBS_GIRATORIO, LARGURA, cy - alt / 2, larg, alt, 0);
}

Elemento *criar_obstaculo(Elemento *lista)
{
    for (int tentativa = 0; tentativa < 10; tentativa++) {
        Elemento *nova;
        switch (GetRandomValue(0, 3)) {
            case 0:  nova = criar_obstaculo_vertical(lista);   break;
            case 1:  nova = criar_obstaculo_horizontal(lista); break;
            case 2:  nova = criar_obstaculo_inclinado(lista);  break;
            default: nova = criar_obstaculo_giratorio(lista);  break;
        }
        if (nova != lista) return nova;
    }
    return lista;
}

void desenhar_tela_inicial(int scores[], int qtd)
{
    const char *titulo = "REBEL CHICKEN";
    const char *aviso  = "Aperte ESPACO para iniciar";
    const char *dica   = "Segure ESPACO para subir, solte para descer";

    int larg_titulo = MeasureText(titulo, 60);
    DrawText(titulo, (LARGURA - larg_titulo) / 2 + 3, 63, 60, BLACK);
    DrawText(titulo, (LARGURA - larg_titulo) / 2,     60, 60, WHITE);

    if ((int)(GetTime() * 2) % 2 == 0) {
        int larg_aviso = MeasureText(aviso, 26);
        DrawText(aviso, (LARGURA - larg_aviso) / 2 + 2, 152, 26, BLACK);
        DrawText(aviso, (LARGURA - larg_aviso) / 2,     150, 26, YELLOW);
    }

    int larg_dica = MeasureText(dica, 18);
    DrawText(dica, (LARGURA - larg_dica) / 2, 190, 18, DARKGRAY);

    if (qtd > 0) {
        const char *rec = TextFormat("Melhor pontuacao: %d", scores[0]);
        int larg_rec = MeasureText(rec, 20);
        DrawText(rec, (LARGURA - larg_rec) / 2, 230, 20, BLACK);
    }
}

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Rebel Chicken");
    SetTargetFPS(60);

    InitAudioDevice();
    Music trilha = LoadMusicStream("assets/musica.mp3");
    trilha.looping = true;
    SetMusicVolume(trilha, 0.5f);
    PlayMusicStream(trilha);

    EstadoJogo estado = TELA_INICIAL;

    Elemento *lista = NULL;
    Rectangle galinha = { 100, 200, 36, 36 };

    float vel_y = 0;
    float deslocamento = 0;
    float timer = 0;
    int morreu = 0;

    float tempo_jogado = 0;
    float vel_cenario = VEL_X_INICIAL;
    int pontos = 0;

    int milho = 0;
    float timer_milho = 0;
    float intervalo_milho = 2.5f;

    float timer_falcao = 0;
    float intervalo_falcao = 3.2f;
    int   aviso_ativo = 0;
    float aviso_timer = 0;
    float faixa_y = 0, faixa_alt = 0;
    int   qtd_falcoes = 1;

    int scores[MAX_SCORES];
    int qtd_scores = carregar_scores(scores);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateMusicStream(trilha);


        if (estado == TELA_INICIAL) {
            if (IsKeyPressed(KEY_SPACE)) estado = JOGANDO;
        }
        else if (!morreu) {
            tempo_jogado += dt;

            if (tempo_jogado <= 45.0f) {
                vel_cenario = VEL_X_INICIAL + ((VEL_X_MAXIMA - VEL_X_INICIAL) * (tempo_jogado / 45.0f));
            } else {
                vel_cenario = VEL_X_MAXIMA;
            }

            if (IsKeyDown(KEY_SPACE)) vel_y += SOBE * dt;
            else vel_y += DESCE * dt;

            if (vel_y >  VEL_MAXIMA) vel_y =  VEL_MAXIMA;
            if (vel_y < -VEL_MAXIMA) vel_y = -VEL_MAXIMA;

            galinha.y += vel_y * dt;

            if (galinha.y < 0) { galinha.y = 0; vel_y = 0; }
            if (galinha.y + galinha.height > CHAO_Y) {
                galinha.y = CHAO_Y - galinha.height;
                vel_y = 0;
            }

            deslocamento += vel_cenario * dt;
            timer += dt;

            if (timer >= 1.6f) {
                lista = criar_obstaculo(lista);
                timer = 0;
            }

            timer_falcao += dt;
            if (!aviso_ativo && tempo_jogado >= TEMPO_INICIO_FALCAO && timer_falcao >= intervalo_falcao) {
                aviso_ativo = 1;
                aviso_timer = 0;

                int nivel = (int)(tempo_jogado / SEGUNDOS_POR_NIVEL);
                int max_qtd = 1 + nivel;
                if (max_qtd > MAX_FALCOES) max_qtd = MAX_FALCOES;
                qtd_falcoes = GetRandomValue(1, max_qtd);

                faixa_alt = (float)(qtd_falcoes * FALCAO_ALT + (qtd_falcoes - 1) * GAP_FALCAO);
                faixa_y = galinha.y + galinha.height / 2 - faixa_alt / 2;
                if (faixa_y < 0) faixa_y = 0;
                if (faixa_y + faixa_alt > CHAO_Y) faixa_y = CHAO_Y - faixa_alt;
            }
            if (aviso_ativo) {
                aviso_timer += dt;
                if (aviso_timer >= AVISO_TEMPO) {
                    lista = criar_onda_falcoes(lista, faixa_y, qtd_falcoes);
                    aviso_ativo = 0;
                    timer_falcao = 0;
                    intervalo_falcao = (float) GetRandomValue(22, 42) / 10.0f;
                }
            }

            timer_milho += dt;
            if (timer_milho >= intervalo_milho) {
                lista = criar_grupo_milho(lista);
                timer_milho = 0;
                intervalo_milho = (float) GetRandomValue(18, 35) / 10.0f;
            }

            lista = atualizar(lista, dt, vel_cenario);
            morreu = colidiu(lista, galinha);

            lista = coletar_milho(lista, galinha, &milho);
            pontos = (int)(deslocamento / 100) + milho * PONTOS_POR_MILHO;

            if (morreu) {
                registrar_score(scores, &qtd_scores, pontos);
                StopMusicStream(trilha);
            }
        }
        else if (IsKeyPressed(KEY_R)) {
            SeekMusicStream(trilha, 0.0f);
            lista = liberar_lista(lista);
            galinha.y = 200;
            vel_y = 0;
            deslocamento = 0;
            pontos = 0;
            timer = 0;
            tempo_jogado = 0;
            vel_cenario = VEL_X_INICIAL;
            morreu = 0;
            milho = 0;
            timer_milho = 0;
            intervalo_milho = 2.5f;
            timer_falcao = 0;
            intervalo_falcao = 3.2f;
            aviso_ativo = 0;
            aviso_timer = 0;
            PlayMusicStream(trilha);
        }

        BeginDrawing();

        desenhar_cenario(deslocamento);

        desenhar_lista(lista);
        if (!morreu && aviso_ativo) desenhar_alerta(faixa_y, faixa_alt, qtd_falcoes, aviso_timer);
        desenhar_galinha((int) galinha.x, (int) galinha.y);

        if (estado == TELA_INICIAL) {
            desenhar_tela_inicial(scores, qtd_scores);
        } else {
            DrawText(TextFormat("Pontos: %d", pontos), 10, 10, 24, BLACK);
            DrawText(TextFormat("Milho: %d", milho), 10, 38, 24, YELLOW);
        }

        if (morreu) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.7f));
            DrawText("GAME OVER", LARGURA / 2 - 105, ALTURA / 2 - 100, 40, RED);
            DrawText("Pressione R para reiniciar", LARGURA / 2 - 128, ALTURA / 2 - 50, 20, WHITE);
            DrawText(TextFormat("Sua pontuacao: %d", pontos), LARGURA / 2 - 95, ALTURA / 2 - 20, 22, WHITE);
            DrawText("TOP 5", LARGURA / 2 - 25, ALTURA / 2 + 20, 18, WHITE);
            for (int i = 0; i < qtd_scores; i++) {
                DrawText(TextFormat("%d. %d", i + 1, scores[i]),
                         LARGURA / 2 - 20, ALTURA / 2 + 45 + i * 20, 18, WHITE);
            }
        }
        EndDrawing();
    }

    lista = liberar_lista(lista);
    UnloadMusicStream(trilha);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}