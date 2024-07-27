import pyfirmata;
import time;
import serial
from motors import *;
from program import Program;
from pynput.keyboard import Key, Listener

stop = False

def on_press(key):
    if key == Key.esc:
        print("Stopping...")
        stop = True

def on_release(key):
    print('{0} release'.format(
        key))
    if key == Key.esc:
        # Stop listener
        return False

# Collect events until released
with Listener(
        on_press=on_press,
        on_release=on_release) as listener:
    listener.join()

def printer(*args, **kwargs):
    print(str(bytes(args), "utf-8"))

board = pyfirmata.Arduino('COM13')
it = pyfirmata.util.Iterator(board)
it.start()
print("Connection successful")
motorsInit(board)

board.add_cmd_handler(pyfirmata.STRING_DATA, printer)

run = Program()
run.start()

while (listener.is_alive()):
    run.loop()
    time.sleep(0.5)
