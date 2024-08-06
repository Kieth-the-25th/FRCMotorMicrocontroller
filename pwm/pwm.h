#include <Arduino.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define TIMER_PRESCALER 1024
#define usToTicks(_us)    (( clockCyclesPerMicrosecond()* _us) / TIMER_PRESCALER)     // converts microseconds to ticks (assumes prescaler of 8)  // 12 Aug 2009
#define ticksToUs(_ticks) (( (unsigned)_ticks * TIMER_PRESCALER)/ clockCyclesPerMicrosecond() ) // converts from ticks back to microseconds

#define TIMER_REFRESH_RATE 15000

typedef enum { _timer5, _timer1, _timer3, _timer4, _Nbr_16timers } timer16_Sequence_t;

void setup_pwm()
{
  cli();//stop interrupts

  //https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1 = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = usToTicks(TIMER_REFRESH_RATE);// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS10) | (1 << CS12);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

const uint8_t outputs_per_timer = 4;
uint32_t refresh = usToTicks(TIMER_REFRESH_RATE);
uint8_t pwm_output_count = 0;
uint32_t pwm_outputs[outputs_per_timer];
uint32_t pwm_widths[outputs_per_timer];
int index = -1;

uint8_t add_port(int port, int delay) {
  pwm_outputs[pwm_output_count] = port;
  pwm_widths[pwm_output_count] = usToTicks(delay);
  pinMode(port, OUTPUT);
  return pwm_output_count++;
}

void set_delay(int newDelay) {
  pwm_widths[pwm_output_count] = newDelay;
}

void handle_interrupt(int timer, volatile uint16_t* TCNTn, volatile uint16_t* OCRnA) {
  uint16_t port = pwm_outputs[timer][index];

  if (index == -1) {
    *TCNTn = 0;
    *OCRnA = pwm_widths[timer][0];
  }
  else {
    digitalWrite(pwm_outputs[timer][index], LOW);
  }

  indexes[timer]++;
  if (indexes[timer] < pwm_output_count) {
    digitalWrite(pwm_outputs[timer][indexes[timer]], HIGH);
    *OCRnA = *TCNTn + pwm_widths[timer][indexes[timer]];
  }
  else {
    if (*TCNTn < refresh) {
      *OCRnA = refresh;
    }
    else {
      *OCRnA = *TCNTn + 4;
    }
    indexes[timer] = -1;
  }
}

void set_rate() {

}

SIGNAL(TIMER1_COMPA_vect) {
  handle_interrupt(0, &TCNT1, &OCR1A);
}