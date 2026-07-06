#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#define MAX_FILA 100
MCUFRIEND_kbv tela;

// --- CORES DA TELA CONVERTIDAS DO NEOPIXEL ---
#define TELA_PRETA         0x0000 // Fundo
#define TELA_BRANCA        0xFFFF // Detalhes e Pós-Loop
#define TELA_VERDE_SUAVE    0x3620 // Frente/Trás: RGB(50, 200, 0)
#define TELA_AZUL     0x001F // Direita/Esquerda: RGB(0, 0, 255)
#define TELA_LARANJA  0xFC60 // Loop: RGB(255, 140, 0)
#define TELA_VERMELHO 0xF800 // Derrota: RGB(255, 0, 0)
#define TELA_VERDE  0x07E0 // Vitória: RGB(0, 255, 0) 
#define TELA_ROXO 0xF81F // Ação


// Pinos no Arduíno MEGA

int campainha = 26;
int pinoLed = 24;

unsigned long instanteAnterior = 0;
int contagemPiscadas = 0;
bool estadoLed = false;
bool estaPiscando = false;
uint32_t corAtual;
int frequenciaAtual;

int tabuleiro[2][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0}, // Linha 0 (Física 1): Comandos
  {0, 0, 0, 0, 0, 0, 0, 0}  // Linha 1 (Física 2): Quantidade ou Índice
};


int filaTipos[MAX_FILA];
int filaPosicoes[MAX_FILA];

int tamanhoFila = 0;
int indiceFila = 0;
unsigned long tempoFila = 0;

// Cor atual da telinha

uint16_t corTelaAtual; 

// Inicialização da Fita

Adafruit_NeoPixel fita = Adafruit_NeoPixel(16, pinoLed, NEO_GRB + NEO_KHZ800);

uint32_t VERDE_SUAVE = fita.Color(50, 200, 0); // Frente/Trás
uint32_t AZUL    = fita.Color(0, 0, 255);   // Direita/Esquerda
uint32_t LARANJA   = fita.Color(255, 140, 0); // Loop
uint32_t BRANCO   = fita.Color(255, 255, 255); // Pós-Loop (Contador)

uint32_t VERDE   = fita.Color(0, 255, 0);   // Caso de Vitória
uint32_t VERMELHO = fita.Color(255, 0, 0);   // Caso de Derrota

uint32_t ROXO = fita.Color(128, 0, 128); // Ação



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(10);
  Serial1.begin(9600);      // Liga os Pinos 19(RX) e 18(TX)
  Serial1.setTimeout(100);  // Tempo para ler a string sem cortar no meio
  pinMode(campainha, OUTPUT);
  fita.begin();
  fita.show();
  fita.setBrightness(50);
  tela.begin( tela.readID() );
  tela.setRotation(1); // Para deitar a telinha (horizontal)
  tela.fillScreen(TELA_PRETA);
}


void mostrarErroMatriz(String motivo) {
  fita.clear(); 
  fita.show();
  tone(campainha, 150, 800); 
  // O -1 significa "Nenhuma coluna rodando agora"
  desenharPainel(-1, "ERRO: " + motivo, TELA_VERMELHO); 
}

void mostrarSucessoInsercao(String comando, int linha, int coluna, int tipo) {
  fita.clear();
   fita.show();
  String msg = "INSERIDO: " + comando;
  msg.toUpperCase();
  desenharPainel(-1, msg, TELA_BRANCA); 
}


// Função que recebe o número da instrução, e de que tipo ela é (frente/trás, direita/esquerda, ...)

void novaInstrucao(int colunaFisica, int tipo) {
  uint32_t corLed;
  
  switch(tipo) {
    case 1:  
    case 5: // Trás acende Verde também
      corLed = VERDE_SUAVE;
      tone(campainha, 330, 500);
      break;
    case 2: 
    case 6: // Esquerda acende Azul também
      corLed = AZUL; 
      tone(campainha, 550, 500); 
      break;
    case 7: // Ação
      corLed = ROXO;
      tone(campainha, 440, 500); 
      break;
    default: 
      corLed = 0; 
  }

  fita.clear();
  fita.setPixelColor(colunaFisica * 2, corLed); 
  fita.show();

  // Manda a tela atualizar dizendo exatamente qual coluna física está ativa
  desenharPainel(colunaFisica, "EXECUTANDO...", TELA_BRANCA);
}

