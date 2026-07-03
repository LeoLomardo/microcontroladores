#include <GFButton.h>

GFButton botao(2);
String lista[16];
String cima[8];
String baixo[8];
int comandoAtual = 0;
unsigned long tempoAnterior = 0;
bool interpretacaoConcluida = false;
int indiceRetorno = -1;
int jaFiz = 0;
int repeticoes = 0;
bool acabou = false;
bool botaoPressionado = false;
bool iniciado = false;

int casasMovimento[8];
int casasQuantidade[8];
int casasMovimentoAnterior[8];
int casasQuantidadeAnterior[8];
int intervalosAnalog[15] = { 0, 137, 178, 221, 285, 372, 465, 548, 625, 673, 767, 819, 887, 943, 1023 };
unsigned long tempAtual, tempAnterior = 0;
String superComando = "ini ";

bool ehAcao(String bloco) {
  return (bloco == "DIREITA" || bloco == "ESQUERDA" || bloco == "CIMA" || bloco == "BAIXO" || bloco == "ACAO");
}

bool ehNumeroPuro(String bloco) {
  return (bloco == "UM" || bloco == "DOIS" || bloco == "TRES" || bloco == "QUATRO" || bloco == "CINCO" || bloco == "SEIS");
}

int blocoParaNumero(String bloco) {
  if (bloco == "UM") {
    return 1;
  }
  if (bloco == "DOIS") {
    return 2;
  }
  if (bloco == "TRES") {
    return 3;
  }
  if (bloco == "QUATRO") {
    return 4;
  }
  if (bloco == "CINCO") {
    return 5;
  }
  if (bloco == "SEIS") {
    return 6;
  }
  if (bloco == "VAZIO") {
    return 1;
  }
  return 0;
}

bool interpretarComandos(String lista[16]) {
  for (int i = 0; i < 8; i++) {
    cima[i] = lista[i];
    baixo[i] = lista[i + 8];
  }
  
  for (int i = 0; i < 8; i++) {
    String b = baixo[i];
    if (b == "VAZIO") {
      continue;
    }
    if (b == "LOOP" || b == "IF" || ehAcao(b)) {
      Serial.println("ERRO"); 
      return false;
    }
    if (ehNumeroPuro(b)) {
      bool valido = ehAcao(cima[i]) || cima[i] == "LOOP" || cima[i] == "IF" || (cima[i] == "VAZIO" && i > 0 && cima[i - 1] == "LOOP");
      if (!valido) { 
        Serial.println("ERRO"); 
        return false; 
      }
    }
  }
  
  for (int i = 0; i < 8; i++) {
    String c = cima[i];
    if (c == "VAZIO") {
      continue;
    }
    if (ehNumeroPuro(c)) {
      Serial.println("ERRO"); 
      return false;
    }
    if (c == "IF") {
      if (!ehNumeroPuro(baixo[i])) {
        Serial.println("ERRO"); 
        return false;
      }
    }
    if (c == "LOOP") {
      if (i + 1 >= 8 || cima[i + 1] != "VAZIO") {
        Serial.println("ERRO"); 
        return false;
      }
      if (!ehNumeroPuro(baixo[i])) {
        Serial.println("ERRO"); 
        return false;
      }
      if (!ehNumeroPuro(baixo[i + 1]) && baixo[i + 1] != "VAZIO") {
        Serial.println("ERRO"); 
        return false;
      }
      int indiceRetorno = blocoParaNumero(baixo[i]) - 1;
      if (indiceRetorno >= i) {
        Serial.println("ERRO"); 
        return false;
      }
    }
  }
  return true;
}

void apertou(GFButton& b) { 
  botaoPressionado = true;  
}

void soltou(GFButton& b) { 
  botaoPressionado = false; 
}

int identBloco(int valor) {
  for (int i = 0; i < 14; i++) {
    if (intervalosAnalog[i] <= valor && intervalosAnalog[i + 1] > valor) {
      return i;
    }
  }
  if (valor == 1023) {
    return 13;
  }
  return 0;
}

String converteComando(int tipo, int posicao, int linha) {
  if (tipo == 0) {
    return "remover " + String(linha) + " " + String(posicao);
  }
  if (tipo >= 1 && tipo <= 6) {
    return String(tipo) + " " + String(linha) + " " + String(posicao);
  }
  if (tipo == 7) {
    return "acao " + String(linha) + " " + String(posicao);
  }
  if (tipo == 8) {
    return "if " + String(linha) + " " + String(posicao);
  }
  if (tipo == 9) {
    return "loop " + String(linha) + " " + String(posicao);
  }
  if (tipo == 10) {
    return "direita " + String(linha) + " " + String(posicao);
  }
  if (tipo == 11) {
    return "esquerda " + String(linha) + " " + String(posicao);
  }
  if (tipo == 12) {
    return "tras " + String(linha) + " " + String(posicao);
  }
  if (tipo == 13) {
    return "frente " + String(linha) + " " + String(posicao);
  }
  return "remover " + String(linha) + " " + String(posicao);
}

void vetorParaVetor(int antigo[], int atual[]) {
  for (int i = 0; i < 8; i++) {
    antigo[i] = atual[i];
  }
}

