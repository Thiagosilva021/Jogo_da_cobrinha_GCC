// ==========================================================
// COMPILAR:
// gcc moving.c -Iinclude -Llib/x64 -lfreeglut -lopengl32 -lglu32 -o moving.exe
// ==========================================================

#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "snake_background.h"

// ==========================================================
// CONFIGURAÇÕES
// ==========================================================

// Tamanho da grade do jogo
#define GRID_SIZE              20
// Tamanho máximo da cobra (número máximo de segmentos)
#define MAX_COBRA              400

// Tamanho da janela
#define LARGURA_JANELA         700
#define ALTURA_JANELA          700

// Intervalo de atualização do jogo (em milissegundos)
#define INTERVALO_ATUALIZACAO  150

#define PI 3.14159f

// ==========================================================
// ESTRUTURAS
// ==========================================================

// Estrutura para representar um segmento da cobra
typedef struct {
    int x;
    int y;
} Segmento;

// ==========================================================
// VARIÁVEIS GLOBAIS
// ==========================================================

// Cobra
static Segmento cobra[MAX_COBRA];

// Tamanho atual da cobra
static int tamanhoCobra = 3;

// Direção atual da cobra
static int direcaoX = 1;
static int direcaoY = 0;

// Posição da comida
static int comidaX = 10;
static int comidaY = 10;

// Pontuação do jogador
static int pontuacao = 0;

// Estado do jogo
static int jogoEncerrado = 0;

// Textura de fundo
static GLuint texturaFundo = 0;

// =====================================
// SCOREBOARD
// =====================================

int melhoresPontuacoes[5] = {0, 0, 0, 0, 0};

// ==========================================================
// PROTÓTIPOS
// ==========================================================

// Função de atualização do jogo
void atualizar(int valor);

// ==========================================================
// FUNÇÕES AUXILIARES
// ==========================================================

// Função para obter o tamanho de cada célula da grade
float obterTamanhoCelula() {
    return 2.0f / GRID_SIZE;
}

// Função para converter coordenada da grade para coordenada de tela
float converterGridParaTelaX(int x) {
    return -1.0f + (x * obterTamanhoCelula());
}

// Função para converter coordenada da grade para coordenada de tela
float converterGridParaTelaY(int y) {
    return -1.0f + (y * obterTamanhoCelula());
}

// ==========================================================
// TEXTURA DE FUNDO
// ==========================================================

// Carrega a textura de fundo a partir dos dados da imagem definidos em snake_background.h, criando uma textura OpenGL e configurando os parâmetros de filtragem para garantir que a textura seja exibida corretamente no jogo
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

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );
}

