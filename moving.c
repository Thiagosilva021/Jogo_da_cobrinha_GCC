//gcc moving.c -Iinclude -Llib/x64 -lfreeglut -lopengl32 -lglu32 -o moving.exe

#include <GL/freeglut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "snake_background.h"

// Define o tamanho da tela do jogo
#define GRID_SIZE 20
// Define o tamanho máximo da cobra
#define MAX_SNAKE 400

// Estrutura para representar cada segmento da cobra
typedef struct {
    int x, y;
} Segmento;

// vetor que guarda os segmentos da cobra
Segmento cobra[MAX_SNAKE];

// Tamanho inicial da cobra
int tamanho = 3;

// Direção inicial da cobra (movendo para a direita)
int dirX = 1;
int dirY = 0;

// Posição da comida
int comidaX = 10;
int comidaY = 10;

// Variável para controlar o estado do jogo
int gameOver = 0;
int score = 0;

// careegar testura
GLuint texturaFundo;

void carregarTexturaDoArray() {
    glGenTextures(1, &texturaFundo);
    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        BG_WIDTH,
        BG_HEIGHT,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        BG_PIXELS
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// desenhar imagem 
void desenharFundo() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    glColor3f(1,1,1);

    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex2f(-1,-1);
        glTexCoord2f(1,1); glVertex2f( 1,-1);
        glTexCoord2f(1,0); glVertex2f( 1, 1);
        glTexCoord2f(0,0); glVertex2f(-1, 1);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Função para gerar uma nova posição para a comida
void gerarComida() {
    comidaX = rand() % GRID_SIZE;
    comidaY = rand() % GRID_SIZE;
}

void desenharComidaBolinha(int x, int y) {
    float tamCelula = 2.0f / GRID_SIZE;

    // Centro da célula
    float centroX = -1.0f + x * tamCelula + tamCelula / 2.0f;
    float centroY = -1.0f + y * tamCelula + tamCelula / 2.0f;

    // Raio da bolinha
    float raio = tamCelula * 0.25f;

    glColor3f(1, 1, 1);

    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centroX, centroY);

        for(int i = 0; i <= 30; i++) {
            float ang = 2.0f * 3.14159f * i / 30;

            glVertex2f(
                centroX + cos(ang) * raio,
                centroY + sin(ang) * raio
            );
        }

    glEnd();
}

void desenharQuadrado(int x, int y, float r, float g, float b) {
    float tam = 2.0f / GRID_SIZE;

    float px = -1.0f + x * tam;
    float py = -1.0f + y * tam;

    glColor3f(r, g, b);

    glBegin(GL_QUADS);
        glVertex2f(px, py);
        glVertex2f(px + tam, py);
        glVertex2f(px + tam, py + tam);
        glVertex2f(px, py + tam);
    glEnd();
}

// Função de renderização do jogo
void display() {
    // Limpa a tela
    glClear(GL_COLOR_BUFFER_BIT);


    // desenha fundo
    desenharFundo();


    // Desenha comida
    desenharComidaBolinha(comidaX, comidaY);

    // Desenha cobra
    for(int i = 0; i < tamanho; i++) {
    if(i == 0)
        desenharQuadrado(cobra[i].x, cobra[i].y, 1, 1, 0); // cabeça
    else
        desenharQuadrado(cobra[i].x, cobra[i].y, 0, 1, 0);   // corpo
    }

    // atualiza a tela
    glFlush();
}

// Função para atualizar o jogo (executada a cada intervalo de tempo)
void atualizar(int value) {

    // Se o jogo acabou, não atualiza mais
    if(gameOver) return;

    // Move corpo
    // Move cada segmento para a posição do segmento anterior
    for(int i = tamanho; i > 0; i--) {
        cobra[i] = cobra[i - 1];
    }

    // Move cabeça
    cobra[0].x += dirX;
    cobra[0].y += dirY;

    // Colisão com parede
    if(cobra[0].x < 0 || cobra[0].x >= GRID_SIZE ||
       cobra[0].y < 0 || cobra[0].y >= GRID_SIZE) {
        gameOver = 1;
        printf("Game Over! Score: %d\n", score);
        return;
    }

    // Colisão com o proprio corpo
    for(int i = 1; i < tamanho; i++) {
        if(cobra[0].x == cobra[i].x &&
           cobra[0].y == cobra[i].y) {
            gameOver = 1;
            printf("Game Over! Score: %d\n", score);
            return;
        }
    }

    // Verifica se comeu a comida
    if(cobra[0].x == comidaX && cobra[0].y == comidaY) {
        tamanho++;// aumenta o tamanho da cobra
        score++;// aumenta a pontuação
        gerarComida();// gera nova comida
    }

    // Redesenha a tela
    glutPostRedisplay();
    // Agenda a próxima atualização
    glutTimerFunc(150, atualizar, 0);
}

// Função para lidar com a entrada do teclado
void teclado(unsigned char key, int x, int y) {
    switch(key) {
        case 'w':
            // Evita que a cobra se mova na direção oposta
            if(dirY != -1){ dirX = 0; dirY = 1; }
            break;
        case 's':
            if(dirY != 1){ dirX = 0; dirY = -1; }
            break;
        case 'a':
            if(dirX != 1){ dirX = -1; dirY = 0; }
            break;
        case 'd':
            if(dirX != -1){ dirX = 1; dirY = 0; }
            break;
    }
}

// Função para inicializar a cobra com os segmentos iniciais
void initSnake() {
    cobra[0] = (Segmento){5, 5};// cabeça
    cobra[1] = (Segmento){4, 5};// corpo
    cobra[2] = (Segmento){3, 5};// corpo
}

// Função principal
int main(int argc, char** argv) {
    

    // Inicializa a semente para geração de números aleatórios
    srand(time(NULL));

    // Inicializa a janela do jogo
    glutInit(&argc, argv);
    // Configura o modo de exibição (tela única e RGB)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    // Configura o tamanho da janela
    glutInitWindowSize(800, 800);
    // Configura a posição da janela
    glutCreateWindow("Jogo da Cobrinha");

    // carrega fundo
    carregarTexturaDoArray();

    // Configura a cor de fundo da janela (preto)
    //initSnake();

    // Registra as funções de callback
    glutDisplayFunc(display);
    // Registra a função de teclado para lidar com a entrada do usuário
    glutKeyboardFunc(teclado);
    // Inicia o loop de atualização do jogo
    glutTimerFunc(150, atualizar, 0);

    // Inicia o loop principal do OpenGL
    glutMainLoop();
    return 0;
}