#include "raylib.h"
#include <stdlib.h>

#define LARGURA       800
#define ALTURA        450
#define CHAO_Y        420     

#define VEL_X_INICIAL 250.0f
#define VEL_X_MAXIMA  750.0f

#define SOBE         -1400.0f  
#define DESCE         900.0f   
#define VEL_MAXIMA    550.0f   


#define SPRITE_LARG   12
#define SPRITE_ALT    12
#define ESCALA_SPRITE 3    


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
// ---------------------------------------

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

Elemento *atualizar(Elemento *inicio, float dt, float vel_cenario)
{
    Elemento *anterior = NULL;
    Elemento *atual = inicio;

    while (atual != NULL) {
        atual->caixa.x -= vel_cenario * dt;

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
    Rectangle galinha = { 100, 200, 36, 36 };  
    
    float vel_y = 0;
    float deslocamento = 0;        
    float timer = 0;                    
    int morreu = 0;
    
    float tempo_jogado = 0;
    float vel_cenario = VEL_X_INICIAL;

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
            timer += dt;
            
            if (timer >= 1.6f) {
                float alt = (float) GetRandomValue(70, 150);
                float y = (float) GetRandomValue(0, (int)(CHAO_Y - alt));
                lista = inserir(lista, LARGURA, y, 22, alt);
                timer = 0;
            }

            lista = atualizar(lista, dt, vel_cenario);
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
        desenhar_galinha((int) galinha.x, (int) galinha.y);


        if (morreu) {
            DrawText("GAME OVER", LARGURA / 2 - 110, ALTURA / 2 - 20, 40, RED);
        }
        EndDrawing();
    }
    
    lista = liberar_lista(lista); 
    CloseWindow();
    return 0;
}