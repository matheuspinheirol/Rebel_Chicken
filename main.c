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

#define MAX_SCORES     5
#define ARQUIVO_SCORES "top_scores.txt"

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
    float bater = sinf((float) GetTime() * 16.0f);          // -1 .. 1

    // cauda (atras = lado direito)
    desenhar_triangulo((Vector2){ x + w - 12, y + 15 },
                       (Vector2){ x + w + 6,  y + 9 },
                       (Vector2){ x + w + 6,  y + 23 }, DARKBROWN);
    // asa batendo (varrida para tras)
    desenhar_triangulo((Vector2){ x + 22, y + 15 },
                       (Vector2){ x + 46, y + 15 },
                       (Vector2){ x + 44, y + 14 - 18 * bater }, BROWN);
    // corpo e peito claro
    DrawEllipse(x + 32, y + 17, 24, 9, BROWN);
    DrawEllipse(x + 30, y + 20, 16, 5, BEIGE);
    // cabeca
    DrawCircle(x + 13, y + 13, 9, DARKBROWN);
    // bico (aponta para a esquerda)
    desenhar_triangulo((Vector2){ x - 3, y + 14 },
                       (Vector2){ x + 6, y + 10 },
                       (Vector2){ x + 6, y + 18 }, GOLD);
    // olho bravo
    DrawCircle(x + 11, y + 11, 3, WHITE);
    DrawCircle(x + 10, y + 11, 1.5f, BLACK);
    DrawLine(x + 6, y + 5, x + 15, y + 9, BLACK);
}

typedef enum { OBSTACULO, MILHO, FALCAO } TipoElemento;

typedef struct Elemento {
    TipoElemento tipo;          
    Rectangle caixa;            
    struct Elemento *prox;
} Elemento;

Elemento *inserir(Elemento *inicio, float x, float y, float larg, float alt)
{
    Elemento *novo = (Elemento *) malloc(sizeof(Elemento));
    if (novo == NULL) return inicio;      

    novo->caixa = (Rectangle){ x, y, larg, alt };
    novo->tipo = OBSTACULO;               
    novo->prox = inicio;                  
    return novo;                          
}

Elemento *inserir_falcao(Elemento *inicio, float x, float y)
{
    Elemento *novo = inserir(inicio, x, y, FALCAO_LARG, FALCAO_ALT);
    if (novo != inicio) novo->tipo = FALCAO;
    return novo;
}

Elemento *atualizar(Elemento *inicio, float dt, float vel_cenario)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        float multiplicador = (atual->tipo == FALCAO) ? 1.35f : 1.0f;
        atual->caixa.x -= vel_cenario * multiplicador * dt;

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

int colidiu(Elemento *inicio, Rectangle galinha)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if ((p->tipo == OBSTACULO || p->tipo == FALCAO) && CheckCollisionRecs(galinha, p->caixa)) {
            return 1;
        }
    }
    return 0;
}

void desenhar_lista(Elemento *inicio)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        if (p->tipo == MILHO) {
            desenhar_milho(p->caixa);
        } else if (p->tipo == FALCAO) {
            desenhar_falcao(p->caixa);
        } else {
            DrawRectangleRec(p->caixa, DARKGRAY);
            DrawRectangleLinesEx(p->caixa, 2, BLACK);
        }
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

int area_livre(Elemento *lista, Rectangle area)
{
    Rectangle a = com_folga(area);
    for (Elemento *p = lista; p != NULL; p = p->prox) {
        if (CheckCollisionRecs(a, com_folga(p->caixa))) return 0;
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

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Rebel Chicken");
    SetTargetFPS(60);

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

    int scores[MAX_SCORES];
    int qtd_scores = carregar_scores(scores);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        if (!morreu) {
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
            pontos = (int)(deslocamento / 100) + milho * PONTOS_POR_MILHO;
            timer += dt;
            
            if (timer >= 1.6f) {
                for (int tentativa = 0; tentativa < 10; tentativa++) {
                    float alt = (float) GetRandomValue(70, 150);
                    float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
                    if (area_livre(lista, (Rectangle){ LARGURA, y, 22, alt })) {
                        lista = inserir(lista, LARGURA, y, 22, alt);
                        break;
                    }
                }
                timer = 0;
            }

            timer_falcao += dt;
            if (timer_falcao >= intervalo_falcao) {
                for (int tentativa = 0; tentativa < 10; tentativa++) {
                    float y = (float) GetRandomValue(20, CHAO_Y - FALCAO_ALT - 20);
                    if (area_livre(lista, (Rectangle){ LARGURA, y, FALCAO_LARG, FALCAO_ALT })) {
                        lista = inserir_falcao(lista, LARGURA, y);
                        break;
                    }
                }
                timer_falcao = 0;
                intervalo_falcao = (float) GetRandomValue(22, 42) / 10.0f;
            }

            timer_milho += dt;
            if (timer_milho >= intervalo_milho) {
                lista = criar_grupo_milho(lista);
                timer_milho = 0;
                intervalo_milho = (float) GetRandomValue(18, 35) / 10.0f;
            }

            lista = atualizar(lista, dt, vel_cenario);
            morreu = colidiu(lista, galinha);

            int antes = milho;
            lista = coletar_milho(lista, galinha, &milho);
            pontos += (milho - antes) * PONTOS_POR_MILHO;

            if (morreu) {
                registrar_score(scores, &qtd_scores, pontos);
            }
        }
        else if (IsKeyPressed(KEY_R)) {
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
        }

        BeginDrawing();
        
        ClearBackground(SKYBLUE);
        DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, DARKGREEN);
        
        int desloc_cerca = (int) deslocamento % 100;
        for (int i = -1; i <= LARGURA / 100 + 1; i++) {
            DrawRectangle(i * 100 - desloc_cerca, CHAO_Y - 25, 8, 25, BROWN);
        }

        desenhar_lista(lista);
        desenhar_galinha((int) galinha.x, (int) galinha.y);
        DrawText(TextFormat("Pontos: %d", pontos), 10, 10, 24, BLACK);
        DrawText(TextFormat("Milho: %d", milho), 10, 38, 24, BLACK);

        if (morreu) {
            DrawText("GAME OVER", LARGURA / 2 - 110, ALTURA / 2 - 20, 40, RED);
            DrawText(TextFormat("Sua pontuacao: %d", pontos), LARGURA / 2 - 110, ALTURA / 2 + 60, 22, BLACK);
            DrawText("Pressione R para reiniciar", LARGURA / 2 - 130, ALTURA / 2 + 30, 20, DARKGRAY);
            DrawText("TOP 5", LARGURA / 2 - 110, ALTURA / 2 + 90, 18, BLACK);
            for (int i = 0; i < qtd_scores; i++) {
                DrawText(TextFormat("%d. %d", i + 1, scores[i]),
                         LARGURA / 2 - 110, ALTURA / 2 + 112 + i * 20, 18, DARKGRAY);
            }
        }
        EndDrawing();
    }
    
    lista = liberar_lista(lista); 
    CloseWindow();
    return 0;
}