// Desenha o fundo do jogo usando a textura carregada, mapeando a textura para um quadrado que cobre toda a tela, e garantindo que a textura seja exibida corretamente usando a cor branca para evitar alterações na cor da textura, e desabilitando o uso de texturas após o desenho para evitar afetar outros elementos do jogo
void desenharFundo() {

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f( 1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f( 1.0f,  1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f,  1.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// ==========================================================
// TEXTO
// ==========================================================

// Desenha o texto na tela usando a função glutBitmapCharacter para renderizar cada caractere da string, definindo a posição do texto usando glRasterPos2f e iterando sobre cada caractere da string para desenhá-lo usando a fonte GLUT_BITMAP_HELVETICA_18, garantindo que o texto seja exibido corretamente na posição desejada na tela
void desenharTexto(float x, float y, const char *texto) {

    glRasterPos2f(x, y);

    while (*texto) {

        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            *texto
        );

        texto++;
    }
}

// ==========================================================
// SCOREBOARD
// ==========================================================

// Carrega os recordes do arquivo "recordes.txt" para o array melhoresPontuacoes, lendo as pontuações armazenadas no arquivo e armazenando-as no array para que possam ser exibidas na tela de recordes do jogo, garantindo que os recordes sejam persistidos entre as sessões de jogo e que o arquivo seja fechado corretamente após a leitura para evitar vazamentos de recursos
void carregarRecordes() {

    FILE *arquivo = fopen("recordes.txt", "r");

    if (arquivo == NULL) {
        return;
    }

    for (int i = 0; i < 5; i++) {

        fscanf(
            arquivo,
            "%d",
            &melhoresPontuacoes[i]
        );
    }

    fclose(arquivo);
}

void salvarRecordes() {

    FILE *arquivo = fopen("recordes.txt", "w");

    if (arquivo == NULL) {
        return;
    }

    for (int i = 0; i < 5; i++) {

        fprintf(
            arquivo,
            "%d\n",
            melhoresPontuacoes[i]
        );
    }

    fclose(arquivo);
}

void atualizarRecordes(int novaPontuacao) {

    for (int i = 0; i < 5; i++) {

        if (novaPontuacao > melhoresPontuacoes[i]) {

            for (int j = 4; j > i; j--) {

                melhoresPontuacoes[j] =
                    melhoresPontuacoes[j - 1];
            }

            melhoresPontuacoes[i] =
                novaPontuacao;

            salvarRecordes();

            break;
        }
    }
}

// ==========================================================
// GAME OVER
// ==========================================================

// Desenha a tela de game over com uma mensagem personalizada com base na pontuação do jogador, exibindo a pontuação final e os melhores recordes, e fornecendo opções para o jogador reiniciar o jogo ou sair, criando uma experiência de encerramento de jogo completa que informa o jogador sobre seu desempenho e oferece opções para continuar jogando ou encerrar o jogo de forma visualmente atraente e informativa
void desenharTelaGameOver() {

    // =====================================
    // FUNDO GRADIENTE
    // =====================================

    glBegin(GL_QUADS);

        glColor3f(0.02f, 0.02f, 0.02f);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f( 1.0f, -1.0f);

        glColor3f(0.15f, 0.00f, 0.00f);
        glVertex2f( 1.0f,  1.0f);
        glVertex2f(-1.0f,  1.0f);

    glEnd();

    // =====================================
    // MOLDURA EXTERNA
    // =====================================

    glLineWidth(6);

    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.80f, -0.85f);
        glVertex2f( 0.80f, -0.85f);
        glVertex2f( 0.80f,  0.85f);
        glVertex2f(-0.80f,  0.85f);
    glEnd();

    // =====================================
    // MOLDURA INTERNA
    // =====================================

    glLineWidth(2);

    glColor3f(1.0f, 0.4f, 0.4f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.76f, -0.81f);
        glVertex2f( 0.76f, -0.81f);
        glVertex2f( 0.76f,  0.81f);
        glVertex2f(-0.76f,  0.81f);
    glEnd();

    // =====================================
    // TÍTULO
    // =====================================

    glColor3f(1.0f, 0.1f, 0.1f);

    desenharTexto(
        -0.18f,
        0.65f,
        "SE FUDEU"
    );

    // =====================================
    // CAIXA DA PONTUAÇÃO
    // =====================================

    glColor3f(0.10f, 0.10f, 0.10f);

    glBegin(GL_QUADS);
        glVertex2f(-0.35f, 0.42f);
        glVertex2f( 0.35f, 0.42f);
        glVertex2f( 0.35f, 0.56f);
        glVertex2f(-0.35f, 0.56f);
    glEnd();

    char textoPontuacao[64];

    snprintf(
        textoPontuacao,
        sizeof(textoPontuacao),
        "PONTUACAO: %d",
        pontuacao
    );

    glColor3f(1.0f, 1.0f, 0.0f);

    desenharTexto(
        -0.18f,
        0.48f,
        textoPontuacao
    );

    // =====================================
    // MENSAGEM PERSONALIZADA
    // =====================================

    glColor3f(0.0f, 1.0f, 1.0f);

    if (pontuacao < 10)
    {
        desenharTexto(
            -0.25f,
            0.30f,
            "Voce e muito ruim! KKK"
        );
    }
    else if (pontuacao < 20)
    {
        desenharTexto(
            -0.33f,
            0.30f,
            "Melhorou, mas ainda e fraco!"
        );
    }
    else if (pontuacao < 30)
    {
        desenharTexto(
            -0.20f,
            0.30f,
            "Ta melhorando!"
        );
    }
    else
    {
        desenharTexto(
            -0.25f,
            0.30f,
            "Ta de hack ne patrao?"
        );
    }

    // =====================================
    // LINHA SEPARADORA
    // =====================================

    glColor3f(0.5f, 0.5f, 0.5f);

    glLineWidth(2);

    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.20f);
        glVertex2f( 0.55f, 0.20f);
    glEnd();

    // =====================================
    // CAIXA DO RANKING
    // =====================================

    glColor3f(0.08f, 0.08f, 0.08f);

    glBegin(GL_QUADS);
        glVertex2f(-0.45f, -0.35f);
        glVertex2f( 0.45f, -0.35f);
        glVertex2f( 0.45f,  0.12f);
        glVertex2f(-0.45f,  0.12f);
    glEnd();

    glColor3f(0.0f, 1.0f, 1.0f);

    desenharTexto(
        -0.18f,
        0.05f,
        "RANKING"
    );

    char texto[64];

    for (int i = 0; i < 5; i++)
    {
        snprintf(
            texto,
            sizeof(texto),
            "%d. %d pontos",
            i + 1,
            melhoresPontuacoes[i]
        );

        desenharTexto(
            -0.15f,
            -0.05f - (i * 0.07f),
            texto
        );
    }

    // =====================================
    // CAIXA DOS COMANDOS
    // =====================================

    glColor3f(0.15f, 0.15f, 0.15f);

    glBegin(GL_QUADS);
        glVertex2f(-0.40f, -0.70f);
        glVertex2f( 0.40f, -0.70f);
        glVertex2f( 0.40f, -0.48f);
        glVertex2f(-0.40f, -0.48f);
    glEnd();

    glLineWidth(2);

    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.40f, -0.70f);
        glVertex2f( 0.40f, -0.70f);
        glVertex2f( 0.40f, -0.48f);
        glVertex2f(-0.40f, -0.48f);
    glEnd();

    // =====================================
    // BOTÃO REINICIAR
    // =====================================

    glColor3f(1.0f, 1.0f, 1.0f);

    desenharTexto(
        -0.24f,
        -0.57f,
        "R - JOGAR NOVAMENTE"
    );

    // =====================================
    // BOTÃO SAIR
    // =====================================

    glColor3f(0.8f, 0.1f, 0.1f);

    desenharTexto(
        -0.10f,
        -0.64f,
        "ESC - SAIR"
    );
}

