import pyfirmata;
import time;
from motors import *;
from program import Program;
from pynput.keyboard import Key, Listener
from threading import Thread, Lock, Condition
import controller
import hid

joystick = controller.Controller()

for device in hid.enumerate():
    for key in list(device.keys()):
        print(key, " ", device[key])

keyInput: Condition = Condition()
value = 0
loopValue = 0

def setValue(v):
    global value
    global keyInput
    if keyInput.acquire(True):
        value = v
        keyInput.notify_all()
        keyInput.release()


def on_press(key):
    global keyInput
    global value
    if key == Key.up:
        setValue(1)
    if key == Key.down:
        setValue(-1)
    if key == Key.esc:
        print("Stopping...")
        run.end()
        return False

def on_release(key):
    global keyInput
    global value
    print('{0} release'.format(
        key))
    if key == Key.up:
        setValue(0)
    if key == Key.down:
        setValue(0)
    if key == Key.esc:
        # Stop listener
        return False

# Collect events until released
listener = Listener(
        on_press=on_press,
        on_release=on_release)
listener.start()

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
time.sleep(1)

while (listener.is_alive()):
    if keyInput.acquire(True, 0.25):
        loopValue = value
        keyInput.notify_all()
        keyInput.release()
    joystick.read()
    if (joystick.B):
        run.loop(joystick.ly)
    else:
        run.loop(0)
