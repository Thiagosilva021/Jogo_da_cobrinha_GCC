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
#define MAX_COBRA              400

// Tamanho da janela
#define LARGURA_JANELA         800
#define ALTURA_JANELA          800

// Intervalo de atualização do jogo (em milissegundos)
#define INTERVALO_ATUALIZACAO  150

#define PI 3.14159265359f

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

    // Gera a textura e obtém um identificador para ela usando a função glGenTextures, que cria uma nova textura OpenGL e armazena seu identificador na variável texturaFundo
    glGenTextures(1, &texturaFundo);

    // Vincula a textura para configuração usando a função glBindTexture, que associa a textura recém-criada ao alvo GL_TEXTURE_2D, permitindo que as próximas chamadas de configuração afetem essa textura específica
    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    // Define os dados da textura usando a função glTexImage2D, que especifica a imagem da textura com base nos dados fornecidos em BG_PIXELS, definindo o formato da textura como RGB e o tipo de dados como GL_UNSIGNED_BYTE para garantir que a textura seja carregada corretamente
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

    // Configura os parâmetros de filtragem da textura usando a função glTexParameteri para definir o filtro de minificação (GL_TEXTURE_MIN_FILTER) e o filtro de ampliação (GL_TEXTURE_MAG_FILTER) como GL_LINEAR, garantindo que a textura seja suavemente interpolada quando for redimensionada, o que melhora a qualidade visual da textura no jogo
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    // Configura os parâmetros de filtragem da textura usando a função glTexParameteri para definir o filtro de ampliação (GL_TEXTURE_MAG_FILTER) como GL_LINEAR, garantindo que a textura seja suavemente interpolada quando for redimensionada, o que melhora a qualidade visual da textura no jogo
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );
}