// ==========================================================
// COMIDA
// ==========================================================

// Verifica se a posição (x, y) está ocupada por algum segmento da cobra iterando sobre cada segmento da cobra e comparando suas coordenadas com as coordenadas fornecidas, retornando 1 (verdadeiro) se a posição estiver ocupada por algum segmento da cobra, ou 0 (falso) caso contrário, o que é útil para garantir que a comida seja gerada em uma posição válida que não esteja ocupada pela cobra
int posicaoOcupadaPelaCobra(int x, int y) {

    for (int i = 0; i < tamanhoCobra; i++) {

        if (
            cobra[i].x == x &&
            cobra[i].y == y
        ) {
            return 1;
        }
    }

    return 0;
}

// Gera uma nova posição para a comida, garantindo que não esteja ocupada pela cobra e evitando loops infinitos caso a cobra ocupe quase toda a grade, usando um número máximo de tentativas para garantir que o processo de geração da comida seja eficiente mesmo em situações onde a cobra ocupa uma grande parte da grade
void gerarComida() {

    int tentativaMaxima = 1000;
    int tentativa = 0;

    do {

        comidaX = rand() % GRID_SIZE;
        comidaY = rand() % GRID_SIZE;

        tentativa++;

    } while (
        posicaoOcupadaPelaCobra(comidaX, comidaY) &&
        tentativa < tentativaMaxima
    );
}

