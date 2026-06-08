// ==========================================================
// COMPILAR:
// gcc moving.c -Iinclude -Llib/x64 -lfreeglut -lopengl32 -lglu32 -o moving.exe
// ==========================================================

#include <GL/freeglut.h> // criar janelas, lidar com eventos, desenhar formas básicas, etc.
#include <stdlib.h> // funções de utilidade geral, como alocação de memória, controle de processos, conversões, etc.
#include <stdio.h> // entrada e saída padrão, como printf, scanf, etc.
#include <time.h> // manipulação de data e hora, como medir o tempo decorrido, gerar números aleatórios baseados no tempo, etc.
#include <math.h> // funções matemáticas, como trigonometria, exponenciação, etc.

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

// Tempo entre movimentos (milissegundos). Ajuste para tornar o jogo mais rápido ou mais lento.
#define INTERVALO_ATUALIZACAO  150

// Usado para desenhar círculos.
#define PI 3.14159f

// ==========================================================
// ESTRUTURAS
// ==========================================================

// Estrutura para representar um segmento da cobra
typedef struct {
    int x; // Representa a coluna da grade.
    int y; // Representa a linha da grade.
} Segmento;

// ==========================================================
// VARIÁVEIS GLOBAIS
// ==========================================================

// Armazena todos os segmentos. O primeiro segmento (índice 0) é a cabeça da cobra, e os segmentos subsequentes representam o corpo da cobra, permitindo que o jogo mantenha o estado completo da cobra e atualize suas posições conforme ela se move e cresce, criando uma estrutura de dados eficiente para representar a cobra no jogo
static Segmento cobra[MAX_COBRA];

// Tamanho atual da cobra
static int tamanhoCobra = 3;

// Direção atual da cobra
static int direcaoX = 1; // Movimento horizontal.
static int direcaoY = 0; // Movimento vertical.

// Posição da comida
static int comidaX = 10;
static int comidaY = 10;

// Pontuação do jogador
static int pontuacao = 0;

// Estado do jogo (0 = jogando, 1 = jogo encerrado)
static int jogoEncerrado = 0;

// Armazena a textura usada como plano de fundo.
static GLuint texturaFundo = 0;

// SCOREBOARD - Armazena as 5 melhores pontuações lidas do arquivo "recordes.txt" para exibição na tela de game over, permitindo que o jogo mantenha um registro dos melhores desempenhos dos jogadores e os exiba de forma informativa quando o jogo termina
int melhoresPontuacoes[5] = {0, 0, 0, 0, 0};

// Função de atualização do jogo chamada regularmente para atualizar o estado do jogo, mover a cobra, verificar colisões, atualizar a pontuação e solicitar a atualização da tela
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
// IMAGEM DE FUNDO
// ==========================================================