String intParaBloco(int tipo) {
  if (tipo == 0) {
    return "VAZIO";
  }
  if (tipo == 1) {
    return "UM";
  }
  if (tipo == 2) {
    return "DOIS";
  }
  if (tipo == 3) {
    return "TRES";
  }
  if (tipo == 4) {
    return "QUATRO";
  }
  if (tipo == 5) {
    return "CINCO";
  }
  if (tipo == 6) {
    return "SEIS";
  }
  if (tipo == 7) {
    return "ACAO";
  }
  if (tipo == 8) {
    return "IF";
  }
  if (tipo == 9) {
    return "LOOP";
  }
  if (tipo == 10) {
    return "DIREITA";
  }
  if (tipo == 11) {
    return "ESQUERDA";
  }
  if (tipo == 12) {
    return "BAIXO";
  }
  if (tipo == 13) {
    return "CIMA";
  }
  return "VAZIO";
}

void lerTabuleiro() {
  superComando = "ini ";
  for (int i = 0; i < 8; i++) {
    int valor = analogRead(A0 + i);
    for (int k = 0; k < 4; k++) {
      valor += analogRead(A0 + i);
    }
    int media = valor / 5;
    casasMovimento[i] = identBloco(media);
    int quant = analogRead(A0 + i + 8);
    for (int k = 0; k < 4; k++) {
      quant += analogRead(A0 + i + 8);
    }
    int medQuant = quant / 5;
    casasQuantidade[i] = identBloco(medQuant);
    lista[i] = intParaBloco(casasMovimento[i]);
    lista[i + 8] = intParaBloco(casasQuantidade[i]);
    superComando += converteComando(casasMovimento[i], i + 1, 1) + " ";
    superComando += converteComando(casasQuantidade[i], i + 1, 2) + " ";
  }
}

void atualizarTabuleiro() {
  if (tempAtual - tempAnterior > 600) {
    tempAnterior = tempAtual;
    for (int i = 0; i < 8; i++) {
      int valor = analogRead(A0 + i);
      for (int k = 0; k < 4; k++) {
        valor += analogRead(A0 + i);
      }
      int media = valor / 5;
      casasMovimento[i] = identBloco(media);
      int quant = analogRead(A0 + i + 8);
      for (int k = 0; k < 4; k++) {
        quant += analogRead(A0 + i + 8);
      }
      int medQuant = quant / 5;
      casasQuantidade[i] = identBloco(medQuant);
    }
    for (int i = 0; i < 8; i++) {
      if (casasMovimentoAnterior[i] != casasMovimento[i]) {
        if (casasMovimento[i] != 0) {
          Serial1.println(converteComando(0, i + 1, 1));
        }
        Serial1.println(converteComando(casasMovimento[i], i + 1, 1));
      }
      if (casasQuantidadeAnterior[i] != casasQuantidade[i]) {
        if (casasQuantidade[i] != 0) {
          Serial1.println(converteComando(0, i + 1, 2));
        }
        Serial1.println(converteComando(casasQuantidade[i], i + 1, 2));
      }
    }
    vetorParaVetor(casasMovimentoAnterior, casasMovimento);
    vetorParaVetor(casasQuantidadeAnterior, casasQuantidade);
  }
  tempAtual = millis();
}

void verificarSerial() {
  if (Serial.available() > 0) {
    String texto = Serial.readStringUntil('\n');
    texto.trim();
    if (texto == "START" && !iniciado) {
      lerTabuleiro();
      interpretacaoConcluida = interpretarComandos(lista);
      if (interpretacaoConcluida) {
        comandoAtual = 0;
        jaFiz = 0;
        acabou = false;
        tempoAnterior = millis();
        iniciado = true;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  botao.setPressHandler(apertou);
  botao.setReleaseHandler(soltou);
  Serial1.println("limpar");
  lerTabuleiro();
  delay(3000);
  Serial1.println(superComando);
}

void loop() {
  botao.process();
  tempAtual = millis();
  verificarSerial();
  if (!iniciado) {
    atualizarTabuleiro();
    return;
  }
  if (interpretacaoConcluida && comandoAtual < 8) {
    unsigned long tempoAtual = millis();
    if (tempoAtual > tempoAnterior + 1000) {
      tempoAnterior = tempoAtual;
      if (cima[comandoAtual] == "VAZIO") {
        comandoAtual++;
      }
      if (comandoAtual < 8) {
        if (ehAcao(cima[comandoAtual])) {
          int vezes = blocoParaNumero(baixo[comandoAtual]);
          Serial.println(String(vezes) + " " + cima[comandoAtual]);
        }
        if (cima[comandoAtual] == "LOOP") {
          jaFiz++;
          indiceRetorno = blocoParaNumero(baixo[comandoAtual]) - 1;
          repeticoes = blocoParaNumero(baixo[comandoAtual + 1]);
          if (baixo[comandoAtual + 1] == "VAZIO") {
            jaFiz = 0;
          }
          if (repeticoes > jaFiz) {
            comandoAtual = indiceRetorno;
          } else {
            jaFiz = 0;
            comandoAtual++;
          }
        } else if (cima[comandoAtual] == "IF") {
          if (botaoPressionado) {
            comandoAtual++;
          } else {
            comandoAtual = blocoParaNumero(baixo[comandoAtual]) - 1;
          }
        } else {
          comandoAtual++;
        }
      }
    }
    if (comandoAtual == 8 && !acabou) {
      acabou = true;
      iniciado = false;
      Serial.println("FIM");
    }
  }
}