// Desenha o fundo do jogo usando a textura carregada, mapeando a textura para um quadrado que cobre toda a tela, e garantindo que a textura seja exibida corretamente usando a cor branca para evitar alterações na cor da textura, e desabilitando o uso de texturas após o desenho para evitar afetar outros elementos do jogo
void desenharFundo() {

    // Habilita o uso de texturas para que a textura de fundo possa ser aplicada ao quadrado que será desenhado
    glEnable(GL_TEXTURE_2D);

    // Vincula a textura de fundo para que ela seja usada no próximo desenho, garantindo que a textura correta seja aplicada ao quadrado que será desenhado
    glBindTexture(GL_TEXTURE_2D, texturaFundo);

    // Define a cor branca para garantir que a textura seja exibida corretamente 
    glColor3f(1.0f, 1.0f, 1.0f);

    // Desenha um quadrado cobrindo toda a tela com a textura de fundo
    glBegin(GL_QUADS);

        // Mapeia as coordenadas da textura para os vértices do quadrado que cobre toda a tela, garantindo que a textura seja exibida corretamente em toda a área do fundo do jogo
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f( 1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f( 1.0f,  1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f,  1.0f);

    // Finaliza o desenho do quadrado com a textura aplicada
    glEnd();

    // Desabilita o uso de texturas para evitar afetar outros elementos do jogo que não devem ser texturizados, garantindo que apenas o fundo do jogo seja afetado pela textura
    glDisable(GL_TEXTURE_2D);
}

// ==========================================================
// TEXTO
// ==========================================================

// Desenha o texto na tela usando a função glutBitmapCharacter para renderizar cada caractere da string, definindo a posição do texto usando glRasterPos2f e iterando sobre cada caractere da string para desenhá-lo usando a fonte GLUT_BITMAP_HELVETICA_18, garantindo que o texto seja exibido corretamente na posição desejada na tela
void desenharTexto(float x, float y, const char *texto) {

    // Define a posição do texto na tela
    glRasterPos2f(x, y);

    // Itera sobre cada caractere da string e desenha usando a função de glut
    while (*texto) {

        // Desenha o caractere atual usando a fonte especificada
        glutBitmapCharacter(
            // Especifica a fonte a ser usada para desenhar o caractere
            GLUT_BITMAP_HELVETICA_18,
            // Passa o caractere atual para a função de desenho
            *texto
        );

        // Move para o próximo caractere na string
        texto++;
    }
}

// ==========================================================
// GAME OVER
// ==========================================================

// Desenha a tela de game over, exibindo uma mensagem de "GAME OVER" e a pontuação final do jogador, além de instruções para reiniciar ou sair do jogo, usando cores e posicionamento adequados para criar uma tela de encerramento visualmente atraente e informativa para o jogador quando o jogo termina
void desenharTelaGameOver() {

    // Desenha um fundo preto para a tela de game over
    glColor3f(0.0f, 0.0f, 0.0f);

    // Desenha um quadrado cobrindo toda a tela
    glBegin(GL_QUADS);

        // Define os vértices do quadrado para cobrir toda a tela
        glVertex2f(-1.0f, -1.0f);
        glVertex2f( 1.0f, -1.0f);
        glVertex2f( 1.0f,  1.0f);
        glVertex2f(-1.0f,  1.0f);

    glEnd();

    // Define a cor vermelha para o texto de "GAME OVER"
    glColor3f(1.0f, 0.0f, 0.0f);

    // Desenha o texto de "GAME OVER" no centro da tela
    desenharTexto(-0.15f, 0.10f, "GAME OVER");

    // Prepara o texto para exibir a pontuação final do jogador
    char textoPontuacao[64];

    // Formata a string com a pontuação final usando snprintf para evitar estouro de buffer
    snprintf(
        textoPontuacao,
        sizeof(textoPontuacao),
        "Pontuacao Final: %d",
        pontuacao
    );

    // Define a cor branca para o texto da pontuação
    glColor3f(1.0f, 1.0f, 1.0f);

    // Desenha o texto da pontuação final abaixo do texto de "GAME OVER"
    desenharTexto(-0.25f, -0.05f, textoPontuacao);

    // Desenha instruções para o jogador sobre como reiniciar ou sair do jogo
    desenharTexto(
        -0.35f,
        -0.20f,
        "Pressione R para jogar novamente"
    );

    // Desenha instruções para o jogador sobre como sair do jogo
    desenharTexto(
        -0.18f,
        -0.30f,
        "ESC para sair"
    );
}

// ==========================================================
// COMIDA
// ==========================================================

// Verifica se a posição (x, y) está ocupada por algum segmento da cobra iterando sobre cada segmento da cobra e comparando suas coordenadas com as coordenadas fornecidas, retornando 1 (verdadeiro) se a posição estiver ocupada por algum segmento da cobra, ou 0 (falso) caso contrário, o que é útil para garantir que a comida seja gerada em uma posição válida que não esteja ocupada pela cobra
int posicaoOcupadaPelaCobra(int x, int y) {

    // Itera sobre cada segmento da cobra para verificar se a posição coincide com algum deles
    for (int i = 0; i < tamanhoCobra; i++) {

        // Verifica se as coordenadas do segmento atual da cobra coincidem com as coordenadas fornecidas
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

    // Define um número máximo de tentativas para evitar loops infinitos caso a cobra ocupe quase toda a grade
    int tentativaMaxima = 1000;
    // Contador de tentativas para gerar uma posição válida para a comida
    int tentativa = 0;

    // Loop para gerar uma posição aleatória para a comida até encontrar uma posição que não esteja ocupada pela cobra
    do {

        // Gera coordenadas aleatórias para a comida dentro dos limites da grade
        comidaX = rand() % GRID_SIZE;
        comidaY = rand() % GRID_SIZE;

        // Incrementa o contador de tentativas para evitar loops infinitos
        tentativa++;

    // Continua tentando gerar uma posição para a comida enquanto a posição gerada estiver ocupada pela cobra e o número de tentativas for menor que o máximo permitido
    } while (
        posicaoOcupadaPelaCobra(comidaX, comidaY) &&
        tentativa < tentativaMaxima
    );
}

// Desenha a maçã na posição especificada, usando um círculo vermelho para representar a maçã e um retângulo marrom para representar o cabo da maçã, garantindo que a maçã seja desenhada de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas
void desenharMaca(int x, int y) {

    // Obtém o tamanho de cada célula da grade para calcular a posição e o tamanho da maçã na tela
    float tamanhoCelula = obterTamanhoCelula();

    // Calcula as coordenadas do centro da maçã com base na posição da grade e no tamanho da célula
    float centroX =
        // Converte a coordenada x da grade para a coordenada de tela e ajusta para o centro da célula
        converterGridParaTelaX(x)
        // Ajusta para o centro da célula adicionando metade do tamanho da célula à coordenada convertida, garantindo que a maçã seja desenhada centralizada na célula correspondente à sua posição na grade
        + (tamanhoCelula / 2.0f);

    // Converte a coordenada y da grade para a coordenada de tela e ajusta para o centro da célula
    float centroY =
        // Converte a coordenada y da grade para a coordenada de tela e ajusta para o centro da célula
        converterGridParaTelaY(y)
        // Ajusta para o centro da célula adicionando metade do tamanho da célula à coordenada convertida, garantindo que a maçã seja desenhada centralizada na célula correspondente à sua posição na grade
        + (tamanhoCelula / 2.0f);

    // Calcula o raio da maçã com base no tamanho da célula para garantir que a maçã seja proporcional ao tamanho da grade
    float raio = tamanhoCelula * 0.30f;

    // Corpo da maçã
    glColor3f(1.0f, 0.0f, 0.0f);

    // Desenha a maçã como um círculo usando um triângulo fan para criar uma forma suave
    glBegin(GL_TRIANGLE_FAN);

        // Define o vértice central do círculo para o triângulo fan
        glVertex2f(centroX, centroY);

        // Itera para criar os vértices do círculo, calculando as coordenadas usando funções trigonométricas para distribuir os vértices uniformemente ao redor do centro
        for (int i = 0; i <= 40; i++) {

            // Calcula o ângulo para o vértice atual com base no número total de vértices (40 neste caso) para criar um círculo completo
            float angulo =
                (2.0f * PI * i) / 40.0f;

            // Calcula as coordenadas do vértice atual usando as funções cosf e sinf para criar um círculo ao redor do centro da maçã, multiplicando pelo raio para ajustar o tamanho do círculo
            glVertex2f(
                // Calcula a coordenada x do vértice atual usando a função cosf para criar um círculo ao redor do centro da maçã
                centroX + (cosf(angulo) * raio),
                centroY + (sinf(angulo) * raio)
            );
        }

    // Finaliza o desenho do círculo que representa a maçã
    glEnd();

    // Cabo da maçã
    glColor3f(0.4f, 0.2f, 0.0f);

    // Desenha o cabo da maçã como um retângulo estreito que se estende para cima a partir do centro da maçã, usando as coordenadas do centro e o raio para posicionar corretamente o cabo
    glBegin(GL_QUADS);

        // Define os vértices do retângulo para o cabo da maçã, posicionando-o centralizado em relação ao centro da maçã e estendendo-se para cima, com uma largura de 0.01f e um comprimento que varia de 0.8 a 1.3 vezes o raio da maçã para criar um cabo proporcional ao tamanho da maçã
        glVertex2f(
            // Calcula a coordenada x do vértice esquerdo do cabo, centralizado em relação ao centro da maçã
            centroX - 0.005f,
            // Calcula a coordenada y do vértice inferior do cabo, posicionando-o um pouco acima do centro da maçã
            centroY + (raio * 0.8f)
        );

        // Calcula a coordenada x do vértice direito do cabo, centralizado em relação ao centro da maçã
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

    // Desenha o globo ocular como um círculo usando um triângulo fan para criar uma forma suave, com um raio de 0.008f para garantir que o olho seja proporcional ao tamanho da cabeça da cobra
    glBegin(GL_TRIANGLE_FAN);

        // Define o vértice central do círculo para o triângulo fan
        glVertex2f(centroX, centroY);

        // Itera para criar os vértices do círculo, calculando as coordenadas usando funções trigonométricas para distribuir os vértices uniformemente ao redor do centro do olho
        for (int i = 0; i <= 20; i++) {

            // Calcula o ângulo para o vértice atual com base no número total de vértices (20 neste caso) para criar um círculo completo
            float angulo = (2.0f * PI * i) / 20.0f;

            // Calcula as coordenadas do vértice atual usando as funções cosf e sinf para criar um círculo ao redor do centro do olho, multiplicando pelo raio para ajustar o tamanho do círculo
            glVertex2f(
                centroX + (cosf(angulo) * 0.008f),
                centroY + (sinf(angulo) * 0.008f)
            );
        }

    glEnd();

    // Pupila
    glColor3f(0.0f, 0.0f, 0.0f);

    // Desenha a pupila como um círculo menor usando um triângulo fan para criar uma forma suave, com um raio de 0.004f para garantir que a pupila seja proporcional ao tamanho do globo ocular, e posiciona a pupila de acordo com a direção atual da cobra para criar uma aparência expressiva
    glBegin(GL_TRIANGLE_FAN);

        // Define o vértice central do círculo para o triângulo fan
        glVertex2f(centroX, centroY);
        // Itera para criar os vértices do círculo, calculando as coordenadas usando funções trigonométricas para distribuir os vértices uniformemente ao redor do centro da pupila
        for (int i = 0; i <= 20; i++) {
            // Calcula o ângulo para o vértice atual com base no número total de vértices (20 neste caso) para criar um círculo completo
            float angulo =
                // Calcula o ângulo para o vértice atual com base no número total de vértices (20 neste caso) para criar um círculo completo
                (2.0f * PI * i) / 20.0f;

            // Calcula as coordenadas do vértice atual usando as funções cosf e sinf para criar um círculo ao redor do centro da pupila, multiplicando pelo raio para ajustar o tamanho do círculo, e deslocando a posição da pupila em direção à direção atual da cobra para criar uma aparência expressiva
            glVertex2f(
                centroX + (cosf(angulo) * 0.004f),
                centroY + (sinf(angulo) * 0.004f)
            );
        }

    glEnd();
}

// Desenha a língua da cobra como um triângulo que se estende a partir da cabeça da cobra na direção atual, usando uma cor vermelha para criar um efeito visual de língua, e posicionando o triângulo de acordo com a direção atual da cobra para criar uma aparência expressiva
void desenharLingua(
    // Coordenada x do centro da cabeça da cobra, usada para posicionar a língua corretamente em relação à cabeça
    float centroX,
    // Coordenada y do centro da cabeça da cobra, usada para posicionar a língua corretamente em relação à cabeça
    float centroY,
    // Raio da cabeça da cobra, usado para calcular a posição inicial da língua em relação à cabeça
    float raioCabeca
) {

    // Define a cor vermelha para a língua da cobra
    glColor3f(1.0f, 0.0f, 0.0f);

    // Desenha a língua como um triângulo usando glBegin(GL_TRIANGLES) para criar uma forma pontiaguda, e posiciona o triângulo de acordo com a direção atual da cobra para criar uma aparência expressiva, estendendo-se a partir da cabeça da cobra na direção em que a cobra está se movendo, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
    glBegin(GL_TRIANGLES);

        // Verifica a direção atual da cobra e posiciona os vértices do triângulo da língua de acordo, garantindo que a língua se estenda na direção correta em relação à cabeça da cobra para criar uma aparência expressiva
        if (direcaoX == 1) {

            // Calcula as coordenadas dos vértices do triângulo da língua para a direção direita, posicionando o vértice central da base da língua no centro da cabeça e os outros dois vértices formando um triângulo que se estende para a direita, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
            glVertex2f(
                // Calcula a coordenada x do vértice da ponta da língua, posicionando-a um pouco além do raio da cabeça na direção direita
                centroX + raioCabeca,
                // A coordenada y do vértice da ponta da língua é a mesma do centro da cabeça para manter a língua alinhada horizontalmente
                centroY
            );

            glVertex2f(
                centroX + raioCabeca + 0.03f,
                centroY + 0.01f
            );

            glVertex2f(
                centroX + raioCabeca + 0.03f,
                centroY - 0.01f
            );
        }

        // Verifica a direção atual da cobra e posiciona os vértices do triângulo da língua de acordo, garantindo que a língua se estenda na direção correta em relação à cabeça da cobra para criar uma aparência expressiva, estendendo-se para a esquerda quando a cobra estiver se movendo para a esquerda, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
        else if (direcaoX == -1) {
            
            // Calcula as coordenadas dos vértices do triângulo da língua para a direção esquerda, posicionando o vértice central da base da língua no centro da cabeça e os outros dois vértices formando um triângulo que se estende para a esquerda, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
            glVertex2f(
                centroX - raioCabeca,
                centroY
            );

            glVertex2f(
                centroX - raioCabeca - 0.03f,
                centroY + 0.01f
            );

            glVertex2f(
                centroX - raioCabeca - 0.03f,
                centroY - 0.01f
            );
        }

        // Verifica a direção atual da cobra e posiciona os vértices do triângulo da língua de acordo, garantindo que a língua se estenda na direção correta em relação à cabeça da cobra para criar uma aparência expressiva, estendendo-se para cima quando a cobra estiver se movendo para cima, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
        else if (direcaoY == 1) {

            glVertex2f(
                centroX,
                centroY + raioCabeca
            );

            glVertex2f(
                centroX - 0.01f,
                centroY + raioCabeca + 0.03f
            );

            glVertex2f(
                centroX + 0.01f,
                centroY + raioCabeca + 0.03f
            );
        }

        // Verifica a direção atual da cobra e posiciona os vértices do triângulo da língua de acordo, garantindo que a língua se estenda na direção correta em relação à cabeça da cobra para criar uma aparência expressiva, estendendo-se para baixo quando a cobra estiver se movendo para baixo, com um comprimento de 0.03f para criar uma língua proporcional ao tamanho da cabeça da cobra
        else {

            glVertex2f(
                centroX,
                centroY - raioCabeca
            );

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
}

// Desenha a cabeça da cobra na posição especificada, usando um círculo verde para representar a cabeça e chamando funções auxiliares para desenhar os olhos e a língua da cobra, garantindo que a cabeça seja desenhada de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas, e que os olhos e a língua sejam posicionados de acordo com a direção atual da cobra para criar uma aparência expressiva
void desenharCabeca(int x, int y) {

    // Obtém o tamanho de cada célula da grade para calcular a posição e o tamanho da cabeça da cobra na tela
    float tamanhoCelula = obterTamanhoCelula();

    // Calcula as coordenadas do centro da cabeça da cobra com base na posição da grade e no tamanho da célula, garantindo que a cabeça seja desenhada centralizada na célula correspondente à sua posição na grade
    float centroX =
        // Converte a coordenada x da grade para a coordenada de tela e ajusta para o centro da célula
        converterGridParaTelaX(x)
        // Ajusta para o centro da célula adicionando metade do tamanho da célula à coordenada convertida, garantindo que a cabeça seja desenhada centralizada na célula correspondente à sua posição na grade
        + (tamanhoCelula / 2.0f);

    // Calcula as coordenadas do centro da cabeça da cobra com base na posição da grade e no tamanho da célula, garantindo que a cabeça seja desenhada centralizada na célula correspondente à sua posição na grade
    float centroY =
        converterGridParaTelaY(y)
        + (tamanhoCelula / 2.0f);

    // Calcula o raio da cabeça da cobra com base no tamanho da célula para garantir que a cabeça seja proporcional ao tamanho da grade
    float raioCabeca = tamanhoCelula * 0.55f;

    // Cabeça
    glColor3f(0.0f, 0.0f, 0.0f);

    // Desenha a cabeça da cobra como um círculo usando um triângulo fan para criar uma forma suave, com um raio calculado para garantir que a cabeça seja proporcional ao tamanho da grade, e posiciona a cabeça de acordo com as coordenadas fornecidas para garantir que ela seja desenhada corretamente na tela
    glBegin(GL_TRIANGLE_FAN);

        // Define o vértice central do círculo para o triângulo fan
        glVertex2f(centroX, centroY);

        // Itera para criar os vértices do círculo, calculando as coordenadas usando funções trigonométricas para distribuir os vértices uniformemente ao redor do centro da cabeça da cobra, multiplicando pelo raio para ajustar o tamanho do círculo
        for (int i = 0; i <= 40; i++) {

            // Calcula o ângulo para o vértice atual com base no número total de vértices (40 neste caso) para criar um círculo completo
            float angulo = (2.0f * PI * i) / 40.0f;

            // Calcula as coordenadas do vértice atual usando as funções cosf e sinf para criar um círculo ao redor do centro da cabeça da cobra, multiplicando pelo raio para ajustar o tamanho do círculo, e posiciona a cabeça de acordo com as coordenadas fornecidas para garantir que ela seja desenhada corretamente na tela
            glVertex2f(
                centroX + (cosf(angulo) * raioCabeca),
                centroY + (sinf(angulo) * raioCabeca)
            );
        }

    glEnd();

    // Desenha a língua da cobra chamando a função desenharLingua, passando as coordenadas do centro da cabeça e o raio da cabeça para garantir que a língua seja posicionada corretamente em relação à cabeça da cobra e se estenda na direção correta de acordo com a direção atual da cobra para criar uma aparência expressiva
    desenharLingua(
        centroX,
        centroY,
        raioCabeca
    );

    // Desenha os olhos da cobra chamando a função desenharOlho para cada olho, calculando o deslocamento dos olhos com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra, criando uma aparência visualmente atraente e consistente com a direção em que a cobra está se movendo
    float deslocamentoOlho = 0.015f;

    // Variáveis para armazenar as coordenadas dos olhos, que serão calculadas com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra
    float olho1X = 0.0f;
    float olho1Y = 0.0f;

    // Variáveis para armazenar as coordenadas dos olhos, que serão calculadas com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra
    float olho2X = 0.0f;
    float olho2Y = 0.0f;

    // Calcula o deslocamento dos olhos com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra, criando uma aparência visualmente atraente e consistente com a direção em que a cobra está se movendo
    if (direcaoX == 1) {

        // Calcula as coordenadas dos olhos para a direção direita, posicionando-os um pouco acima e abaixo do centro da cabeça, com um deslocamento horizontal para a direita para criar uma aparência expressiva e proporcional à cabeça da cobra
        olho1X = deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    // Calcula o deslocamento dos olhos com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra, criando uma aparência visualmente atraente e consistente com a direção em que a cobra está se movendo, estendendo-se para a esquerda quando a cobra estiver se movendo para a esquerda, com um deslocamento horizontal para a esquerda para criar uma aparência expressiva e proporcional à cabeça da cobra
    else if (direcaoX == -1) {

        olho1X = -deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = -deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    // Calcula o deslocamento dos olhos com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra, criando uma aparência visualmente atraente e consistente com a direção em que a cobra está se movendo, estendendo-se para cima quando a cobra estiver se movendo para cima, com um deslocamento vertical para cima para criar uma aparência expressiva e proporcional à cabeça da cobra
    else if (direcaoY == 1) {

        olho1X = -deslocamentoOlho;
        olho1Y = deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = deslocamentoOlho;
    }

    // Calcula o deslocamento dos olhos com base na direção atual da cobra para garantir que os olhos sejam posicionados de forma expressiva e proporcional à cabeça da cobra, criando uma aparência visualmente atraente e consistente com a direção em que a cobra está se movendo, estendendo-se para baixo quando a cobra estiver se movendo para baixo, com um deslocamento vertical para baixo para criar uma aparência expressiva e proporcional à cabeça da cobra
    else {

        olho1X = -deslocamentoOlho;
        olho1Y = -deslocamentoOlho;

        olho2X = deslocamentoOlho;
        olho2Y = -deslocamentoOlho;
    }

    // Desenha o primeiro olho da cobra chamando a função desenharOlho, passando as coordenadas calculadas para o primeiro olho, que são baseadas no centro da cabeça e no deslocamento calculado para garantir que o olho seja posicionado de forma expressiva e proporcional à cabeça da cobra
    desenharOlho(
        centroX + olho1X,
        centroY + olho1Y
    );

    // Desenha o segundo olho da cobra chamando a função desenharOlho, passando as coordenadas calculadas para o segundo olho, que são baseadas no centro da cabeça e no deslocamento calculado para garantir que o olho seja posicionado de forma expressiva e proporcional à cabeça da cobra
    desenharOlho(
        centroX + olho2X,
        centroY + olho2Y
    );
}

// Desenha um segmento do corpo da cobra na posição especificada, usando um quadrado verde para representar o segmento do corpo, garantindo que o segmento seja desenhado de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas, criando uma aparência visualmente consistente para o corpo da cobra e diferenciando-o da cabeça da cobra para melhorar a clareza visual do jogo
void desenharCorpo(int x, int y) {

    // Obtém o tamanho de cada célula da grade para calcular a posição e o tamanho do segmento do corpo da cobra na tela
    float tamanhoCelula =
        obterTamanhoCelula();

    // Calcula as coordenadas do canto inferior esquerdo do segmento do corpo da cobra com base na posição da grade e no tamanho da célula, garantindo que o segmento seja desenhado corretamente na tela de acordo com as coordenadas fornecidas
    float posicaoX =
        converterGridParaTelaX(x);

    // Calcula as coordenadas do canto inferior esquerdo do segmento do corpo da cobra com base na posição da grade e no tamanho da célula, garantindo que o segmento seja desenhado corretamente na tela de acordo com as coordenadas fornecidas
    float posicaoY =
        converterGridParaTelaY(y);

    // Define a cor para o segmento do corpo da cobra
    glColor3f(0.0f, 0.0f, 0.0f);

    // Desenha o segmento do corpo da cobra como um quadrado usando glBegin(GL_QUADS) para criar uma forma sólida, e posiciona o quadrado de acordo com as coordenadas fornecidas para garantir que ele seja desenhado corretamente na tela, criando uma aparência visualmente consistente para o corpo da cobra e diferenciando-o da cabeça da cobra para melhorar a clareza visual do jogo
    glBegin(GL_QUADS);

        // Define os vértices do quadrado para o segmento do corpo da cobra, posicionando-os de acordo com as coordenadas calculadas para garantir que o segmento seja desenhado corretamente na tela, com um tamanho igual ao tamanho da célula para criar uma aparência proporcional ao tamanho da grade
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

    // Define os segmentos iniciais da cobra, com a cabeça na posição (5, 5) e os outros segmentos alinhados atrás da cabeça para criar uma configuração inicial para a cobra no início do jogo
    cobra[0] = (Segmento){5, 5};
    cobra[1] = (Segmento){4, 5};
    cobra[2] = (Segmento){3, 5};
}

// Encerra o jogo definindo a variável jogoEncerrado como 1 (verdadeiro) e exibindo uma mensagem de "GAME OVER" junto com a pontuação final do jogador, criando uma tela de encerramento que informa o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma experiência de jogo completa ao informar o jogador sobre o término do jogo e seu desempenho final
void encerrarJogo() {

    // Define a variável jogoEncerrado como 1 (verdadeiro) para indicar que o jogo foi encerrado, o que é usado para controlar o fluxo do jogo e impedir que a cobra continue se movendo ou que o jogador possa interagir com o jogo após o término
    jogoEncerrado = 1;

    // Exibe uma mensagem de "GAME OVER" junto com a pontuação final do jogador, criando uma tela de encerramento que informa o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma experiência de jogo completa ao informar o jogador sobre o término do jogo e seu desempenho final
    printf("\n========================\n");
    printf("GAME OVER\n");
    printf("Pontuacao: %d\n", pontuacao);
    printf("========================\n");
}

// Reinicia o jogo definindo as variáveis de estado para os valores iniciais, inicializando a cobra e gerando uma nova posição para a comida, e reiniciando o timer de atualização para permitir que o jogo seja jogado novamente, criando uma função que pode ser chamada quando o jogador deseja reiniciar o jogo após um término, permitindo que eles comecem uma nova partida com as condições iniciais do jogo restauradas, e garantindo que o jogo seja jogável novamente sem a necessidade de reiniciar o programa
void reiniciarJogo() {

    // Define o tamanho inicial da cobra como 3 segmentos, criando uma configuração inicial para a cobra no início do jogo
    tamanhoCobra = 3;

    // Define a direção inicial da cobra para a direita, criando uma configuração inicial para a direção da cobra no início do jogo
    direcaoX = 1;
    direcaoY = 0;

    // Define a pontuação inicial como 0, criando uma configuração inicial para a pontuação do jogador no início do jogo
    pontuacao = 0;

    // Define a variável jogoEncerrado como 0 (falso) para indicar que o jogo está ativo, permitindo que a cobra se mova e que o jogador possa interagir com o jogo, criando uma configuração inicial para o estado do jogo no início de uma nova partida
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

    // Armazena a posição da cauda antiga para corrigir o crescimento da cobra quando ela comer a comida, garantindo que a cobra cresça corretamente ao adicionar um novo segmento na posição da cauda antiga, criando uma mecânica de crescimento consistente e visualmente correta para a cobra no jogo
    Segmento antigaCauda =
        cobra[tamanhoCobra - 1];

    // Atualiza as posições dos segmentos do corpo da cobra, começando do final da cobra e movendo cada segmento para a posição do segmento anterior, garantindo que os segmentos do corpo sigam a cabeça de forma suave e criando uma mecânica de movimento fluida e responsiva para a cobra no jogo ao mover a cabeça na direção correta, e garantindo que a posição da cauda antiga seja armazenada para corrigir o crescimento da cobra quando ela comer a comida, criando uma mecânica de crescimento consistente e visualmente correta para a cobra no jogo
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

    // Verifica se a coordenada x da cabeça da cobra é menor que 0 ou maior ou igual ao tamanho da grade, ou se a coordenada y da cabeça da cobra é menor que 0 ou maior ou igual ao tamanho da grade, indicando que a cabeça da cobra colidiu com as paredes da grade, e retornando 1 (verdadeiro) para indicar a colisão, ou 0 (falso) caso contrário, garantindo que o jogo possa detectar quando a cobra atinge as bordas da área de jogo e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão com as paredes para o jogo
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

    // Formata a string de pontuação para exibir o valor atual da pontuação, usando snprintf para garantir que a string seja formatada corretamente e que não haja estouro de buffer, criando uma string de pontuação que pode ser exibida na tela para informar o jogador sobre seu progresso no jogo
    char textoPontuacao[64];

    // Formata a string de pontuação para exibir o valor atual da pontuação, usando snprintf para garantir que a string seja formatada corretamente e que não haja estouro de buffer, criando uma string de pontuação que pode ser exibida na tela para informar o jogador sobre seu progresso no jogo
    snprintf(
        textoPontuacao,
        sizeof(textoPontuacao),
        "Pontuacao: %d",
        pontuacao
    );

    // Define a cor branca para o texto da pontuação, garantindo que o texto seja claramente visível contra o fundo do jogo, criando uma experiência de jogo mais envolvente e informativa ao exibir a pontuação de forma clara e visível na tela, criando uma mecânica de exibição de pontuação para o jogo
    glColor3f(1.0f, 1.0f, 1.0f);

    // Desenha o texto da pontuação no canto superior esquerdo da tela, usando a função desenharTexto para posicionar o texto de forma clara e visível, garantindo que o jogador possa acompanhar seu progresso no jogo e criar uma experiência de jogo mais envolvente e informativa ao exibir a pontuação de forma clara e visível na tela, criando uma mecânica de exibição de pontuação para o jogo
    desenharTexto(
        -0.95f,
        0.92f,
        textoPontuacao
    );
}

// Renderiza a cena do jogo, desenhando o fundo, a comida, a cobra e a pontuação, e se o jogo estiver encerrado, desenha a tela de "GAME OVER", garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica
void display() {

    // Limpa a tela com a cor de fundo definida anteriormente, garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica de renderização para o jogo
    glClear(GL_COLOR_BUFFER_BIT);

    // Verifica se o jogo foi encerrado, e se sim, desenha a tela de "GAME OVER" e exibe a pontuação final do jogador, garantindo que o jogador seja informado sobre o término do jogo e seu desempenho final, criando uma experiência de jogo completa ao informar o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma tela de encerramento visualmente atraente para o jogo
    if (jogoEncerrado) {

        // Desenha a tela de "GAME OVER" e exibe a pontuação final do jogador
        desenharTelaGameOver();

        // Solicita a troca dos buffers para exibir a tela de "GAME OVER" e a pontuação final do jogador, garantindo que o jogador seja informado sobre o término do jogo e seu desempenho final, criando uma experiência de jogo completa ao informar o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma tela de encerramento visualmente atraente para o jogo
        glutSwapBuffers();

        return;
    }

    // Desenha o fundo do jogo, garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica de renderização para o jogo
    desenharFundo();

    // Desenha a comida (maçã) no tabuleiro
    desenharMaca(comidaX, comidaY);

    // Desenha a cobra iterando pelos segmentos do corpo da cobra e chamando a função desenharCabeca para o primeiro segmento (cabeça) e a função desenharCorpo para os outros segmentos, garantindo que a cabeça da cobra seja desenhada de forma distinta dos segmentos do corpo para melhorar a clareza visual do jogo, e que todos os segmentos sejam desenhados de forma proporcional ao tamanho da grade e posicionados corretamente na tela com base nas coordenadas fornecidas, criando uma aparência visualmente consistente para a cobra no jogo
    for (int i = 0; i < tamanhoCobra; i++) {

        // Verifica se o segmento atual é a cabeça da cobra (índice 0) e chama a função desenharCabeca para desenhar a cabeça, ou chama a função desenharCorpo para desenhar os segmentos do corpo, garantindo que a cabeça da cobra seja desenhada de forma distinta dos segmentos do corpo para melhorar a clareza visual do jogo, e que todos os segmentos sejam desenhados de forma proporcional ao tamanho da grade e posicionados corretamente na tela com base nas coordenadas fornecidas, criando uma aparência visualmente consistente para a cobra no jogo
        if (i == 0) {

            desenharCabeca(
                cobra[i].x,
                cobra[i].y
            );
        }

        // Verifica se o segmento atual é o corpo da cobra (índice diferente de 0) e chama a função desenharCorpo para desenhar o segmento, garantindo que todos os segmentos sejam desenhados de forma proporcional ao tamanho da grade e posicionados corretamente na tela com base nas coordenadas fornecidas, criando uma aparência visualmente consistente para a cobra no jogo
        else {

            desenharCorpo(
                cobra[i].x,
                cobra[i].y
            );
        }
    }

    // Desenha a pontuação atual do jogador no canto superior esquerdo da tela, garantindo que o jogador possa acompanhar seu progresso no jogo e criar uma experiência de jogo mais envolvente e informativa ao exibir a pontuação de forma clara e visível na tela, criando uma mecânica de exibição de pontuação para o jogo
    desenharPontuacao();

    // Solicita a troca dos buffers para exibir a cena renderizada, garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica de renderização para o jogo
    glutSwapBuffers();
}

// ==========================================================
// ATUALIZAÇÃO
// ==========================================================

// Atualiza o estado do jogo, movendo a cobra, verificando colisões e atualizando a tela, e se o jogo não estiver encerrado, agenda a próxima atualização usando glutTimerFunc para criar um loop de atualização contínuo, garantindo que o jogo seja atualizado regularmente para criar uma experiência de jogo fluida e responsiva, e que o estado do jogo seja verificado para detectar colisões e encerrar o jogo adequadamente quando necessário, criando uma mecânica de atualização para o jogo
void atualizar(int valor) {

    // O parâmetro 'valor' é necessário para a função de callback do timer, mas não é utilizado no corpo da função, então é marcado como (void) para evitar avisos de variável não utilizada, garantindo que o código seja limpo e livre de avisos desnecessários, e que a função de atualização possa ser chamada corretamente pelo timer sem a necessidade de usar o parâmetro 'valor' no corpo da função
    (void)valor;

    // Verifica se o jogo foi encerrado, e se sim, retorna imediatamente para evitar que a cobra continue se movendo ou que o jogador possa interagir com o jogo após o término, garantindo que o estado do jogo seja controlado adequadamente e que a experiência de jogo seja consistente ao encerrar o jogo quando necessário, criando uma mecânica de controle de estado para o jogo
    if (jogoEncerrado) {
        return;
    }

    // Move a cobra atualizando as posições dos segmentos do corpo com base na direção atual, garantindo que a cabeça da cobra se movimente na direção correta e que os segmentos do corpo sigam a cabeça de forma suave, criando uma mecânica de movimento fluida e responsiva para a cobra no jogo
    moverCobra();

    // Verifica se a cabeça da cobra colidiu com as paredes da grade ou com o próprio corpo, e se sim, encerra o jogo e solicita a atualização da tela para exibir a tela de "GAME OVER", garantindo que o jogo possa detectar quando a cobra atinge as bordas da área de jogo ou se machuca, e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão para o jogo
    if (
        verificarColisaoParede() ||
        verificarColisaoCorpo()
    ) {

        // Encerra o jogo definindo a variável jogoEncerrado como 1 (verdadeiro) e exibindo uma mensagem de "GAME OVER" junto com a pontuação final do jogador, criando uma tela de encerramento que informa o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma experiência de jogo completa ao informar o jogador sobre o término do jogo e seu desempenho final
        encerrarJogo();

        // Solicita a atualização da tela para exibir a tela de "GAME OVER" e a pontuação final do jogador, garantindo que o jogador seja informado sobre o término do jogo e seu desempenho final, criando uma experiência de jogo completa ao informar o jogador sobre o resultado do jogo e sua pontuação final, e fornecendo uma tela de encerramento visualmente atraente para o jogo
        glutPostRedisplay();

        return;
    }

    // Verifica se a cabeça da cobra colidiu com a comida, e se sim, aumenta o tamanho da cobra, incrementa a pontuação e gera uma nova posição para a comida, garantindo que o jogo possa detectar quando a cobra come a comida e atualizar o estado do jogo adequadamente, criando uma mecânica de detecção de colisão com a comida e crescimento da cobra para o jogo
    verificarComida();

    // Solicita a atualização da tela para refletir as mudanças feitas ao mover a cobra, verificar colisões e atualizar a pontuação, garantindo que a cena do jogo seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma função de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica de renderização para o jogo
    glutPostRedisplay();

    // Agenda a próxima atualização usando glutTimerFunc para criar um loop de atualização contínuo, garantindo que o jogo seja atualizado regularmente para criar uma experiência de jogo fluida e responsiva, e que o estado do jogo seja verificado para detectar colisões e encerrar o jogo adequadamente quando necessário, criando uma mecânica de atualização para o jogo
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
// OPENGL
// ==========================================================

// Configura as definições iniciais do OpenGL, como a cor de fundo, o modo de projeção e a matriz de modelagem, garantindo que a cena do jogo seja renderizada corretamente na tela com as configurações adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
void configurarOpenGL() {

    // Define a cor de fundo para preto com opacidade total, garantindo que a tela seja limpa com essa cor antes de desenhar os elementos do jogo, criando uma experiência visual consistente para o jogo ao usar uma cor de fundo sólida e escura que contrasta bem com os elementos do jogo,
    glClearColor(0.0f, 0.0f ,0.0f ,1.0f);

    // Define o modo de projeção para ortogonal usando gluOrtho2D, configurando as coordenadas de visualização para um sistema de coordenadas 2D que vai de -1.0 a 1.0 em ambos os eixos x e y, garantindo que a cena do jogo seja renderizada corretamente na tela com as configurações adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glMatrixMode(GL_PROJECTION);

    // Carrega a matriz de identidade para resetar qualquer transformação anterior, garantindo que a configuração da projeção seja aplicada corretamente sem interferência de transformações anteriores, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glLoadIdentity();

    // Define o modo de projeção para ortogonal usando gluOrtho2D, configurando as coordenadas de visualização para um sistema de coordenadas 2D que vai de -1.0 a 1.0 em ambos os eixos x e y, garantindo que a cena do jogo seja renderizada corretamente na tela com as configurações adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    // Define o modo de modelagem para GL_MODELVIEW e carrega a matriz de identidade para resetar qualquer transformação anterior, garantindo que a configuração da modelagem seja aplicada corretamente sem interferência de transformações anteriores, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glMatrixMode(GL_MODELVIEW);

    // Carrega a matriz de identidade para resetar qualquer transformação anterior, garantindo que a configuração da modelagem seja aplicada corretamente sem interferência de transformações anteriores, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glLoadIdentity();
}

// ==========================================================
// MAIN
// ==========================================================

// Função principal do programa, onde a execução começa, inicializando o jogo, configurando o OpenGL, e entrando no loop principal do GLUT para processar eventos e renderizar a cena do jogo, garantindo que o jogo seja iniciado corretamente e que a janela do jogo seja exibida para o jogador, criando uma experiência de jogo completa desde o início da execução do programa
int main(int argc, char **argv) {

    // Semente para geração de números aleatórios, garantindo que a posição da comida seja diferente a cada vez que o jogo é iniciado, criando uma experiência de jogo mais variada e imprevisível para o jogador
    srand((unsigned int)time(NULL));

    // Inicializa a cobra com um tamanho inicial de 3 segmentos, posicionando os segmentos de forma que a cabeça da cobra esteja na posição (5, 5) e os outros segmentos estejam alinhados atrás da cabeça, criando uma configuração inicial para a cobra no início do jogo
    inicializarCobra();

    // Gera uma posição inicial para a comida, garantindo que ela seja colocada em um local válido na grade e não esteja ocupada pela cobra, criando uma configuração inicial para a posição da comida no início do jogo
    glutInit(&argc, argv);

    // Define o modo de exibição para usar buffers duplos e cor RGB, garantindo que a cena do jogo seja renderizada corretamente na tela com as configurações adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Define o tamanho da janela para as constantes LARGURA_JANELA e ALTURA_JANELA, garantindo que a janela do jogo seja exibida com as dimensões adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    glutInitWindowSize(LARGURA_JANELA, ALTURA_JANELA);

    // Cria a janela do jogo com o título "Jogo da Cobrinha", garantindo que a janela seja exibida para o jogador com um título apropriado que indica o nome do jogo, criando uma experiência de jogo completa desde o início da execução do programa
    glutCreateWindow("Jogo da Cobrinha");

    // Configura as definições iniciais do OpenGL, como a cor de fundo, o modo de projeção e a matriz de modelagem, garantindo que a cena do jogo seja renderizada corretamente na tela com as configurações adequadas para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    configurarOpenGL();

    // Carrega a textura de fundo para o jogo, garantindo que o fundo seja renderizado corretamente na tela com a textura apropriada para criar uma experiência visual envolvente para o jogador, criando uma função de configuração do OpenGL que é chamada no início do programa para preparar o ambiente gráfico para o jogo
    carregarTexturaFundo();

    // Gera uma posição inicial para a comida, garantindo que ela seja colocada em um local válido na grade e não esteja ocupada pela cobra, criando uma configuração inicial para a posição da comida no início do jogo
    gerarComida();

    // Registra a função de callback para renderizar a cena do jogo, garantindo que a função display seja chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente, criando uma mecânica de renderização para o jogo
    glutDisplayFunc(display);

    // Registra a função de callback para manipular as entradas do teclado, garantindo que a função teclado seja chamada quando o jogador pressiona uma tecla para controlar a direção da cobra, reiniciar o jogo ou encerrar o jogo, criando uma mecânica de controle de entrada para o jogo
    glutKeyboardFunc(teclado);

    // Registra a função de callback para atualizar o estado do jogo, garantindo que a função atualizar seja chamada regularmente para mover a cobra, verificar colisões e atualizar a tela, e se o jogo não estiver encerrado, agenda a próxima atualização usando glutTimerFunc para criar um loop de atualização contínuo, garantindo que o jogo seja atualizado regularmente para criar uma experiência de jogo fluida e responsiva, e que o estado do jogo seja verificado para detectar colisões e encerrar o jogo adequadamente quando necessário, criando uma mecânica de atualização para o jogo
    glutTimerFunc(INTERVALO_ATUALIZACAO, atualizar, 0);

    // Entra no loop principal do GLUT para processar eventos e renderizar a cena do jogo, garantindo que o jogo seja iniciado corretamente e que a janela do jogo seja exibida para o jogador, criando uma experiência de jogo completa desde o início da execução do programa
    glutMainLoop();

    return 0;
}