// Função acionada em caso de vitória/derrota

void iniciarPisca(uint32_t cor, int freq, uint16_t corTela_param) {
  corAtual = cor;
  corTelaAtual = corTela_param;
  frequenciaAtual = freq;
  contagemPiscadas = 0;
  estaPiscando = true;
  instanteAnterior = millis();
}

void vitoria() {
  iniciarPisca(VERDE, 660, TELA_VERDE);
}

void derrota() {
  iniciarPisca(VERMELHO, 880, TELA_VERMELHO);
}

// Função auxiliar para desenhar na telinha em caso de vitória ou derrota
void desenharSimboloFimDeJogo(uint16_t corTela) {
  if (corTela == TELA_VERDE) {
    // É vitória -> Checkzinho Verde Grosso
    for(int i=0; i<7; i++) {
      tela.drawLine(60+i, 160, 110+i, 210, corTela);
    }
    for(int i=0; i<7; i++) {
      tela.drawLine(110+i, 210, 190+i, 110,corTela);
    } 
  } 
  else if (corTela == TELA_VERMELHO) {
    // É derrota -> X Vermelho Grosso
    for(int i=0; i<7; i++) {
      tela.drawLine(60+i, 100, 180+i, 220,corTela);
    }
    for(int i=0; i<7; i++) {
      tela.drawLine(180+i, 100, 60+i, 220,corTela);
    }
  }
}

void desenharPainel(int colunaAtiva, String mensagemSup, uint16_t corMsg) {
  
  tela.fillRect(0, 0, 320, 50, TELA_PRETA);
  
  // Mensagem de Cabeçalho
  tela.setTextSize(2);
  tela.setTextColor(corMsg);
  tela.setCursor(10, 10);
  tela.print(mensagemSup);

  // Variáveis de geometria para caber 8 colunas em 320 pixels
  int startX = 8;    // Margem esquerda
  int startY = 50;   // Margem superior
  int tam = 34;      // Tamanho do bloco (Quadrado 34x34)
  int espaco = 38;   // Passo de uma coluna para a outra

  // Varre as 8 colunas desenhando a matriz
  for(int c = 0; c < 8; c++) {
     int x = startX + (c * espaco);

     //  Linha 1 (Comandos)
     int tipo = tabuleiro[0][c];
     if(tipo == 0) {
        // Se estiver vazio, desenha só a cordinha (borda cinza escura)
        tela.fillRect(x, startY, tam, tam, TELA_PRETA);
        tela.drawRect(x, startY, tam, tam, 0x7BEF); 
     } 
     else {
        uint16_t corB = TELA_PRETA;
        if(tipo == 1 || tipo == 5){
          corB = TELA_VERDE_SUAVE;
        } 
        else if(tipo == 2 || tipo == 6){
          corB = TELA_AZUL;
        } 
        else if(tipo == 3){
          corB = TELA_LARANJA; 
        }
        else if(tipo == 4){
          corB = TELA_BRANCA;
        } 

        else if(tipo == 7){
        corB = TELA_ROXO;
      }
        
        tela.fillRect(x, startY, tam, tam, corB);
        
        // Mini-Símbolos dentro dos blocos de 34x34
        if(tipo == 1){
          tela.fillTriangle(x+17, startY+6, x+6, startY+26, x+28, startY+26, TELA_BRANCA); // Seta Frente (Cima)
        }
        else if(tipo == 2){
          tela.fillTriangle(x+26, startY+17, x+8, startY+6, x+8, startY+28, TELA_BRANCA); // Seta Direita
        }
        else if(tipo == 3){
          tela.drawCircle(x+17, startY+17, 10, TELA_BRANCA); // Loop
        }
        else if(tipo == 4) {
          tela.setTextColor(TELA_PRETA); 
          tela.setCursor(x+11, startY+10); 
          tela.print("X"); 
        }
        else if(tipo == 5){
          tela.fillTriangle(x+17, startY+28, x+6, startY+8, x+28, startY+8, TELA_BRANCA); // Seta Trás (Baixo)
        }
        else if(tipo == 6){
          tela.fillTriangle(x+8, startY+17, x+26, startY+6, x+26, startY+28, TELA_BRANCA); // Seta Esquerda
        }
        else if(tipo == 7) {
        tela.setTextColor(TELA_BRANCA); 
        tela.setCursor(x+11, startY+10); 
        tela.print("!"); 
      }

     }

     // Linha 2 (Números) 
     int num = tabuleiro[1][c];
     int yNum = startY + espaco; // Fica logo abaixo da primeira linha
     
     if(num == 0) {
        tela.fillRect(x, yNum, tam, tam, TELA_PRETA);
        tela.drawRect(x, yNum, tam, tam, 0x7BEF); // Vazio
     } 
     else {
        tela.fillRect(x, yNum, tam, tam, TELA_BRANCA);
        tela.setTextColor(TELA_PRETA); 
        tela.setCursor(x + 11, yNum + 10);
        tela.print(num); // Imprime o número
     }

     // Se essa coluna for a instrução que está rodando agora
     if(c == colunaAtiva) {
        // Desenha a bolinha azul de rastreio
        tela.fillCircle(x + 17, yNum + 45, 6, TELA_AZUL);
     }
     else{
        tela.fillCircle(x + 17, yNum + 45, 6, TELA_PRETA);
     }
  }
}

