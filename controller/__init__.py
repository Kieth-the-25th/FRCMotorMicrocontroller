import hid
import threading

class Controller:
    source = hid.device()
    found = False
    lx = 0
    ly = 0
    B = False

    def __init__(self) -> None:
        for device in hid.enumerate():
            # if device is generic desktop usage 4: "Joystick"
            if (device["usage_page"]) == 1 and device["usage"] == 4:
                print("Found joystick")
                self.source.open(device["vendor_id"], device["product_id"])
                self.source.set_nonblocking(True)
                self.found = True

    def read(self) -> any:
        if (self.found):
            bytes = self.source.read(64)
            if (len(bytes) > 0):
                self.lx = (float(bytes[0]) - 127) / 128
                self.ly = (float(bytes[1]) - 127) / 128
                self.B = bytes[4] & 0b01000000
                return bytes
    
    def B(self) -> bool:
        return self.B

    