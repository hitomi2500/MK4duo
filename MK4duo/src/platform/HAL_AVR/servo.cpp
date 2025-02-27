/**
 * MK4duo Firmware for 3D Printer, Laser and CNC
 *
 * Based on Marlin, Sprinter and grbl
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 * Copyright (c) 2019 Alberto Cotronei @MagoKimbra
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef __AVR__

#include "../../../MK4duo.h"

#if HAS_SERVOS

#include "../common/servo.h"

static volatile int8_t Channel[_Nbr_16timers]; // counter for the servo being pulsed for each timer (or -1 if refresh interval)

/************ static functions common to all instances ***********************/

static inline void handle_interrupts(timer16_Sequence_t timer, volatile uint16_t *TCNTn, volatile uint16_t* OCRnA) {
  if (Channel[timer] < 0)
    *TCNTn = 0; // channel set to -1 indicated that refresh interval completed so reset the timer
  else if (SERVO_INDEX(timer, Channel[timer]) < ServoCount && SERVO(timer,Channel[timer]).Pin.isActive == true)
    digitalWrite(SERVO(timer, Channel[timer]).Pin.nbr, LOW); // pulse this channel low if activated

  Channel[timer]++;    // increment to the next channel
  if (SERVO_INDEX(timer, Channel[timer]) < ServoCount && Channel[timer] < SERVOS_PER_TIMER) {
    *OCRnA = *TCNTn + SERVO(timer, Channel[timer]).ticks;
    if (SERVO(timer, Channel[timer]).Pin.isActive)     // check if activated
      digitalWrite(SERVO(timer, Channel[timer]).Pin.nbr, HIGH); // its an active channel so pulse it high
  }
  else {
    // finished all channels so wait for the refresh period to expire before starting over
    if (((unsigned)*TCNTn) + 4 < usToTicks(REFRESH_INTERVAL))  // allow a few ticks to ensure the next OCR1A not missed
      *OCRnA = (unsigned int)usToTicks(REFRESH_INTERVAL);
    else
      *OCRnA = *TCNTn + 4;  // at least REFRESH_INTERVAL has elapsed

    Channel[timer] = -1; // this will get incremented at the end of the refresh period to start again at the first channel
  }
}

#ifndef WIRING // Wiring pre-defines signal handlers so don't define any if compiling for the Wiring platform

  // Interrupt handlers for Arduino
  #ifdef _useTimer1
    SIGNAL (TIMER1_COMPA_vect) { handle_interrupts(_timer1, &TCNT1, &OCR1A); }
  #endif

  #ifdef _useTimer3
    SIGNAL (TIMER3_COMPA_vect) { handle_interrupts(_timer3, &TCNT3, &OCR3A); }
  #endif

  #ifdef _useTimer4
    SIGNAL (TIMER4_COMPA_vect) { handle_interrupts(_timer4, &TCNT4, &OCR4A); }
  #endif

  #ifdef _useTimer5
    SIGNAL (TIMER5_COMPA_vect) { handle_interrupts(_timer5, &TCNT5, &OCR5A); }
  #endif

#else // !WIRING

  // Interrupt handlers for Wiring
  #ifdef _useTimer1
    void Timer1Service() { handle_interrupts(_timer1, &TCNT1, &OCR1A); }
  #endif
  #ifdef _useTimer3
    void Timer3Service() { handle_interrupts(_timer3, &TCNT3, &OCR3A); }
  #endif

#endif // !WIRING

/****************** end of static functions ******************************/

void initISR(timer16_Sequence_t timer) {
  #ifdef _useTimer1
    if (timer == _timer1) {
      TCCR1A = 0;             // normal counting mode
      TCCR1B = _BV(CS11);     // set prescaler of 8
      TCNT1 = 0;              // clear the timer count
    #if defined(__AVR_ATmega8__)|| defined(__AVR_ATmega128__)
      SBI(TIFR, OCF1A);      // clear any pending interrupts;
      SBI(TIMSK, OCIE1A);    // enable the output compare interrupt
    #else
      // here if not ATmega8 or ATmega128
      SBI(TIFR1, OCF1A);     // clear any pending interrupts;
      SBI(TIMSK1, OCIE1A);   // enable the output compare interrupt
    #endif
    #ifdef WIRING
      timerAttach(TIMER1OUTCOMPAREA_INT, Timer1Service);
    #endif
    }
  #endif

  #ifdef _useTimer3
    if (timer == _timer3) {
      TCCR3A = 0;             // normal counting mode
      TCCR3B = _BV(CS31);     // set prescaler of 8
      TCNT3 = 0;              // clear the timer count
      #ifdef __AVR_ATmega128__
        SBI(TIFR, OCF3A);     // clear any pending interrupts;
        SBI(ETIMSK, OCIE3A);  // enable the output compare interrupt
      #else
        SBI(TIFR3, OCF3A);   // clear any pending interrupts;
        SBI(TIMSK3, OCIE3A); // enable the output compare interrupt
      #endif
      #ifdef WIRING
        timerAttach(TIMER3OUTCOMPAREA_INT, Timer3Service);  // for Wiring platform only
      #endif
    }
  #endif

  #ifdef _useTimer4
    if (timer == _timer4) {
      TCCR4A = 0;             // normal counting mode
      TCCR4B = _BV(CS41);     // set prescaler of 8
      TCNT4 = 0;              // clear the timer count
      TIFR4 = _BV(OCF4A);     // clear any pending interrupts;
      TIMSK4 = _BV(OCIE4A);   // enable the output compare interrupt
    }
  #endif

  #ifdef _useTimer5
    if (timer == _timer5) {
      TCCR5A = 0;             // normal counting mode
      TCCR5B = _BV(CS51);     // set prescaler of 8
      TCNT5 = 0;              // clear the timer count
      TIFR5 = _BV(OCF5A);     // clear any pending interrupts;
      TIMSK5 = _BV(OCIE5A);   // enable the output compare interrupt
    }
  #endif
}

void finISR(timer16_Sequence_t timer) {
  //disable use of the given timer
  #ifdef WIRING
    if (timer == _timer1) {
      #if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
        TIMSK1 &= ~_BV(OCIE1A);  // disable timer 1 output compare interrupt
      #else
        TIMSK  &= ~_BV(OCIE1A);  // disable timer 1 output compare interrupt
      #endif
      timerDetach(TIMER1OUTCOMPAREA_INT);
    }
    else if (timer == _timer3) {
      #if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
        TIMSK3 &= ~_BV(OCIE3A);    // disable the timer3 output compare A interrupt
      #else
        ETIMSK &= ~_BV(OCIE3A);    // disable the timer3 output compare A interrupt
      #endif
      timerDetach(TIMER3OUTCOMPAREA_INT);
    }
  #else
    //For arduino - in future: call here to a currently undefined function to reset the timer
    UNUSED(timer);
  #endif
}

#endif // HAS_SERVOS

#endif // __AVR__
