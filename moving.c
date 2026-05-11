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

// careegar testura de fundo (Imagem)
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

void desenharMaca(int x, int y) {

    float tam = 2.0f / GRID_SIZE;

    // Centro da célula
    float centroX = -1.0f + x * tam + tam / 2;
    float centroY = -1.0f + y * tam + tam / 2;

    // =========================
    // CORPO DA MAÇÃ
    // =========================

    float raio = tam * 0.30f;

    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);

        for(int i = 0; i <= 40; i++) {

            float ang = 2.0f * 3.14159f * i / 40;

            glVertex2f(
                centroX + cos(ang) * raio,
                centroY + sin(ang) * raio
            );
        }

    glEnd();



    // =========================
    // CABO DA MAÇÃ
    // =========================

    glColor3f(0.4f, 0.2f, 0.0f);

    glBegin(GL_QUADS);

        glVertex2f(centroX - 0.005f, centroY + raio * 0.8f);
        glVertex2f(centroX + 0.005f, centroY + raio * 0.8f);

        glVertex2f(centroX + 0.005f, centroY + raio * 1.3f);
        glVertex2f(centroX - 0.005f, centroY + raio * 1.3f);

    glEnd();
}
 // DESENAHR A COBRA:
 // função para desenha a cabeça da cobra como um círculo

void desenharCabeca(int x, int y) {

    // =========================================
    // TAMANHO DA CÉLULA E CENTRO DA CABEÇA
    // =========================================

    float tamanhoCelula = 2.0f / GRID_SIZE;

    float centroX = -1.0f + x * tamanhoCelula + tamanhoCelula / 2.0f;
    float centroY = -1.0f + y * tamanhoCelula + tamanhoCelula / 2.0f;

    float raioCabeca = tamanhoCelula * 0.55f;

    // =========================================
    // DESENHAR CABEÇA
    // =========================================

    glColor3f(0.0f, 0.8f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);

        for(int i = 0; i <= 40; i++) {

            float angulo = 2.0f * 3.14159f * i / 40;

            glVertex2f(
                centroX + cos(angulo) * raioCabeca,
                centroY + sin(angulo) * raioCabeca
            );
        }

    glEnd();

    // =========================================
    // DESENHAR LÍNGUA
    // =========================================

    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLES);

        // DIREITA
        if(dirX == 1) {

            glVertex2f(centroX + raioCabeca, centroY);

            glVertex2f(
                centroX + raioCabeca + 0.03f,
                centroY + 0.01f
            );

            glVertex2f(
                centroX + raioCabeca + 0.03f,
                centroY - 0.01f
            );
        }

        // ESQUERDA
        else if(dirX == -1) {

            glVertex2f(centroX - raioCabeca, centroY);

            glVertex2f(
                centroX - raioCabeca - 0.03f,
                centroY + 0.01f
            );

            glVertex2f(
                centroX - raioCabeca - 0.03f,
                centroY - 0.01f
            );
        }

        // CIMA
        else if(dirY == 1) {

            glVertex2f(centroX, centroY + raioCabeca);

            glVertex2f(
                centroX - 0.01f,
                centroY + raioCabeca + 0.03f
            );

            glVertex2f(
                centroX + 0.01f,
                centroY + raioCabeca + 0.03f
            );
        }

        // BAIXO
        else if(dirY == -1) {

            glVertex2f(centroX, centroY - raioCabeca);

            glVertex2f(
                centroX - 0.01f,
                centroY - raioCabeca - 0.03f
            );

            glVertex2f(
                centroX + 0.01f,
                centroY - raioCabeca - 0.03f
            );
        }

    glEnd();

    // =========================================
    // POSIÇÃO DOS OLHOS
    // =========================================

    float olho1X = 0.0f;
    float olho1Y = 0.0f;

    float olho2X = 0.0f;
    float olho2Y = 0.0f;

    // DIREITA
    if(dirX == 1) {

        olho1X = 0.015f;
        olho1Y = 0.015f;

        olho2X = 0.015f;
        olho2Y = -0.015f;
    }

    // ESQUERDA
    else if(dirX == -1) {

        olho1X = -0.015f;
        olho1Y = 0.015f;

        olho2X = -0.015f;
        olho2Y = -0.015f;
    }

    // CIMA
    else if(dirY == 1) {

        olho1X = -0.015f;
        olho1Y = 0.015f;

        olho2X = 0.015f;
        olho2Y = 0.015f;
    }

    // BAIXO
    else if(dirY == -1) {

        olho1X = -0.015f;
        olho1Y = -0.015f;

        olho2X = 0.015f;
        olho2Y = -0.015f;
    }

    // =========================================
    // DESENHAR OLHOS
    // =========================================

    for(int olho = 0; olho < 2; olho++) {

        float offsetX = (olho == 0) ? olho1X : olho2X;
        float offsetY = (olho == 0) ? olho1Y : olho2Y;

        // =====================
        // BRANCO DO OLHO
        // =====================

        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLE_FAN);

            glVertex2f(
                centroX + offsetX,
                centroY + offsetY
            );

            for(int i = 0; i <= 20; i++) {

                float angulo = 2.0f * 3.14159f * i / 20;

                glVertex2f(
                    centroX + offsetX + cos(angulo) * 0.008f,
                    centroY + offsetY + sin(angulo) * 0.008f
                );
            }

        glEnd();

        // =====================
        // PUPILA
        // =====================

        glColor3f(0.0f, 0.0f, 0.0f);

        glBegin(GL_TRIANGLE_FAN);

            glVertex2f(
                centroX + offsetX,
                centroY + offsetY
            );

            for(int i = 0; i <= 20; i++) {

                float angulo = 2.0f * 3.14159f * i / 20;

                glVertex2f(
                    centroX + offsetX + cos(angulo) * 0.004f,
                    centroY + offsetY + sin(angulo) * 0.004f
                );
            }

        glEnd();
    }
}

// função para desenhar o corpo da cobra como um quadrado
void desenharCorpo(int x, int y) {

    float tam = 2.0f / GRID_SIZE;

    float px = -1.0f + x * tam;
    float py = -1.0f + y * tam;

    glColor3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

        glVertex2f(px, py);
        glVertex2f(px + tam, py);
        glVertex2f(px + tam, py + tam);
        glVertex2f(px, py + tam);

    glEnd();
}


// função para desenhar texto
void desenharTexto(float x, float y, char *string) {

    // Define cor do texto
    glColor3f(0, 0, 0);

    // Posiciona texto na tela
    glRasterPos2f(x, y);

    // Desenha caractere por caractere
    while(*string) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *string);
        string++;
    }
}


// Função de renderização do jogo
void display() {
    // Limpa a tela
    glClear(GL_COLOR_BUFFER_BIT);


    // desenha fundo
    desenharFundo();
    

    // Desenha comida
    desenharMaca(comidaX, comidaY);

    // Desenha cobra
    for(int i = 0; i < tamanho; i++) {

    // Cabeça
    if(i == 0) {
        desenharCabeca(cobra[i].x, cobra[i].y);
    }

    // Corpo
    else {
        desenharCorpo(cobra[i].x, cobra[i].y);
    }
}

    // TEXTO DA PONTUAÇÃO

    glColor3f(1, 1, 1);

    
    char texto[50];

    sprintf(texto, "Pontuacao: %d", score);

    desenharTexto(-0.95f, 0.92f, texto);

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