void loop() {
  
  if (estaPiscando) {
    if (millis() - instanteAnterior >= 300) {
      instanteAnterior = millis();
      estadoLed = !estadoLed;
      
      if (estadoLed) {
        fita.fill(corAtual);
        tela.fillScreen(TELA_PRETA);
        desenharSimboloFimDeJogo(corTelaAtual);
        tone(campainha, frequenciaAtual, 300);
        contagemPiscadas++;
      } 
      else {
        fita.clear();
        tela.fillScreen(TELA_PRETA); 
      }
      fita.show();
      
      if (contagemPiscadas >= 3 && !estadoLed){
        estaPiscando = false;
        fita.clear();
        tela.fillScreen(TELA_PRETA);
        fita.show();
      } 
    }
  }
  
  if (!estaPiscando && indiceFila < tamanhoFila) {
    if (millis() - tempoFila >= 1000) { 
      tempoFila = millis();
      novaInstrucao(filaPosicoes[indiceFila], filaTipos[indiceFila]);
      indiceFila++;
      
      if(indiceFila == tamanhoFila) {
         tela.fillScreen(TELA_PRETA); 
         fita.clear();
         fita.show();
      }
    }
  }

  // Leitura Serial
  String texto = "";
  if (Serial1.available() > 0) {
    texto = Serial1.readStringUntil('\n');
    Serial.println("Recebido do outro arduino: [" + texto + "]");
  }
  else if (Serial.available() > 0) {
    texto = Serial.readStringUntil('\n');
    Serial.println("Recebido do PC: [" + texto + "]");
  }

  if(texto!=""){
    texto.trim();
  
    if (texto.startsWith("ini ")) {
      // Zera a matriz e apaga a fita na memória
      for(int l=0; l<2; l++) {
        for(int c=0; c<8; c++) {
          tabuleiro[l][c] = 0;
        }
      }
      fita.clear();
      fita.show();

      // Remove o "ini " do começo da frase
      texto = texto.substring(4); 

      // Lê de 3 em 3 palavras
      while (texto.length() > 0) {
        int esp1 = texto.indexOf(' ');
        if (esp1 == -1){
          break; // Proteção contra strings cortadas
        }
        int esp2 = texto.indexOf(' ', esp1 + 1);
        if (esp2 == -1){
          break;
        }
        int esp3 = texto.indexOf(' ', esp2 + 1); // Pode não existir se for o último bloco da frase

        String strValor = texto.substring(0, esp1);
        int linhaFisica = texto.substring(esp1 + 1, esp2).toInt();
        int colunaFisica;

        if (esp3 != -1) {
          colunaFisica = texto.substring(esp2 + 1, esp3).toInt();
          texto = texto.substring(esp3 + 1); // Corta o trio lido e guarda o resto para a próxima volta
        } else {
          colunaFisica = texto.substring(esp2 + 1).toInt();
          texto = ""; // Esvazia para encerrar o laço while
        }

        // Insere na Matriz
        if (linhaFisica >= 1 && linhaFisica <= 2 && colunaFisica >= 1 && colunaFisica <= 8) {
          int l = linhaFisica - 1;
          int c = colunaFisica - 1;

          if (strValor == "remover") {
             tabuleiro[l][c] = 0;
          } 
          else if (linhaFisica == 1) {
            int tipo = 0;
            if (strValor == "frente"){
              tipo = 1;
            }
            else if (strValor == "direita"){
              tipo = 2;
            } 
            else if (strValor == "loop"){
              tipo = 3;
            }
            else if (strValor == "vezes"){
              tipo = 4;
            } 
            else if (strValor == "tras"){
              tipo = 5;
            }
            else if (strValor == "esquerda"){
              tipo = 6;
            } 
            else if (strValor == "acao"){ 
              tipo = 7; 
            }
            
            if (tipo != 0){
              tabuleiro[l][c] = tipo;
            } 
          } 
          else if (linhaFisica == 2) {
            int param = strValor.toInt();
            if (!(param == 0 && strValor != "0")) {
              tabuleiro[l][c] = param;
            }
          }
        }
      } // Fim do while

      // Atualiza a tela de uma vez só
      tela.fillScreen(TELA_PRETA);
      desenharPainel(-1, "LEITURA INICIAL OK", TELA_AZUL);
      return; // Encerra o if aqui para não bater nos comandos debaixo
    }
    
    int espaco1 = texto.indexOf(' ');
    int espaco2 = texto.lastIndexOf(' ');

    // Se possui o formato "comando linha coluna" (ex: frente 1 4)
    if (espaco1 != -1 && espaco2 != -1 && espaco1 != espaco2) {
      String strValor = texto.substring(0, espaco1);
      int linhaFisica = texto.substring(espaco1 + 1, espaco2).toInt();
      int colunaFisica = texto.substring(espaco2 + 1).toInt();

      // Barreira de Limites Físicos
      if (linhaFisica < 1 || linhaFisica > 2 || colunaFisica < 1 || colunaFisica > 8) {
        String mot = "Fora dos limites \n(Matriz 2x8)";
        mostrarErroMatriz(mot);
        return;
      }

      int l = linhaFisica - 1; // Ajusta pra linguagem C (0 e 1)
      int c = colunaFisica - 1; // Ajusta pra linguagem C (0 a 7)

      if (strValor == "remover") {
        tabuleiro[l][c] = 0; // Zera a posição na matriz
        fita.setPixelColor(c * 2, 0); // Apaga o LED correspondente na fita caso estivesse aceso
        fita.show();
        
        // Atualiza o painel mostrando a grade com o espaço vazio novamente
        desenharPainel(-1, "REMOVIDO: Linha" + String(linhaFisica) + " Coluna" + String(colunaFisica), TELA_BRANCA);
        return; // Retorna imediatamente, ignorando os filtros abaixo
      }

      // Barreira de Ocupação de Espaço
      if (tabuleiro[l][c] != 0) {
        mostrarErroMatriz("Posicao ja ocupada!");
        return; 
      }

      // Processamento Linha 1 (Comandos)
      if (linhaFisica == 1) { 
        int tipo = 0;
        if (strValor == "frente"){ tipo = 1; } 
        else if (strValor == "direita"){ 
          tipo = 2; 
        }
        else if (strValor == "loop"){ 
          tipo = 3; 
        }
        else if (strValor == "vezes"){ 
          tipo = 4; 
        }
        else if (strValor == "tras"){ 
          tipo = 5; 
        }     
        else if (strValor == "esquerda"){ 
          tipo = 6; 
        } 
        else if (strValor == "acao"){ 
            tipo = 7; 
        }
        
        if (tipo == 0) {
          mostrarErroMatriz("Comando p/ L1 \nInvalido");
          return;
        }
        
        tabuleiro[l][c] = tipo; 
        mostrarSucessoInsercao(strValor, linhaFisica, colunaFisica, tipo);
        
        // Feedback físico imediato se for Loop ou Vezes
        if(tipo == 3) {
           fita.setPixelColor(c * 2, LARANJA); 
           fita.show(); 
           tone(campainha, 770, 500);
        } else if (tipo == 4) {
           fita.setPixelColor(c * 2, BRANCO); 
           fita.show(); 
           tone(campainha, 110, 500);
        }

      // Processamento Linha 2 (Números/Índices)
      } 
      else if (linhaFisica == 2) { 
        int param = strValor.toInt();
        if (param == 0 && strValor != "0") { // Proteção contra texto na L2
          mostrarErroMatriz("L2 exige um numero!");
          return;
        }
        tabuleiro[l][c] = param; 
        mostrarSucessoInsercao(strValor, linhaFisica, colunaFisica, 0);
      }
    } 
    // Comandos Globais
    else {
      if (texto == "vitoria"){
        vitoria();
      }
      else if (texto == "derrota"){
        derrota();
      }
      else if (texto == "limpar") {
         // Esvazia o tabuleiro na memória
         for(int l=0; l<2; l++) {
           for(int c=0; c<8; c++) {
             tabuleiro[l][c] = 0;
           }
         }
         fita.clear(); 
         fita.show();
         
         // Limpa a tela inteira (para matar qualquer sujeira antiga) e chama o Painel
         tela.fillScreen(TELA_PRETA);
         desenharPainel(-1, "TABULEIRO LIMPO", TELA_BRANCA);
      }
      else if (texto == "iniciar") {
         tamanhoFila = 0;
         indiceFila = 0;
         
         for(int c=0; c<8; c++) {
            int tipo = tabuleiro[0][c];
            int qtd = tabuleiro[1][c];
            if (qtd == 0){
              qtd = 1;
            } // Se o jogador não colocou bloco de número na L2, anda 1 vez
            
            if (tipo == 1 || tipo == 2 || tipo == 5 || tipo == 6 || tipo == 7) {
               // Expande os movimentos normais
               for(int rep=0; rep < qtd; rep++) {
                  filaPosicoes[tamanhoFila] = c; // Guarda a coluna física para acender o LED certo
                  filaTipos[tamanhoFila] = tipo;
                  tamanhoFila++;
               }
            } 
            else if (tipo == 4) { // Achou o bloco "Vezes"
               // Procura qual era o Índice do Loop lá atrás
               int ptRetorno = -1;
               for(int busca = c-1; busca >= 0; busca--) {
                  if(tabuleiro[0][busca] == 3) {
                     ptRetorno = tabuleiro[1][busca] - 1; // Converte índice visual (ex: L2 = 1) para índice do C (0)
                     break;
                  }
               }
               
               // Se achou um loop válido antes dele, clona tudo!
               if (ptRetorno != -1 && ptRetorno >= 0 && ptRetorno < c) {
                  for(int r = 0; r < qtd - 1; r++) { // Multiplica pelo número do bloco "Vezes"
                     for(int copyC = ptRetorno; copyC < c; copyC++) { // Copia do índice do Loop até antes do Vezes
                        int copyTipo = tabuleiro[0][copyC];
                        int copyQtd = tabuleiro[1][copyC];
                        if(copyQtd == 0){
                          copyQtd = 1;
                        }
                        
                        if(copyTipo == 1 || copyTipo == 2 || copyTipo == 5 || copyTipo == 6) {
                           for(int repCopy = 0; repCopy < copyQtd; repCopy++) {
                              filaPosicoes[tamanhoFila] = copyC;
                              filaTipos[tamanhoFila] = copyTipo;
                              tamanhoFila++;
                           }
                        }
                     }
                  }
               }
            }
         }
         // Avisa que vai começar
         fita.clear(); 
         fita.show();
         
         tela.fillScreen(TELA_PRETA);
         desenharPainel(-1, "INICIANDO...", TELA_LARANJA);
         
         tempoFila = millis();
      }
    }
  }
}