// Carrega a imagem de fundo a partir dos dados definidos em snake_background.h, criando uma textura OpenGL e configurando os parâmetros de filtragem para garantir que a textura seja exibida corretamente no jogo
void carregarTexturaFundo() {

    glGenTextures(1, &texturaFundo); // Gera um identificador de textura único e armazena-o na variável texturaFundo para uso posterior, garantindo que a textura seja gerenciada corretamente pelo OpenGL

    glBindTexture(GL_TEXTURE_2D, texturaFundo); // Vincula a textura gerada ao alvo GL_TEXTURE_2D, tornando-a a textura ativa para as operações subsequentes de configuração e carregamento de dados, garantindo que as operações de textura sejam aplicadas à textura correta no contexto do OpenGL

    //Parte mais importante. Ela envia a imagem para a memória da placa de vídeo.
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

    // Define o que acontece quando a textura é reduzida. O filtro GL_LINEAR suaviza a textura quando ela é reduzida, evitando que ela fique pixelada ou distorcida, garantindo que a textura seja exibida de forma visualmente agradável mesmo quando vista de longe ou em uma resolução menor do que a original
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

    glEnable(GL_TEXTURE_2D); // Diz ao OpenGL: "Agora vou desenhar usando texturas."

    glBindTexture(GL_TEXTURE_2D, texturaFundo); // Seleciona a imagem carregada anteriormente.

    glColor3f(1.0f, 1.0f, 1.0f); // define a cor branca

    // Desenha um quadrado que cobre toda a tela, mapeando a textura para ele usando coordenadas de textura que vão de 0.0 a 1.0, garantindo que a textura seja exibida corretamente em toda a área do quadrado, criando um plano de fundo visualmente consistente para o jogo
    glBegin(GL_QUADS);

        // Liga o Canto inferior esquerdo da tela ao Canto inferior esquerdo da imagem
        glTexCoord2f(0.0f, 1.0f); 
        glVertex2f(-1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f( 1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f( 1.0f,  1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f,  1.0f);

    glEnd(); // Termina de desenhar o quadrado com a textura mapeada

    glDisable(GL_TEXTURE_2D); // Desliga o uso de texturas para que outros elementos do jogo não sejam afetados pela textura do fundo
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

    FILE *arquivo = fopen("recordes.txt", "r"); // Abre o arquivo "recordes.txt" para leitura

    // Verifica se o arquivo foi aberto com sucesso. Se o arquivo não existir ou não puder ser aberto, a função retorna sem fazer nada, garantindo que o jogo possa continuar mesmo que o arquivo de recordes não esteja disponível, e evitando erros de leitura ou acesso a arquivos inexistentes
    if (arquivo == NULL) {
        return;
    }

    // Lê as pontuações do arquivo e armazena no array melhoresPontuacoes, iterando sobre as linhas do arquivo e lendo cada pontuação usando fscanf, armazenando-as no array para que possam ser exibidas na tela de recordes do jogo, garantindo que os recordes sejam carregados corretamente para uso durante o jogo
    for (int i = 0; i < 5; i++) {

        fscanf(
            arquivo,
            "%d",
            &melhoresPontuacoes[i]
        );
    }

    fclose(arquivo); //Fecha o arquivo após a leitura
}

// Salva os recordes atuais do array melhoresPontuacoes no arquivo "recordes.txt", escrevendo as pontuações armazenadas no array no arquivo para que possam ser persistidas entre as sessões de jogo, garantindo que o arquivo seja criado ou sobrescrito corretamente e que seja fechado após a escrita para evitar vazamentos de recursos
void salvarRecordes() {

    FILE *arquivo = fopen("recordes.txt", "w"); // Abre o arquivo "recordes.txt" para escrita

    if (arquivo == NULL) {
        return;
    }

    // Escreve as pontuações do array melhoresPontuacoes no arquivo, iterando sobre o array e escrevendo cada pontuação usando fprintf, garantindo que os recordes sejam salvos corretamente para persistência entre as sessões de jogo
    for (int i = 0; i < 5; i++) {

        fprintf(
            arquivo,
            "%d\n",
            melhoresPontuacoes[i]
        );
    }

    fclose(arquivo);
}

// Atualiza os recordes com a nova pontuação, verificando se a nova pontuação é maior do que alguma das pontuações armazenadas no array melhoresPontuacoes, e se for, insere a nova pontuação na posição correta do array, deslocando as pontuações menores para baixo, e depois salva os recordes atualizados no arquivo "recordes.txt" para garantir que os recordes sejam persistidos entre as sessões de jogo, criando uma função que mantém o ranking dos melhores desempenhos dos jogadores de forma eficiente e atualizada
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

    // FUNDO GRADIENTE 
    glBegin(GL_QUADS);

        glColor3f(0.02f, 0.02f, 0.02f);
        glVertex2f(-1.0f, -1.0f); 
        glVertex2f( 1.0f, -1.0f);

        glColor3f(0.15f, 0.00f, 0.00f);
        glVertex2f( 1.0f,  1.0f);
        glVertex2f(-1.0f,  1.0f);

    glEnd();

    // MOLDURA EXTERNA
    glLineWidth(6); // Espessura da borda: 6 pixels

    glColor3f(1.0f, 0.0f, 0.0f);

    // cria um retângulo sem preenchimento
    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.80f, -0.85f);
        glVertex2f( 0.80f, -0.85f);
        glVertex2f( 0.80f,  0.85f);
        glVertex2f(-0.80f,  0.85f);
    glEnd();

    // MOLDURA INTERNA
    glLineWidth(2); // Espessura da borda: 2 pixels

    glColor3f(1.0f, 0.4f, 0.4f);

    // cria um retângulo sem preenchimento
    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.76f, -0.81f);
        glVertex2f( 0.76f, -0.81f);
        glVertex2f( 0.76f,  0.81f);
        glVertex2f(-0.76f,  0.81f);
    glEnd();

    // TÍTULO
    glColor3f(1.0f, 0.1f, 0.1f);

    // Exibe a mensagem "SE FUDEU" na parte superior da tela de game over, usando a função desenharTexto para renderizar o texto na posição especificada
    desenharTexto(
        -0.18f,
        0.65f,
        "GAME OVER"
    );

    // CAIXA DA PONTUAÇÃO
    glColor3f(0.10f, 0.10f, 0.10f);

    glBegin(GL_QUADS);
        glVertex2f(-0.35f, 0.42f);
        glVertex2f( 0.35f, 0.42f);
        glVertex2f( 0.35f, 0.56f);
        glVertex2f(-0.35f, 0.56f);
    glEnd();

    char textoPontuacao[64]; // Buffer para armazenar a string formatada da pontuação

    // Formata a string da pontuação usando snprintf, garantindo que a pontuação seja convertida para uma string de forma segura e armazenada no buffer textoPontuacao para exibição na tela de game over
    snprintf(
        textoPontuacao,
        sizeof(textoPontuacao),
        "PONTUACAO: %d",
        pontuacao
    );

    glColor3f(1.0f, 1.0f, 0.0f);

    // Exibe a pontuação final do jogador na caixa de pontuação, usando a função desenharTexto para renderizar o texto formatado da pontuação na posição especificada na tela de game over
    desenharTexto(
        -0.18f,
        0.48f,
        textoPontuacao
    );

    // MENSAGEM PERSONALIZADA
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

    // LINHA SEPARADORA
    glColor3f(0.5f, 0.5f, 0.5f);

    glLineWidth(2); // Espessura da linha: 2 pixels

    glBegin(GL_LINES);
        glVertex2f(-0.55f, 0.20f);
        glVertex2f( 0.55f, 0.20f);
    glEnd();

    // CAIXA DO RANKING
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

    char texto[64]; // Buffer para armazenar a string formatada do ranking

    // Exibe as 5 melhores pontuações na caixa do ranking, iterando sobre o array melhoresPontuacoes e formatando cada pontuação em uma string usando snprintf, e depois usando a função desenharTexto para renderizar cada pontuação formatada na posição especificada na tela de game over, criando uma exibição visual dos melhores desempenhos dos jogadores para incentivar a competição e o replay do jogo
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

    // CAIXA DOS COMANDOS
    glColor3f(0.15f, 0.15f, 0.15f);

    glBegin(GL_QUADS);
        glVertex2f(-0.40f, -0.70f);
        glVertex2f( 0.40f, -0.70f);
        glVertex2f( 0.40f, -0.48f);
        glVertex2f(-0.40f, -0.48f);
    glEnd();

    glLineWidth(2); // Espessura da borda: 2 pixels

    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.40f, -0.70f);
        glVertex2f( 0.40f, -0.70f);
        glVertex2f( 0.40f, -0.48f);
        glVertex2f(-0.40f, -0.48f);
    glEnd();

    // BOTÃO REINICIAR
    glColor3f(1.0f, 1.0f, 1.0f);

    desenharTexto(
        -0.24f,
        -0.57f,
        "R - JOGAR NOVAMENTE"
    );

    // BOTÃO SAIR
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

    int tentativaMaxima = 1000; // Limite para evitar loops infinitos caso a cobra ocupe quase toda a grade
    int tentativa = 0; // Contador de tentativas para gerar uma posição válida para a comida

    do {
        // Gera coordenadas aleatórias para a comida dentro dos limites da grade, usando a função rand() para gerar números aleatórios e o operador módulo para garantir que as coordenadas estejam dentro do intervalo válido de 0 a GRID_SIZE - 1, criando uma posição aleatória para a comida no jogo
        comidaX = rand() % GRID_SIZE;
        comidaY = rand() % GRID_SIZE;

        tentativa++;

    } while (
        // Verifica se a posição gerada para a comida está ocupada por algum segmento da cobra, e se estiver, continua gerando novas posições até encontrar uma posição válida ou atingir o limite de tentativas, garantindo que a comida seja colocada em um local onde a cobra possa alcançá-la sem colisões imediatas, e evitando loops infinitos caso a cobra ocupe quase toda a grade
        posicaoOcupadaPelaCobra(comidaX, comidaY) &&
        tentativa < tentativaMaxima
    );
}

