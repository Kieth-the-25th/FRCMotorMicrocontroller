import motors
import time
import numpy

class Program:
    sparkMax : motors.PWMSparkMax

    def __init__(self) -> None:
        pass

    def start(self):
        self.sparkMax = motors.PWMSparkMax(3)

    def loop(self):
        self.sparkMax.set(numpy.random.random())