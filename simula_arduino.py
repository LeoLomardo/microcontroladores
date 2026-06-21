"""Arduino falso para testar o controle serial sem o hardware.

Envia comandos (ex.: "2 ESQUERDA") por uma porta serial, um por vez, com
cerca de 1 segundo de intervalo, e termina com "Fim" - igual ao Arduino real
do Projeto 09. Usa o mesmo write(texto.encode("UTF-8")) dos slides.

Como testar o jogo inteiro (sem Arduino), usando portas virtuais do socat:

    # terminal 1 - cria duas portas virtuais ligadas uma na outra:
    socat -d -d pty,raw,echo=0,link=/tmp/ttyGAME pty,raw,echo=0,link=/tmp/ttyARDUINO

    # terminal 2 - roda o jogo apontando para a ponta /tmp/ttyGAME:
    PORTA_SERIAL=/tmp/ttyGAME .venv/bin/python main.py

    # terminal 3 - este script manda os comandos pela outra ponta:
    .venv/bin/python simula_arduino.py /tmp/ttyARDUINO
"""

import sys
import time
from serial import Serial

PORTA_PADRAO = "/tmp/ttyARDUINO"
INTERVALO_S = 1.0   # ~1 segundo entre comandos, como o Arduino real

# Sequencia de teste - edite a vontade (direcoes: CIMA, ABAIXO, DIREITA, ESQUERDA).
COMANDOS = [
    "6 CIMA",
    "4 DIREITA",
    "4 ABAIXO",
    "4 DIREITA",
    "5 CIMA",
    "FIM"
]


def main():
    porta = sys.argv[1] if len(sys.argv) > 1 else PORTA_PADRAO

    with Serial(porta, baudrate=9600) as arduino:
        print(f"Arduino falso conectado em {porta}. Enviando comandos...")
        for comando in COMANDOS:
            arduino.write((comando + "\n").encode("UTF-8"))
            print(f"  enviado: {comando!r}")
            time.sleep(INTERVALO_S)
        print("Pronto.")


if __name__ == "__main__":
    main()
