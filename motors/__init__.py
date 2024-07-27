import pyfirmata;
import numpy;

board : pyfirmata.Arduino

def motorsInit(arduino: pyfirmata.Arduino):
    global board
    board = arduino

class PWMTalonSRX:
    TALON_SRX_MIN_PULSE = 1050
    TALON_SRX_ZERO_PULSE = 1500
    TALON_SRX_HALF_PULSE = 500
    TALON_SRX_MAX_PULSE = 1950
    port : int = 0

    def __init__(self, port: int):
        self.port = port
        board.digital[port]._set_mode(pyfirmata.PWM)

    def set(self, value: float):
        output = self.TALON_SRX_ZERO_PULSE + numpy.clip(self.TALON_SRX_HALF_PULSE * value, self.TALON_SRX_MIN_PULSE, self.TALON_SRX_MAX_PULSE)
        board.digital[self.port].write(output)

    def get(self):
        return board.digital[self.port].read() / self.TALON_SRX_HALF_PULSE
    
class PWMSparkMax:
    SPARK_MAX_MIN_PULSE = 1000
    SPARK_MAX_ZERO_PULSE = 1500
    SPARK_MAX_HALF_PULSE = 500
    SPARK_MAX_MAX_PULSE = 2000
    port : int = 0

    def __init__(self, port: int):
        self.port = port
        board.digital[port]._set_mode(pyfirmata.OUTPUT)

    def set(self, value: float):
        output = int(numpy.clip(self.SPARK_MAX_ZERO_PULSE + (self.SPARK_MAX_HALF_PULSE * value), self.SPARK_MAX_MIN_PULSE, self.SPARK_MAX_MAX_PULSE))
        array = bytearray([self.port, output & 127, (output >> 7) & 127, (output >> 14) & 127])
        print(output)
        board.send_sysex(pyfirmata.EXTENDED_ANALOG, array)

    def get(self):
        return board.digital[self.port].read() / self.SPARK_MAX_HALF_PULSE