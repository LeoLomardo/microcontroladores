//blocos com nomes "ESQUERDA", "DIREITA", "CIMA", "BAIXO", "ACAO", "UM", "DOIS", "TRES", "QUATRO",
//"CINCO", "SEIS", "LOOP", "IF" (sem acentos, cedilha e sem espaços!!) organizados em lista com
//blocos da esquerda para a direita da linha de cima, seguidos da linha de baixo
#include <GFButton.h>

bool ehAcao(String bloco) {
  return (bloco == "DIREITA" || bloco == "ESQUERDA" || bloco == "CIMA" || bloco == "BAIXO" || bloco == "ACAO");
}  //recebe um bloco e retorna True se o bloco for açao (movimento ou ação do personagem)

bool ehNumeroPuro(String bloco) {
  return (bloco == "UM" || bloco == "DOIS" || bloco == "TRES" || bloco == "QUATRO" || bloco == "CINCO" || bloco == "SEIS");
}  //recebe um bloco e retorna True se o bloco for número

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
}  //recebe string do bloco e retorna o int correspondente. Se receber vazio, retorna 1 para a
//ação ser executada uma vez apenas

GFButton botao(2);
String filaDeComandos[18];
int comandoAtual = 0;
unsigned long tempoAnterior = 0;
bool interpretacaoConcluida = false;
String lista[16] = {
  "DIREITA", "IF", "CIMA", "BAIXO", "ESQUERDA", "LOOP", "VAZIO", "ACAO",
  "DOIS", "QUATRO", "VAZIO", "VAZIO", "VAZIO", "UM", "DOIS", "VAZIO"
};
String cima[8];
String baixo[8];
int indiceRetorno = -1;
int jaFiz = 0;
int repeticoes = 0;
bool acabou = false;
bool botaoPressionado = false;

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
      if (!ehNumeroPuro(baixo[i + 1])&&baixo[i+1]!="VAZIO") {
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
}  //funcao que recebe uma lista de blocos e printa erro na serial e retorna false se tiver erro.
//se não tiver erro, retorna True.

void apertou(GFButton& botao) {
  botaoPressionado = true;
}
void soltou(GFButton& botao) {
  botaoPressionado = false;
}

void setup() {
  Serial.begin(9600);
  interpretacaoConcluida = interpretarComandos(lista);
  botao.setPressHandler(apertou);
  botao.setReleaseHandler(soltou);
}

void loop() {
  botao.process();
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
          jaFiz += 1;
          indiceRetorno = blocoParaNumero(baixo[comandoAtual]) - 1;
          repeticoes = blocoParaNumero(baixo[comandoAtual + 1]);
          if(baixo[comandoAtual+1]=="VAZIO")
          {
            jaFiz=0;
          }
          if (repeticoes > jaFiz) {
            comandoAtual = indiceRetorno;
          }
          else {
            jaFiz = 0;
            comandoAtual++;
          }
        } 
        else if (cima[comandoAtual] == "IF") {
          if (botaoPressionado) {
            comandoAtual++;
          }
          else {
            comandoAtual = blocoParaNumero(baixo[comandoAtual]) - 1;
          }
        } 
        else {
          comandoAtual++;
        }
      }
    }
    if (comandoAtual == 8 && acabou == false) {
      acabou = true;
      Serial.println("FIM");
    }
  }
}