// Desenha a maçã na posição especificada, usando um círculo vermelho para representar a maçã e um retângulo marrom para representar o cabo da maçã, garantindo que a maçã seja desenhada de forma proporcional ao tamanho da grade e posicionada corretamente na tela com base nas coordenadas fornecidas
void desenharMaca(int x, int y) {

    float tamanhoCelula = obterTamanhoCelula(); // Obtém o tamanho de cada célula da grade para calcular as posições e dimensões da maçã de forma proporcional ao tamanho da grade, garantindo que a maçã seja desenhada de forma consistente e visualmente adequada em relação à grade do jogo

    // Calcula o centro da célula onde a maçã deve ser desenhada, convertendo as coordenadas da grade para coordenadas de tela e ajustando para o centro da célula, garantindo que a maçã seja posicionada corretamente no centro da célula correspondente às coordenadas fornecidas
    float centroX =
        converterGridParaTelaX(x)
        + (tamanhoCelula / 2.0f);

    float centroY =
        converterGridParaTelaY(y)
        + (tamanhoCelula / 2.0f);

    float raio = tamanhoCelula * 0.30f; // Define o raio da maçã como uma fração do tamanho da célula para garantir que a maçã seja desenhada de forma proporcional ao tamanho da grade, criando uma aparência visualmente consistente para a maçã em relação à grade do jogo

    // Corpo da maçã
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        // Define o centro do círculo como o primeiro vértice do GL_TRIANGLE_FAN, garantindo que o círculo seja desenhado a partir do centro da maçã e se expanda para fora a partir desse ponto, criando um efeito visual de uma maçã sólida e bem definida
        glVertex2f(centroX, centroY);

        // Itera para criar os vértices do círculo que formam o corpo da maçã, calculando as posições dos vértices usando funções trigonométricas para criar um círculo suave e arredondado, garantindo que a maçã seja desenhada de forma visualmente atraente e consistente com a forma típica de uma maçã, e usando um número suficiente de vértices (40 neste caso) para criar um círculo suave sem parecer pixelado ou angular
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

    // Desenha um retângulo estreito para representar o cabo da maçã, posicionando-o acima do corpo da maçã e centralizado em relação ao centro da maçã, garantindo que o cabo seja desenhado de forma visualmente consistente com a forma típica de uma maçã e que esteja corretamente posicionado para criar uma aparência visualmente atraente para a maçã no jogo
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

    // Desenha um círculo branco para representar o globo ocular do olho da cobra, usando GL_TRIANGLE_FAN para criar um círculo sólido e suave, garantindo que o olho seja desenhado de forma visualmente atraente e consistente com a aparência típica de um olho, e usando um número suficiente de vértices (20 neste caso) para criar um círculo suave sem parecer pixelado ou angular, e posicionando o centro do círculo de acordo com as coordenadas fornecidas para criar um olho expressivo para a cobra no jogo
    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY); // Define o centro do círculo como o primeiro vértice do GL_TRIANGLE_FAN, garantindo que o círculo seja desenhado a partir do centro do olho e se expanda para fora a partir desse ponto, criando um efeito visual de um olho sólido e bem definido

        // Itera para criar os vértices do círculo que formam o globo ocular, calculando as posições dos vértices usando funções trigonométricas para criar um círculo suave e arredondado, garantindo que o olho seja desenhado de forma visualmente atraente e consistente com a forma típica de um olho, e usando um número suficiente de vértices (20 neste caso) para criar um círculo suave sem parecer pixelado ou angular
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

        // Desenha um círculo preto menor para representar a pupila do olho da cobra, usando GL_TRIANGLE_FAN para criar um círculo sólido e suave, garantindo que a pupila seja desenhada de forma visualmente atraente e consistente com a aparência típica de uma pupila, e usando um número suficiente de vértices (20 neste caso) para criar um círculo suave sem parecer pixelado ou angular, e posicionando o centro do círculo de acordo com as coordenadas fornecidas para criar um olho expressivo para a cobra no jogo
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

    // Calcula o centro da célula onde a cabeça da cobra deve ser desenhada, convertendo as coordenadas da grade para coordenadas de tela e ajustando para o centro da célula, garantindo que a cabeça seja posicionada corretamente no centro da célula correspondente às coordenadas fornecidas
    float centroX =
        converterGridParaTelaX(x)
        + (tamanhoCelula / 2.0f);
    float centroY =
        converterGridParaTelaY(y)
        + (tamanhoCelula / 2.0f);

    float raioCabeca = tamanhoCelula * 0.55f; // Define o raio da cabeça da cobra como uma fração do tamanho da célula para garantir que a cabeça seja desenhada de forma proporcional ao tamanho da grade, criando uma aparência visualmente consistente para a cabeça da cobra em relação à grade do jogo

    // Cabeça
    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

        glVertex2f(centroX, centroY);

        // Itera para criar os vértices do círculo que formam a cabeça da cobra, calculando as posições dos vértices usando funções trigonométricas para criar um círculo suave e arredondado, garantindo que a cabeça seja desenhada de forma visualmente atraente e consistente com a forma típica de uma cabeça de cobra, e usando um número suficiente de vértices (40 neste caso) para criar um círculo suave sem parecer pixelado ou angular, e posicionando o centro do círculo de acordo com as coordenadas fornecidas para criar uma cabeça expressiva para a cobra no jogo
        for (int i = 0; i <= 40; i++) {

            float angulo = (2.0f * PI * i) / 40.0f;

            glVertex2f(
                centroX + (cosf(angulo) * raioCabeca),
                centroY + (sinf(angulo) * raioCabeca)
            );
        }

    glEnd();

    // Deslocamento dos olhos com base na direção da cobra para criar uma aparência expressiva, definindo um deslocamento para os olhos e ajustando suas posições de acordo com a direção atual da cobra, garantindo que os olhos sejam posicionados de forma visualmente consistente com a direção em que a cobra está se movendo, criando uma aparência mais dinâmica e expressiva para a cabeça da cobra no jogo
    float deslocamentoOlho = 0.015f;

    float olho1X = 0.0f;
    float olho1Y = 0.0f;

    float olho2X = 0.0f;
    float olho2Y = 0.0f;

    // Ajusta a posição dos olhos com base na direção da cobra para criar uma aparência expressiva, verificando a direção atual da cobra e definindo os deslocamentos dos olhos de acordo, garantindo que os olhos sejam posicionados de forma visualmente consistente com a direção em que a cobra está se movendo, criando uma aparência mais dinâmica e expressiva para a cabeça da cobra no jogo
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

    // Desenha os olhos da cobra usando a função desenharOlho para cada olho, passando as coordenadas ajustadas para posicionar os olhos de forma expressiva de acordo com a direção da cobra, garantindo que os olhos sejam desenhados de forma visualmente consistente com a direção em que a cobra está se movendo, criando uma aparência mais dinâmica e expressiva para a cabeça da cobra no jogo
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

    // Calcula o tamanho de cada célula da grade para usar como base para desenhar o segmento do corpo da cobra de forma proporcional ao tamanho da grade, garantindo que o segmento seja desenhado de forma visualmente consistente em relação à grade do jogo e que se encaixe corretamente dentro das células da grade
    float tamanhoCelula =
        obterTamanhoCelula();
    float posicaoX =
        converterGridParaTelaX(x);
    float posicaoY =
        converterGridParaTelaY(y);

    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);

        glVertex2f(posicaoX, posicaoY);

        // Desenha um quadrado para representar o segmento do corpo da cobra, usando as coordenadas calculadas para posicionar o quadrado corretamente na tela de acordo com as coordenadas fornecidas, garantindo que o segmento do corpo seja desenhado de forma visualmente consistente com a grade do jogo e que esteja claramente diferenciado da cabeça da cobra para melhorar a clareza visual do jogo
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

