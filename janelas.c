////gcc janelas.c -Iinclude -Llib/x64 -lfreeglut -lopengl32 -lglu32 -o janelas.exe

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#include "snake_arcade_background.h"

// =====================================
// TAMANHO DA JANELA
// =====================================

int largura = 800;
int altura = 600;

int hover = 0;


// ESTRUTURA DOS BOTÕES


typedef struct {

    float x;
    float y;
    float w;
    float h;

    const char* texto;

} Botao;

// Botões centralizados iguais da imagem
Botao play = { -0.22,  0.40, 0.44, 0.18, "PLAY" };
Botao score = { -0.22,  0.08, 0.44, 0.18, "SCORE" };
Botao skins = { -0.22, -0.24, 0.44, 0.18, "SKINS" };


// DESENHAR TEXTO


void texto(float x, float y, const char* s) {

    glRasterPos2f(x, y);

    while (*s) {

        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s);
        s++;
    }
}

// =====================================
// FUNDO
// =====================================

GLuint texturaFundo;

void carregarTexturaFundo() {

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

    glTexParameteri(GL_TEXTURE_2D,
                     GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D,
                     GL_TEXTURE_MAG_FILTER,
                     GL_LINEAR);
}

void desenharFundo() {

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    glColor3f(0,1,0);

    glBegin(GL_QUADS);

        glTexCoord2f(0,1);
        glVertex2f(-1,-1);

        glTexCoord2f(1,1);
        glVertex2f(1,-1);

        glTexCoord2f(1,0);
        glVertex2f(1,1);

        glTexCoord2f(0,0);
        glVertex2f(-1,1);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// =====================================
// DESENHAR BOTÃO ESTILO PIXEL
// =====================================

void desenharBotao(Botao b, int ativo) {

    // =====================
    // SOMBRA
    // =====================

    glColor3f(0.12, 0.20, 0.32);

    glBegin(GL_QUADS);

    glVertex2f(b.x + 0.015, b.y - 0.015);
    glVertex2f(b.x + b.w + 0.015, b.y - 0.015);
    glVertex2f(b.x + b.w + 0.015, b.y - b.h - 0.015);
    glVertex2f(b.x + 0.015, b.y - b.h - 0.015);

    glEnd();

    // =====================
    // COR PRINCIPAL
    // =====================

    if (ativo)
        glColor3f(0.82, 0.88, 1.0);
    else
        glColor3f(0.72, 0.80, 0.92);

    glBegin(GL_QUADS);

    glVertex2f(b.x, b.y);
    glVertex2f(b.x + b.w, b.y);
    glVertex2f(b.x + b.w, b.y - b.h);
    glVertex2f(b.x, b.y - b.h);

    glEnd();

    // =====================
    // PARTE SUPERIOR CLARA
    // =====================

    glColor3f(0.92, 0.95, 1.0);

    glBegin(GL_QUADS);

    glVertex2f(b.x + 0.01, b.y - 0.01);
    glVertex2f(b.x + b.w - 0.01, b.y - 0.01);
    glVertex2f(b.x + b.w - 0.01, b.y - 0.05);
    glVertex2f(b.x + 0.01, b.y - 0.05);

    glEnd();

    // =====================
    // PARTE INFERIOR ESCURA
    // =====================

    glColor3f(0.30, 0.42, 0.62);

    glBegin(GL_QUADS);

    glVertex2f(b.x, b.y - b.h + 0.03);
    glVertex2f(b.x + b.w, b.y - b.h + 0.03);
    glVertex2f(b.x + b.w, b.y - b.h);
    glVertex2f(b.x, b.y - b.h);

    glEnd();

    // =====================
    // BORDA
    // =====================

    glColor3f(0.14, 0.24, 0.38);

    glLineWidth(5);

    glBegin(GL_LINE_LOOP);

    glVertex2f(b.x, b.y);
    glVertex2f(b.x + b.w, b.y);
    glVertex2f(b.x + b.w, b.y - b.h);
    glVertex2f(b.x, b.y - b.h);

    glEnd();

    // =====================
    // TEXTO
    // =====================

    glColor3f(0.25, 0.35, 0.50);

    texto(b.x + 0.15, b.y - 0.11, b.texto);
}

// =====================================
// DISPLAY
// =====================================

void display() {

    glClear(GL_COLOR_BUFFER_BIT);

    desenharFundo();

    desenharBotao(play, hover == 1);
    desenharBotao(score, hover == 2);
    desenharBotao(skins, hover == 3);

    glutSwapBuffers();
}

// MOVIMENTO DO MOUSE


void movimento(int x, int y) {

    float mx = (float)x / largura * 2 - 1;
    float my = 1 - (float)y / altura * 2;

    hover = 0;

    Botao botoes[3] = { play, score, skins };

    for (int i = 0; i < 3; i++) {

        if (mx >= botoes[i].x &&
            mx <= botoes[i].x + botoes[i].w &&
            my <= botoes[i].y &&
            my >= botoes[i].y - botoes[i].h) {

            hover = i + 1;
        }
    }

    glutPostRedisplay();
}


// CLIQUES


void mouse(int botao, int estado, int x, int y) {

    if (botao == GLUT_LEFT_BUTTON && estado == GLUT_DOWN) {

        if (hover == 1)
            printf("PLAY selecionado!\n");

        if (hover == 2)
            printf("SCORE selecionado!\n");

        if (hover == 3)
            printf("SKINS selecionado!\n");
    }
}


// INIT


void init() {

    glClearColor(0.62, 0.86, 0.28, 1);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluOrtho2D(-1, 1, -1, 1);
}


// MAIN


int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(largura, altura);

    glutCreateWindow("Pixel Menu");

    carregarTexturaFundo();

    init();

    glutDisplayFunc(display);

    glutPassiveMotionFunc(movimento);

    glutMouseFunc(mouse);

    glutMainLoop();

    return 0;
}