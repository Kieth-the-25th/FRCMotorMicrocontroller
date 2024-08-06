import motors;
import time;
import numpy;

class Program:
    sparkMax : motors.PWMSparkMax
    step = -0

    def __init__(self) -> None:
        pass

    def start(self):
        self.sparkMax = motors.PWMSparkMax(3)
        self.sparkMax.set(0)

    def loop(self, input):
        # set motor to 15% power
        if (self.step > -1):
            self.step -= 0.1
        
        self.sparkMax.set(input)

    def end(self):
        self.sparkMax.set(0)
        time.sleep(0.3)