// Desenha a maçã na posição especificada, usando um círculo vermelho para representar a maçã e um retângulo marrom para representar o cabo da maçã, garantindo que a maçã seja desenhada de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas
void desenharMaca(int x, int y) {

    float tamanhoCelula = obterTamanhoCelula();

    float centroX =
        converterGridParaTelaX(x)
        + (tamanhoCelula / 2.0f);

    float centroY =
        converterGridParaTelaY(y)
        + (tamanhoCelula / 2.0f);

    float raio = tamanhoCelula * 0.30f;

    // Corpo da maçã
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        // Define o vértice central do círculo para o triângulo fan
        glVertex2f(centroX, centroY);

        for (int i = 0; i <= 40; i++) {

            float angulo =
                (2.0f * PI * i) / 40.0f;

            glVertex2f(
                centroX + (cosf(angulo) * raio),
                centroY + (sinf(angulo) * raio)
            );
        }

    glEnd();

    // Cabo da maçã
    glColor3f(0.4f, 0.2f, 0.0f);

    glBegin(GL_QUADS);

        glVertex2f(
            centroX - 0.005f,
            centroY + (raio * 0.8f)
        );

        glVertex2f(
            centroX + 0.005f,
            centroY + (raio * 0.8f)
        );

        glVertex2f(
            centroX + 0.005f,
            centroY + (raio * 1.3f)
        );

        glVertex2f(
            centroX - 0.005f,
            centroY + (raio * 1.3f)
        );

    glEnd();
}

// ==========================================================
// COBRA
// ==========================================================

// Desenha um olho da cobra na posição especificada, usando um círculo branco para o globo ocular e um círculo preto menor para a pupila, posicionando-os de acordo com a direção atual da cobra para criar uma aparência expressiva e garantindo que os olhos sejam proporcionais ao tamanho da cabeça da cobra para manter a consistência visual do jogo
void desenharOlho(float centroX, float centroY) {

    // Branco
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);

        for (int i = 0; i <= 20; i++) {

            float angulo = (2.0f * PI * i) / 20.0f;

            glVertex2f(
                centroX + (cosf(angulo) * 0.008f),
                centroY + (sinf(angulo) * 0.008f)
            );
        }

    glEnd();

    // Pupila
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);
        for (int i = 0; i <= 20; i++) {
            float angulo =
                (2.0f * PI * i) / 20.0f;

            glVertex2f(
                centroX + (cosf(angulo) * 0.004f),
                centroY + (sinf(angulo) * 0.004f)
            );
        }

    glEnd();
}