// Encerra o jogo definindo a variável de estado jogoEncerrado para 1 (verdadeiro), atualizando os recordes com a pontuação final do jogador, e exibindo uma mensagem de game over com a pontuação final, criando uma função que é chamada quando o jogo termina para atualizar o estado do jogo, persistir os recordes e fornecer feedback visual ao jogador sobre seu desempenho no jogo, criando uma experiência de encerramento de jogo completa e informativa para o jogador
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

// Move a cobra atualizando as posições dos segmentos do corpo para seguir a cabeça, e depois move a cabeça da cobra na direção atual, garantindo que a cobra se mova de forma fluida e consistente com a direção definida, criando uma mecânica de movimento para a cobra no jogo, onde os segmentos do corpo seguem a posição do segmento anterior, e a cabeça se move de acordo com a direção atual, criando uma experiência de jogo envolvente e responsiva para o jogador
void moverCobra() {

    Segmento antigaCauda = cobra[tamanhoCobra - 1]; // Armazena a posição da cauda antiga para corrigir o crescimento da cobra caso ela coma a comida, garantindo que o segmento adicional seja adicionado na posição correta quando a cobra crescer, criando uma mecânica de crescimento para a cobra no jogo

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
// DESENHAR PONTUAÇÃO
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

// ==========================================================
// RENDERIZAÇÃO
// ==========================================================

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

    desenharFundo(); // Desenha o fundo do jogo, criando uma base visual para os elementos do jogo e garantindo que a tela seja preenchida com uma cor de fundo consistente para melhorar a clareza visual do jogo

    desenharMaca(comidaX, comidaY); // Desenha a comida (maçã) na posição atual da comida

    // Desenha a cobra iterando sobre cada segmento da cobra e chamando as funções de desenho apropriadas para a cabeça e o corpo, garantindo que a cobra seja desenhada de forma clara e visualmente consistente na tela, criando uma mecânica de renderização para a cobra que exibe corretamente a posição e a aparência da cobra no jogo
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

    desenharPontuacao(); // Desenha a pontuação atual do jogador na tela.

    glutSwapBuffers(); // Solicita a troca dos buffers para exibir a cena renderizada, garantindo que a tela seja atualizada corretamente para refletir o estado atual do jogo e criar uma experiência visual envolvente para o jogador, criando uma mecânica de renderização que é chamada regularmente para atualizar a tela do jogo e exibir os elementos do jogo de forma clara e visualmente atraente
}

