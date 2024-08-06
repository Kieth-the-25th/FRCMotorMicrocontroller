import pyfirmata

board : pyfirmata.Arduino

def motorsInit(arduino: pyfirmata.Arduino):
    global board
    board = arduino

class CTREMagEncoder:
    port : int
    loop : int

    def __init__(self, port) -> None:
        self.port = port
        board.digital[port].set_mode(pyfirmata.INPUT)

    def get_rotations(self) -> int:
        return self.loop