// Desenha a cabeça da cobra na posição especificada, usando um círculo verde para representar a cabeça e chamando funções auxiliares para desenhar os olhos e a língua da cobra, garantindo que a cabeça seja desenhada de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas, e que os olhos e a língua sejam posicionados de acordo com a direção atual da cobra para criar uma aparência expressiva
void desenharCabeca(int x, int y) {

    float tamanhoCelula = obterTamanhoCelula();

    float centroX =
        converterGridParaTelaX(x)
        + (tamanhoCelula / 2.0f);

    float centroY =
        converterGridParaTelaY(y)
        + (tamanhoCelula / 2.0f);

    float raioCabeca = tamanhoCelula * 0.55f;

    // Cabeça
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);

        for (int i = 0; i <= 40; i++) {

            float angulo = (2.0f * PI * i) / 40.0f;

            glVertex2f(
                centroX + (cosf(angulo) * raioCabeca),
                centroY + (sinf(angulo) * raioCabeca)
            );
        }

    glEnd();

    float deslocamentoOlho = 0.015f;

    float olho1X = 0.0f;
    float olho1Y = 0.0f;

    float olho2X = 0.0f;
    float olho2Y = 0.0f;

    if (direcaoX == 1) {

        olho1X = deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    else if (direcaoX == -1) {

        olho1X = -deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = -deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    else if (direcaoY == 1) {

        olho1X = -deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = deslocamentoOlho;
    }

    else {

        olho1X = -deslocamentoOlho;
        olho1Y = -deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    desenharOlho(
        centroX + olho1X,
        centroY + olho1Y
    );

    desenharOlho(
        centroX + olho2X,
        centroY + olho2Y
    );
}

// Desenha um segmento do corpo da cobra na posição especificada, usando um quadrado verde para representar o segmento do corpo, garantindo que o segmento seja desenhado de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas, criando uma aparência visualmente consistente para o corpo da cobra e diferenciando-o da cabeça da cobra para melhorar a clareza visual do jogo
void desenharCorpo(int x, int y) {

    float tamanhoCelula =
        obterTamanhoCelula();

    float posicaoX =
        converterGridParaTelaX(x);

    float posicaoY =
        converterGridParaTelaY(y);

    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);

        glVertex2f(posicaoX, posicaoY);

        glVertex2f(
            posicaoX + tamanhoCelula,
            posicaoY
        );

        glVertex2f(
            posicaoX + tamanhoCelula,
            posicaoY + tamanhoCelula
        );

        glVertex2f(
            posicaoX,
            posicaoY + tamanhoCelula
        );

    glEnd();
}

// ==========================================================
// LÓGICA DO JOGO
// ==========================================================

// Inicializa a cobra com um tamanho inicial de 3 segmentos, posicionando os segmentos de forma que a cabeça da cobra esteja na posição (5, 5) e os outros segmentos estejam alinhados atrás da cabeça, criando uma configuração inicial para a cobra no início do jogo
void inicializarCobra() {

    cobra[0] = (Segmento){5, 5};
    cobra[1] = (Segmento){4, 5};
    cobra[2] = (Segmento){3, 5};
}

void encerrarJogo() {

    jogoEncerrado = 1;

    atualizarRecordes(pontuacao);

    printf("\n========================\n");
    printf("GAME OVER\n");
    printf("Pontuacao: %d\n", pontuacao);
    printf("========================\n");
}

// Reinicia o jogo definindo as variáveis de estado para os valores iniciais, inicializando a cobra e gerando uma nova posição para a comida, e reiniciando o timer de atualização para permitir que o jogo seja jogado novamente, criando uma função que pode ser chamada quando o jogador deseja reiniciar o jogo após um término, permitindo que eles comecem uma nova partida com as condições iniciais do jogo restauradas, e garantindo que o jogo seja jogável novamente sem a necessidade de reiniciar o programa
void reiniciarJogo() {

    tamanhoCobra = 3;

    direcaoX = 1;
    direcaoY = 0;

    pontuacao = 0;

    jogoEncerrado = 0;

    // Inicializa a cobra
    inicializarCobra();

    // Gera uma nova posição para a comida, garantindo que ela seja colocada em um local válido na grade e não esteja ocupada pela cobra, criando uma configuração inicial para a posição da comida no início de uma nova partida
    gerarComida();

    // Reinicia o timer de atualização para permitir que o jogo seja jogado novamente, garantindo que a função de atualização seja chamada regularmente para mover a cobra e atualizar o estado do jogo, criando uma configuração inicial para o loop de atualização do jogo no início de uma nova partida
    glutTimerFunc(
        INTERVALO_ATUALIZACAO,
        atualizar,
        0
    );

    // Solicita a atualização da tela para refletir as mudanças feitas ao reiniciar o jogo, garantindo que a nova configuração do jogo seja exibida corretamente na tela para o jogador, criando uma experiência visual consistente ao iniciar uma nova partida
    glutPostRedisplay();
}

// Move a cobra atualizando as posições dos segmentos do corpo com base na direção atual, garantindo que a cabeça da cobra se movimente na direção correta e que os segmentos do corpo sigam a cabeça de forma suave, criando uma mecânica de movimento fluida e responsiva para a cobra no jogo
void moverCobra() {

    Segmento antigaCauda =
        cobra[tamanhoCobra - 1];

    for (
        int i = tamanhoCobra - 1;
        i > 0;
        i--
    ) {

        cobra[i] = cobra[i - 1];
    }

    cobra[0].x += direcaoX;
    cobra[0].y += direcaoY;

    // Corrige crescimento da cobra
    cobra[tamanhoCobra] = antigaCauda;
}

// Verifica se a cabeça da cobra colidiu com as paredes da grade, retornando 1 (verdadeiro) se houver uma colisão e 0 (falso) caso contrário, garantindo que o jogo possa detectar quando a cobra atinge as bordas da área de jogo e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão com as paredes para o jogo
int verificarColisaoParede() {

    return (
        cobra[0].x < 0 ||
        cobra[0].x >= GRID_SIZE ||
        cobra[0].y < 0 ||
        cobra[0].y >= GRID_SIZE
    );
}

// Verifica se a cabeça da cobra colidiu com o próprio corpo, retornando 1 (verdadeiro) se houver uma colisão e 0 (falso) caso contrário, garantindo que o jogo possa detectar quando a cobra se machuca e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão com o corpo para o jogo
int verificarColisaoCorpo() {

    // Itera pelos segmentos do corpo da cobra, começando do segundo segmento (índice 1) até o final da cobra, e verifica se as coordenadas da cabeça da cobra (cobra[0]) são iguais às coordenadas de qualquer segmento do corpo (cobra[i]), indicando que a cabeça da cobra colidiu com o próprio corpo, e retornando 1 (verdadeiro) para indicar a colisão, ou 0 (falso) caso contrário, garantindo que o jogo possa detectar quando a cobra se machuca e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão com o corpo para o jogo
    for (int i = 1; i < tamanhoCobra; i++) {

        if (
            cobra[0].x == cobra[i].x &&
            cobra[0].y == cobra[i].y
        ) {
            return 1;
        }
    }

    return 0;
}

// Verifica se a cabeça da cobra colidiu com a comida, e se sim, aumenta o tamanho da cobra, incrementa a pontuação e gera uma nova posição para a comida, garantindo que o jogo possa detectar quando a cobra come a comida e atualizar o estado do jogo adequadamente, criando uma mecânica de detecção de colisão com a comida e crescimento da cobra para o jogo
void verificarComida() {

    // Verifica se as coordenadas da cabeça da cobra (cobra[0]) são iguais às coordenadas da comida (comidaX, comidaY), indicando que a cabeça da cobra colidiu com a comida, e se sim, aumenta o tamanho da cobra (se ainda não atingiu o tamanho máximo), incrementa a pontuação do jogador, e gera uma nova posição para a comida, garantindo que o jogo possa detectar quando a cobra come a comida e atualizar o estado do jogo adequadamente, criando uma mecânica de detecção de colisão com a comida e crescimento da cobra para o jogo
    if (
        cobra[0].x == comidaX &&
        cobra[0].y == comidaY
    ) {

        if (tamanhoCobra < (MAX_COBRA - 1)) {

            tamanhoCobra++;
        }

        pontuacao++;

        gerarComida();
    }
}

// ==========================================================
// RENDERIZAÇÃO
// ==========================================================

// Desenha a pontuação atual do jogador no canto superior esquerdo da tela, usando uma cor branca para o texto e formatando a string de pontuação para exibir o valor atual da pontuação, garantindo que o jogador possa acompanhar seu progresso no jogo e criar uma experiência de jogo mais envolvente e informativa ao exibir a pontuação de forma clara e visível na tela, criando uma mecânica de exibição de pontuação para o jogo
void desenharPontuacao() {


    char textoPontuacao[64];

    snprintf(
        textoPontuacao,
        sizeof(textoPontuacao),
        "Pontuacao: %d",
        pontuacao
    );

    glColor3f(1.0f, 1.0f, 1.0f);

    desenharTexto(
        -0.95f,
        0.92f,
        textoPontuacao
    );
}

// Renderiza a cena do jogo, desenhando o fundo, a comida, a cobra e a pontuação, e se o jogo estiver encerrado, desenha a tela de "GAME OVER", garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica
void display() {

    glClear(GL_COLOR_BUFFER_BIT);

    if (jogoEncerrado) {

        // Desenha a tela de "GAME OVER" e exibe a pontuação final do jogador
        desenharTelaGameOver();

        // Solicita a troca dos buffers para exibir a tela de "GAME OVER" e a pontuação final do jogador, garantindo que o jogador seja informado sobre o término do jogo e seu desempenho final, criando uma experiência de jogo completa ao informar o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma tela de encerramento visualmente atraente para o jogo
        glutSwapBuffers();

        return;
    }

    desenharFundo();

    desenharMaca(comidaX, comidaY);

    for (int i = 0; i < tamanhoCobra; i++) {

        if (i == 0) {

            desenharCabeca(
                cobra[i].x,
                cobra[i].y
            );
        }

        else {

            desenharCorpo(
                cobra[i].x,
                cobra[i].y
            );
        }
    }

    desenharPontuacao();

    glutSwapBuffers();
}

// ==========================================================
// ATUALIZAÇÃO
// ==========================================================

// Atualiza o estado do jogo, movendo a cobra, verificando colisões e atualizando a tela, e se o jogo não estiver encerrado, agenda a próxima atualização usando glutTimerFunc para criar um loop de atualização contínuo, garantindo que o jogo seja atualizado regularmente para criar uma experiência de jogo fluida e responsiva, e que o estado do jogo seja verificado para detectar colisões e encerrar o jogo adequadamente quando necessário, criando uma mecânica de atualização para o jogo
void atualizar(int valor) {

    (void)valor;

    if (jogoEncerrado) {
        return;
    }

    moverCobra();

    if (
        verificarColisaoParede() ||
        verificarColisaoCorpo()
    ) {

        encerrarJogo();

        glutPostRedisplay();

        return;
    }

    verificarComida();

    glutPostRedisplay();

    glutTimerFunc(
        INTERVALO_ATUALIZACAO,
        atualizar,
        0
    );
}

// ==========================================================
// TECLADO
// ==========================================================

// Manipula as entradas do teclado para controlar a direção da cobra, reiniciar o jogo ou encerrar o jogo, garantindo que o jogador possa interagir com o jogo de forma intuitiva e responsiva, criando uma mecânica de controle de entrada para o jogo, e permitindo que o jogador controle a direção da cobra usando as teclas WASD, reinicie o jogo usando a tecla R, ou encerre o jogo usando a tecla ESC, criando uma experiência de jogo envolvente e interativa para o jogador
void teclado(
    unsigned char tecla,
    int x,
    int y
) {

    (void)x;
    (void)y;

    switch (tecla) {

        case 'w':
        case 'W':

            if (direcaoY != -1) {

                direcaoX = 0;
                direcaoY = 1;
            }

            break;

        case 's':
        case 'S':

            if (direcaoY != 1) {

                direcaoX = 0;
                direcaoY = -1;
            }

            break;

        case 'a':
        case 'A':

            if (direcaoX != 1) {

                direcaoX = -1;
                direcaoY = 0;
            }

            break;

        case 'd':
        case 'D':

            if (direcaoX != -1) {

                direcaoX = 1;
                direcaoY = 0;
            }

            break;
        
        // Permite reiniciar o jogo pressionando a tecla 'R' ou 'r' quando o jogo estiver encerrado, chamando a função reiniciarJogo para restaurar as condições iniciais do jogo e permitir que o jogador comece uma nova partida, garantindo que o jogador possa facilmente reiniciar o jogo após um término sem a necessidade de reiniciar o programa, criando uma experiência de jogo mais fluida e conveniente para o jogador
        case 'r':
        case 'R':

            if (jogoEncerrado) {

                reiniciarJogo();
            }

            break;
        
        // Permite encerrar o jogo pressionando a tecla ESC, chamando a função exit(0) para encerrar o programa, garantindo que o jogador possa facilmente sair do jogo quando desejar, criando uma experiência de jogo mais conveniente para o jogador
        case 27:

            exit(0);

            break;
    }
}

// ==========================================================
// MAIN
// ==========================================================

// Função principal do programa, onde a execução começa, inicializando o jogo, configurando o OpenGL, e entrando no loop principal do GLUT para processar eventos e renderizar a cena do jogo, garantindo que o jogo seja iniciado corretamente e que a janela do jogo seja exibida para o jogador, criando uma experiência de jogo completa desde o início da execução do programa
int main(int argc, char **argv) {


    srand((unsigned int)time(NULL));

    carregarRecordes();

    inicializarCobra();

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(LARGURA_JANELA, ALTURA_JANELA);

    glutCreateWindow("Jogo da Cobrinha");

    carregarTexturaFundo();

    gerarComida();

    glutDisplayFunc(display);

    glutKeyboardFunc(teclado);

    glutTimerFunc(INTERVALO_ATUALIZACAO, atualizar, 0);

    glutMainLoop();

    return 0;
}
