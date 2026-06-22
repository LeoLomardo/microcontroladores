from serial import Serial
from serial.serialutil import SerialException


MOVIMENTO_POR_DIRECAO = {
    "CIMA": (0, -1),
    "BAIXO": (0, 1),
    "DIREITA": (1, 0),
    "ESQUERDA": (-1, 0),
}

# Estado do modulo: a conexao com o Arduino e o pedaco de linha ainda incompleto.
_conexao = None
_buffer = ""


def conectar(porta, baudrate=9600):

    global _conexao, _buffer
    _buffer = ""

    try:
        # timeout=0 deixa a leitura nao-bloqueante: read() devolve na hora
        # apenas o que ja chegou, sem travar o loop do jogo.
        _conexao = Serial(porta, baudrate=baudrate, timeout=0)
        print(f"Conectado ao Arduino em {porta}.")
    except SerialException as erro:
        _conexao = None
        print(f"Nao foi possivel abrir a porta serial {porta}: {erro}")

def interpretar(linha):
    """
    Retorna:
        ("FIM",)                                  -> fim da sequencia
        ("ACAO",)                                 -> desarmar a bomba (bloco ACAO)
        ("MOVER", delta_col, delta_row, passos)   -> basicamente um vetor com direção eixo x, eixo y e tamanho do vetor ()
    """
    linha = linha.strip()

    if linha == "":
        return None

    if linha == "FIM":
        return ("FIM",)

    partes = linha.split()
    if len(partes) != 2:
        print(f"formato deve ser <NUMERO> <DIRECAO>")
        return None

    quantidade_texto, direcao = partes

    try:
        passos = int(quantidade_texto)

    except ValueError:
        print(f"Quantidade de passos invalido, instrucao ignorada")
        return None

    if direcao == "ACAO":
        return ("ACAO",)

    if direcao not in MOVIMENTO_POR_DIRECAO:
        print(f"Instrucao desconhecida, foi ignorada")
        return None

    delta_col, delta_row = MOVIMENTO_POR_DIRECAO[direcao]
    return ("MOVER", delta_col, delta_row, passos)


def ler_instrucoes():
    global _buffer
    instrucoes = []

    if _conexao is None:
        return instrucoes

    quantidade = _conexao.in_waiting
    if quantidade == 0:
        return instrucoes

    _buffer += _conexao.read(quantidade).decode("utf-8", errors="ignore")

    while "\n" in _buffer:
        linha, _buffer = _buffer.split("\n", 1)
        instrucao = interpretar(linha)
        if instrucao is not None:
            instrucoes.append(instrucao)

    return instrucoes

def enviar(texto):
    if _conexao is None:
        print("START nao enviado: Arduino nao conectado.")
        return False

    try:
        mensagem = texto.strip() + "\n"

        _conexao.write(mensagem.encode("UTF-8"))
        _conexao.flush()

        print(f"Enviado ao Arduino: {mensagem!r}")
        return True

    except SerialException as erro:
        print(f"Erro ao enviar para o Arduino: {erro}")
        return False

def fechar():
    global _conexao
    if _conexao is not None:
        _conexao.close()
        _conexao = None
