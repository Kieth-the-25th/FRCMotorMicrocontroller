/*
 * Firmata is a generic protocol for communicating with microcontrollers
 * from software on a host computer. It is intended to work with
 * any host computer software package.
 *
 * To download a host software package, please click on the following link
 * to open the list of Firmata client libraries in your default browser.
 *
 * https://github.com/firmata/arduino#firmata-client-libraries
 */

 /* This firmware supports as many servos as possible using the Servo library
  * included in Arduino 0017
  *
  * This example code is in the public domain.
  */

#include <Servo.h>
#include <Firmata.h>
#include <Arduino.h>
#include <avr/interrupt.h>

Servo servos[MAX_SERVOS];
byte servoPinMap[TOTAL_PINS];
byte servoCount = 0;

void analogWriteCallback(byte pin, int value)
{
  if (IS_PIN_DIGITAL(pin)) {
    servos[servoPinMap[pin]].write(value);
  }
  if (IS_PIN_PWM(pin)) {
    servos[servoPinMap[pin]].writeMicroseconds(value);
  }
}

void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
    // create a mask of the pins on this port that are writable.
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      // do not disturb non-digital pins (eg, Rx & Tx)
      if (IS_PIN_DIGITAL(pin)) {
        // do not touch pins in PWM, ANALOG, SERVO or other modes
        if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (Firmata.getPinMode(pin) == OUTPUT) {
            pinWriteMask |= mask;
          }
          else if (Firmata.getPinMode(pin) == INPUT && pinValue == 1 && Firmata.getPinState(pin) != 1) {
            // only handle INPUT here for backwards compatibility
#if ARDUINO > 100
            pinMode(pin, INPUT_PULLUP);
#else
            // only write to the INPUT pin to enable pullups if Arduino v1.0.0 or earlier
            pinWriteMask |= mask;
#endif
          }
          Firmata.setPinState(pin, pinValue);
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
  }
}

void sysexCallback(byte command, byte argc, byte* argv)
{
  byte mode;
  byte stopTX;
  byte slaveAddress;
  byte data;
  int slaveRegister;
  unsigned int delayTime;

  switch (command) {
  case EXTENDED_ANALOG:
    if (argc > 1) {
      int val = argv[1];
      if (argc > 2) val |= (argv[2] << 7);
      if (argc > 3) val |= (argv[3] << 14);
      analogWriteCallback(argv[0], val);
    }
    break;
  case CAPABILITY_QUERY:
    Firmata.write(START_SYSEX);
    Firmata.write(CAPABILITY_RESPONSE);
    for (byte pin = 0; pin < TOTAL_PINS; pin++) {
      if (IS_PIN_DIGITAL(pin)) {
        Firmata.write((byte)INPUT);
        Firmata.write(1);
        Firmata.write((byte)PIN_MODE_PULLUP);
        Firmata.write(1);
        Firmata.write((byte)OUTPUT);
        Firmata.write(1);
      }
      if (IS_PIN_ANALOG(pin)) {
        Firmata.write(PIN_MODE_ANALOG);
        Firmata.write(10); // 10 = 10-bit resolution
      }
      if (IS_PIN_PWM(pin)) {
        Firmata.write(PIN_MODE_PWM);
        Firmata.write(DEFAULT_PWM_RESOLUTION);
      }
      if (IS_PIN_DIGITAL(pin)) {
        Firmata.write(PIN_MODE_SERVO);
        Firmata.write(14);
      }
      if (IS_PIN_I2C(pin)) {
        Firmata.write(PIN_MODE_I2C);
        Firmata.write(1);  // TODO: could assign a number to map to SCL or SDA
      }
#ifdef FIRMATA_SERIAL_FEATURE
      serialFeature.handleCapability(pin);
#endif
      Firmata.write(127);
    }
    Firmata.write(END_SYSEX);
    break;
  case PIN_STATE_QUERY:
    if (argc > 0) {
      byte pin = argv[0];
      Firmata.write(START_SYSEX);
      Firmata.write(PIN_STATE_RESPONSE);
      Firmata.write(pin);
      if (pin < TOTAL_PINS) {
        Firmata.write(Firmata.getPinMode(pin));
        Firmata.write((byte)Firmata.getPinState(pin) & 0x7F);
        if (Firmata.getPinState(pin) & 0xFF80) Firmata.write((byte)(Firmata.getPinState(pin) >> 7) & 0x7F);
        if (Firmata.getPinState(pin) & 0xC000) Firmata.write((byte)(Firmata.getPinState(pin) >> 14) & 0x7F);
      }
      Firmata.write(END_SYSEX);
    }
    break;
  case ANALOG_MAPPING_QUERY:
    Firmata.write(START_SYSEX);
    Firmata.write(ANALOG_MAPPING_RESPONSE);
    for (byte pin = 0; pin < TOTAL_PINS; pin++) {
      Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
    }
    Firmata.write(END_SYSEX);
    break;
  case SERVO_CONFIG:
    if (argc > 3) {
      // these vars are here for clarity, they'll optimized away by the compiler
      byte pin = argv[0];
      byte active = argv[1];
      int pulseWidth = argv[2] + (argv[3] << 7);
    }
    break;
  case SERIAL_MESSAGE:
#ifdef FIRMATA_SERIAL_FEATURE
    serialFeature.handleSysex(command, argc, argv);
#endif
    break;
  }
}


void systemResetCallback()
{
  servoCount = 0;
}

void setup()
{
  byte pin; 
  
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  Firmata.attach(SYSTEM_RESET, systemResetCallback);
  Firmata.attach(START_SYSEX, sysexCallback);

  Firmata.begin(57600);
  systemResetCallback();

  // attach servos from first digital pin up to max number of
  // servos supported for the board
  for (pin = 0; pin < TOTAL_PINS; pin++) {
    if (IS_PIN_DIGITAL(pin)) {
      if (servoCount < MAX_SERVOS) {
        servoPinMap[pin] = servoCount;
        servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin));
        servoCount++;
      }
    }
  }
}

void loop()
{
  while (Firmata.available())
    Firmata.processInput();
}