// ==========================================================
// ATUALIZAÇÃO
// ==========================================================

// Atualiza o estado do jogo, movendo a cobra, verificando colisões e atualizando a tela, e se o jogo não estiver encerrado, agenda a próxima atualização usando glutTimerFunc para criar um loop de atualização contínuo, garantindo que o jogo seja atualizado regularmente para criar uma experiência de jogo fluida e responsiva, e que o estado do jogo seja verificado para detectar colisões e encerrar o jogo adequadamente quando necessário, criando uma mecânica de atualização para o jogo
void atualizar(int valor) {

    (void)valor; // Evita aviso de variável não utilizada

    // Se o jogo estiver encerrado, não faz nada e retorna, garantindo que a função de atualização não continue a ser chamada ou a atualizar o estado do jogo após o término do jogo, criando uma condição de parada para o loop de atualização do jogo quando o jogo estiver encerrado
    if (jogoEncerrado) {
        return;
    }

    moverCobra(); // Move a cobra para a nova posição com base na direção atual

    // Verifica se a cobra colidiu com as paredes ou com o próprio corpo, e se sim, encerra o jogo e solicita a atualização da tela para exibir a tela de "GAME OVER", garantindo que o jogo possa detectar quando a cobra atinge as bordas da área de jogo ou se machuca, e encerrar o jogo adequadamente, criando uma mecânica de detecção de colisão para o jogo
    if (
        verificarColisaoParede() ||
        verificarColisaoCorpo()
    ) {

        encerrarJogo();

        glutPostRedisplay();

        return;
    }

    verificarComida(); // Verifica se a cobra comeu a comida e atualiza o estado do jogo adequadamente

    glutPostRedisplay(); // Solicita a atualização da tela para refletir as mudanças feitas no estado do jogo, garantindo que a nova configuração do jogo seja exibida corretamente na tela para o jogador, criando uma experiência visual consistente ao atualizar o estado do jogo

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
    // Evita aviso de variável não utilizada
    (void)x; 
    (void)y;

    // Verifica a tecla pressionada e atualiza a direção da cobra de acordo, garantindo que o jogador possa controlar a direção da cobra usando as teclas WASD, e que a direção seja atualizada de forma consistente com as regras do jogo (por exemplo, não permitindo que a cobra se mova na direção oposta imediatamente), criando uma mecânica de controle de direção para o jogo
    switch (tecla) {

        case 'w':
        case 'W':

            // Verifica se a direção atual da cobra não é para baixo (direcaoY != -1) para evitar que a cobra se mova na direção oposta imediatamente, e se for permitido, atualiza a direção da cobra para cima (direcaoX = 0, direcaoY = 1), garantindo que o jogador possa controlar a direção da cobra usando as teclas WASD, e que a direção seja atualizada de forma consistente com as regras do jogo, criando uma mecânica de controle de direção para o jogo
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

    srand((unsigned int)time(NULL)); // Inicializa aleatoriedade

    carregarRecordes(); // Carrega recordes

    inicializarCobra(); // Inicializa a cobra

    glutInit(&argc, argv); // Inicializa o GLUT

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // Configura o modo de exibição para usar buffers duplos e cor RGB

    glutInitWindowSize(LARGURA_JANELA, ALTURA_JANELA); // Define o tamanho da janela do jogo

    glutCreateWindow("Jogo da Cobrinha"); // Cria a janela do jogo com o título "Jogo da Cobrinha"

    carregarTexturaFundo(); // Carrega a textura de fundo do jogo

    gerarComida(); // Gera a posição inicial da comida

    glutDisplayFunc(display); // Define a função de renderização para desenhar a cena do jogo

    glutKeyboardFunc(teclado); // Define a função de teclado para processar as entradas do jogador

    glutTimerFunc(INTERVALO_ATUALIZACAO, atualizar, 0); // Inicia o timer de atualização para mover a cobra e atualizar o estado do jogo regularmente

    glutMainLoop(); // Entra no loop principal do GLUT para processar eventos e renderizar a cena do jogo

    return 0;
}
