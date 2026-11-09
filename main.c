/*
 * Rebel Chicken - Versao 1 (raylib)
 * Escopo: galinha + cenario rolando + arame farpado + milho
 *         + colisoes + LISTA ENCADEADA para obstaculos e milhos.
 */

#include "raylib.h"
#include <stdlib.h>   // malloc, free

/* ---------- CONSTANTES ---------- */
#define LARGURA       800
#define ALTURA        450
#define CHAO_Y        420      // y onde comeca o chao
#define VEL_CENARIO   250.0f   // pixels por segundo
#define SOBE         -1100.0f  // aceleracao com ESPACO pressionado
#define DESCE         900.0f   // aceleracao ("gravidade") sem ESPACO
#define VEL_MAXIMA    380.0f   // limite da velocidade vertical

/* ---------- ESTRUTURAS ---------- */
typedef enum { OBSTACULO, MILHO } TipoElemento;

// Cada NO da lista guarda um elemento do jogo.
// "prox" aponta para o proximo no (ou NULL se for o ultimo).
typedef struct Elemento {
    TipoElemento tipo;
    Rectangle caixa;            // x, y, width, height
    struct Elemento *prox;
} Elemento;

typedef enum { JOGANDO, GAME_OVER } Estado;

/* ---------- FUNCOES DA LISTA ENCADEADA ---------- */

// Cria um no com malloc e coloca no INICIO da lista.
// Devolve o novo inicio da lista.
Elemento *inserir(Elemento *inicio, TipoElemento tipo,
                  float x, float y, float larg, float alt)
{
    Elemento *novo = (Elemento *) malloc(sizeof(Elemento));
    if (novo == NULL) return inicio;      // sem memoria: nao insere

    novo->tipo = tipo;
    novo->caixa = (Rectangle){ x, y, larg, alt };
    novo->prox = inicio;                  // o novo aponta para o antigo primeiro
    return novo;                          // o novo passa a ser o primeiro
}

// Percorre a lista movendo cada elemento para a esquerda.
// Se saiu da tela, REMOVE o no da lista e libera a memoria (free).
Elemento *atualizar(Elemento *inicio, float dt)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        atual->caixa.x -= VEL_CENARIO * dt;

        if (atual->caixa.x + atual->caixa.width < 0) {
            Elemento *remover = atual;
            if (anterior == NULL) inicio = atual->prox;   // era o primeiro
            else anterior->prox = atual->prox;            // "pula" o removido
            atual = atual->prox;
            free(remover);
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
    return inicio;
}

// Percorre a lista verificando colisao com a galinha.
//  - Milho tocado: remove da lista e soma em *milho
//  - Obstaculo tocado: *morreu = 1
// (milho e morreu sao PONTEIROS para variaveis do main)
Elemento *verificar_colisoes(Elemento *inicio, Rectangle galinha,
                             int *milho, int *morreu)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        if (CheckCollisionRecs(galinha, atual->caixa)) {
            if (atual->tipo == OBSTACULO) {
                *morreu = 1;
                anterior = atual;
                atual = atual->prox;
            } else {                                      // MILHO
                (*milho)++;
                Elemento *remover = atual;
                if (anterior == NULL) inicio = atual->prox;
                else anterior->prox = atual->prox;
                atual = atual->prox;
                free(remover);
            }
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
    return inicio;
}

// Desenha todos os elementos da lista.
void desenhar_lista(Elemento *inicio)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if (p->tipo == OBSTACULO) {
            DrawRectangleRec(p->caixa, DARKGRAY);
            DrawRectangleLinesEx(p->caixa, 2, BLACK);
        } else {
            DrawCircle((int)(p->caixa.x + p->caixa.width / 2),
                       (int)(p->caixa.y + p->caixa.height / 2),
                       p->caixa.width / 2, GOLD);
        }
    }
}

// Libera TODOS os nos (usar ao reiniciar e ao fechar o jogo).
Elemento *liberar_lista(Elemento *inicio)
{
    while (inicio != NULL) {
        Elemento *proximo = inicio->prox;
        free(inicio);
        inicio = proximo;
    }
    return NULL;
}

