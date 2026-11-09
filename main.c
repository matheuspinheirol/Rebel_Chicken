#include "raylib.h"
#include <stdlib.h>  

#define LARGURA       800
#define ALTURA        450
#define CHAO_Y        420     
#define VEL_CENARIO   250.0f   
#define SOBE         -1100.0f  
#define DESCE         900.0f   
#define VEL_MAXIMA    380.0f   

typedef struct Elemento {
    Rectangle caixa;            
    struct Elemento *prox;
} Elemento;

Elemento *inserir(Elemento *inicio, float x, float y, float larg, float alt)
{
    Elemento *novo = (Elemento *) malloc(sizeof(Elemento));
    if (novo == NULL) return inicio;      

    novo->caixa = (Rectangle){ x, y, larg, alt };
    novo->prox = inicio;                  
    return novo;                          
}

Elemento *atualizar(Elemento *inicio, float dt)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        atual->caixa.x -= VEL_CENARIO * dt;

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
        if (CheckCollisionRecs(galinha, p->caixa)) return 1;
    }
    return 0;
}


void desenhar_lista(Elemento *inicio)
{
    for (Elemento *p = inicio; p != NULL; p = p->prox) {
        DrawRectangleRec(p->caixa, DARKGRAY);
        DrawRectangleLinesEx(p->caixa, 2, BLACK);
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

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Rebel Chicken");
    SetTargetFPS(60);

    Elemento *lista = NULL;                
    Rectangle galinha = { 100, 200, 40, 40 };
    float vel_y = 0;
    float deslocamento = 0;       
    float timer = 0;                   
    int morreu = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (!morreu) {
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

            deslocamento += VEL_CENARIO * dt;
            timer += dt;
            if (timer >= 1.6f) {
                float alt = (float) GetRandomValue(70, 150);
                float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
                lista = inserir(lista, LARGURA, y, 22, alt);
                timer = 0;
            }

            lista = atualizar(lista, dt);
            morreu = colidiu(lista, galinha);
        }
        BeginDrawing();
        ClearBackground(SKYBLUE);
        DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, DARKGREEN);
        int desloc_cerca = (int) deslocamento % 100;
        for (int i = -1; i <= LARGURA / 100 + 1; i++) {
            DrawRectangle(i * 100 - desloc_cerca, CHAO_Y - 25, 8, 25, BROWN);
        }

        desenhar_lista(lista);
        DrawRectangleRec(galinha, WHITE);            
        DrawRectangleLinesEx(galinha, 2, ORANGE);

        if (morreu) {
            DrawText("GAME OVER", LARGURA / 2 - 110, ALTURA / 2 - 20, 40, RED);
        }
        EndDrawing();
    }
    lista = liberar_lista(lista); 
    CloseWindow();
    return 0;
}