/* ---------- SPAWN (cria elementos novos) ---------- */

Elemento *criar_obstaculo(Elemento *lista)
{
    float alt = (float) GetRandomValue(70, 150);
    float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
    return inserir(lista, OBSTACULO, LARGURA, y, 22, alt);
}

Elemento *criar_fileira_milho(Elemento *lista)
{
    float y = (float) GetRandomValue(30, CHAO_Y - 50);
    for (int i = 0; i < 5; i++) {
        lista = inserir(lista, MILHO, LARGURA + i * 35, y, 18, 18);
    }
    return lista;
}

/* ---------- MAIN ---------- */

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Rebel Chicken");
    SetTargetFPS(60);

    Elemento *lista = NULL;                 // lista vazia
    Rectangle galinha = { 100, 200, 40, 40 };
    float vel_y = 0;
    int milho = 0;
    int morreu = 0;
    float deslocamento = 0;                 // para o cenario rolar
    float timer_obstaculo = 0, timer_milho = 0;
    Estado estado = JOGANDO;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* ---- 1. ENTRADA + 2. ATUALIZACAO ---- */
        if (estado == JOGANDO) {
            // Galinha: espaco sobe, soltar desce
            if (IsKeyDown(KEY_SPACE)) vel_y += SOBE * dt;
            else                      vel_y += DESCE * dt;
            if (vel_y >  VEL_MAXIMA) vel_y =  VEL_MAXIMA;
            if (vel_y < -VEL_MAXIMA) vel_y = -VEL_MAXIMA;
            galinha.y += vel_y * dt;

            // Limites: teto e chao
            if (galinha.y < 0) { galinha.y = 0; vel_y = 0; }
            if (galinha.y + galinha.height > CHAO_Y) {
                galinha.y = CHAO_Y - galinha.height;
                vel_y = 0;
            }

            deslocamento += VEL_CENARIO * dt;

            // Timers de aparecimento
            timer_obstaculo += dt;
            timer_milho += dt;
            if (timer_obstaculo >= 1.6f) {
                lista = criar_obstaculo(lista);
                timer_obstaculo = 0;
            }
            if (timer_milho >= 2.5f) {
                lista = criar_fileira_milho(lista);
                timer_milho = 0;
            }

            // Lista: mover/remover e colisoes
            lista = atualizar(lista, dt);
            lista = verificar_colisoes(lista, galinha, &milho, &morreu);

            if (morreu) estado = GAME_OVER;
        }
        else if (IsKeyPressed(KEY_R)) {     // reiniciar
            lista = liberar_lista(lista);
            galinha.y = 200;
            vel_y = 0;
            milho = 0;
            morreu = 0;
            timer_obstaculo = 0;
            timer_milho = 0;
            estado = JOGANDO;
        }

        /* ---- 3. DESENHO ---- */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // chao
        DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, DARKGREEN);
        // estacas da cerca rolando (da a sensacao de movimento)
        int desloc_cerca = (int) deslocamento % 100;
        for (int i = -1; i <= LARGURA / 100 + 1; i++) {
            DrawRectangle(i * 100 - desloc_cerca, CHAO_Y - 25, 8, 25, BROWN);
        }

        desenhar_lista(lista);
        DrawRectangleRec(galinha, WHITE);               // galinha (por enquanto um quadrado)
        DrawRectangleLinesEx(galinha, 2, ORANGE);

        DrawText(TextFormat("Milho: %d", milho), 10, 10, 24, BLACK);

        if (estado == GAME_OVER) {
            DrawText("GAME OVER", LARGURA / 2 - 110, ALTURA / 2 - 30, 40, RED);
            DrawText("Pressione R para reiniciar", LARGURA / 2 - 130,
                     ALTURA / 2 + 20, 20, BLACK);
        }
        EndDrawing();
    }

    lista = liberar_lista(lista);   // nao deixar memoria vazando
    CloseWindow();
    return 0;
}
