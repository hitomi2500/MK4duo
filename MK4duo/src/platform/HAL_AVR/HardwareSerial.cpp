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

/**
 * HardwareSerial.cpp - Hardware serial library for Wiring
 * Copyright (c) 2006 Nicholas Zambetti. All right reserved.
 *
 * Modified 28 September  2010 by Mark Sproul
 * Modified  3 March      2015 by MagoKimbra
 * Modified 14 February   2016 by Andreas Hardtung (added tx buffer)
 * Modified 01 October    2017 by Eduardo José Tagle (added XON/XOFF)
 * Templatized 01 October 2018 by Eduardo José Tagle to allow multiple instances
 */

#ifdef __AVR__

#include "../../../MK4duo.h"

// Disable HardwareSerial.cpp to support chips without a UART (Attiny, etc.)
#if ENABLED(UBRRH) || ENABLED(UBRR0H) || ENABLED(UBRR1H) || ENABLED(UBRR2H) || ENABLED(UBRR3H)

#include "HardwareSerial.h"

// A SW memory barrier, to ensure GCC does not overoptimize loops
#define sw_barrier() asm volatile("": : :"memory");

/** Protected Parameters */
template<typename Cfg> typename MKHardwareSerial<Cfg>::ring_buffer_r MKHardwareSerial<Cfg>::rx_buffer = { 0, 0, { 0 } };
template<typename Cfg> typename MKHardwareSerial<Cfg>::ring_buffer_t MKHardwareSerial<Cfg>::tx_buffer = { 0 };
template<typename Cfg> bool     MKHardwareSerial<Cfg>::_written = false;
template<typename Cfg> uint8_t  MKHardwareSerial<Cfg>::xon_xoff_state = MKHardwareSerial<Cfg>::XON_XOFF_CHAR_SENT | MKHardwareSerial<Cfg>::XON_CHAR;
template<typename Cfg> uint8_t  MKHardwareSerial<Cfg>::rx_dropped_bytes = 0;
template<typename Cfg> uint8_t  MKHardwareSerial<Cfg>::rx_buffer_overruns = 0;
template<typename Cfg> uint8_t  MKHardwareSerial<Cfg>::rx_framing_errors = 0;
template<typename Cfg> typename MKHardwareSerial<Cfg>::ring_buffer_pos_t MKHardwareSerial<Cfg>::rx_max_enqueued = 0;
template<typename Cfg> volatile bool MKHardwareSerial<Cfg>::rx_tail_value_not_stable = false;
template<typename Cfg> volatile uint16_t MKHardwareSerial<Cfg>::rx_tail_value_backup = 0;

/** Protected Function */
// "Atomically" read the RX head index value without disabling interrupts:
// This MUST be called with RX interrupts enabled, and CAN'T be called
// from the RX ISR itself!
template<typename Cfg>
FORCE_INLINE typename MKHardwareSerial<Cfg>::ring_buffer_pos_t MKHardwareSerial<Cfg>::atomic_read_rx_head() {
  if (Cfg::RX_SIZE > 256) {
    // Keep reading until 2 consecutive reads return the same value,
    // meaning there was no update in-between caused by an interrupt.
    // This works because serial RX interrupts happen at a slower rate
    // than successive reads of a variable, so 2 consecutive reads with
    // the same value means no interrupt updated it.
    ring_buffer_pos_t vold, vnew = rx_buffer.head;
    sw_barrier();
    do {
      vold = vnew;
      vnew = rx_buffer.head;
      sw_barrier();
    } while (vold != vnew);
    return vnew;
  }
  else {
    // With an 8bit index, reads are always atomic. No need for special handling
    return rx_buffer.head;
  }
}

// Set RX tail index, taking into account the RX ISR could interrupt
//  the write to this variable in the middle - So a backup strategy
//  is used to ensure reads of the correct values.
//    -Must NOT be called from the RX ISR -
template<typename Cfg>
FORCE_INLINE void MKHardwareSerial<Cfg>::atomic_set_rx_tail(typename MKHardwareSerial<Cfg>::ring_buffer_pos_t value) {
  if (Cfg::RX_SIZE > 256) {
    // Store the new value in the backup
    rx_tail_value_backup = value;
    sw_barrier();
    // Flag we are about to change the true value
    rx_tail_value_not_stable = true;
    sw_barrier();
    // Store the new value
    rx_buffer.tail = value;
    sw_barrier();
    // Signal the new value is completely stored into the value
    rx_tail_value_not_stable = false;
    sw_barrier();
  }
  else
    rx_buffer.tail = value;
}

// Get the RX tail index, taking into account the read could be
//  interrupting in the middle of the update of that index value
//    -Called from the RX ISR -
template<typename Cfg>
FORCE_INLINE typename MKHardwareSerial<Cfg>::ring_buffer_pos_t MKHardwareSerial<Cfg>::atomic_read_rx_tail() {
  if (Cfg::RX_SIZE > 256) {
    // If the true index is being modified, return the backup value
    if (rx_tail_value_not_stable) return rx_tail_value_backup;
  }
  // The true index is stable, return it
  return rx_buffer.tail;
}

/** Public Function */
template<typename Cfg>
void MKHardwareSerial<Cfg>::begin(const long baud) {
  uint16_t baud_setting;
  bool useU2X = true;

  #if F_CPU == 16000000UL && SERIAL_PORT_1 == 0
    // Hard-coded exception for compatibility with the bootloader shipped
    // with the Duemilanove and previous boards, and the firmware on the
    // 8U2 on the Uno and Mega 2560.
    if (baud == 57600) useU2X = false;
  #endif

  R_UCSRA = 0;
  if (useU2X) {
    B_U2X = 1;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  }
  else
    baud_setting = (F_CPU / 8 / baud - 1) / 2;

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  R_UBRRH = baud_setting >> 8;
  R_UBRRL = baud_setting;

  B_RXEN = 1;
  B_TXEN = 1;
  B_RXCIE = 1;
  if (Cfg::TX_SIZE > 0) B_UDRIE = 0;
  _written = false;
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::end() {
  B_RXEN = 0;
  B_TXEN = 0;
  B_RXCIE = 0;
  B_UDRIE = 0;
}

template<typename Cfg>
int MKHardwareSerial<Cfg>::peek(void) {
  const ring_buffer_pos_t h = atomic_read_rx_head(), t = rx_buffer.tail;
  return h == t ? -1 : rx_buffer.buffer[t];
}

template<typename Cfg>
int MKHardwareSerial<Cfg>::read(void) {
  const ring_buffer_pos_t h = atomic_read_rx_head();

  // Read the tail. Main thread owns it, so it is safe to directly read it
  ring_buffer_pos_t t = rx_buffer.tail;

  // If nothing to read, return now
  if (h == t) return -1;

  // Get the next char
  const int v = rx_buffer.buffer[t];
  t = (ring_buffer_pos_t)(t + 1) & (Cfg::RX_SIZE - 1);

  // Advance tail - Making sure the RX ISR will always get an stable value, even
  // if it interrupts the writing of the value of that variable in the middle.
  atomic_set_rx_tail(t);

  if (Cfg::XONOFF) {
    // If the XOFF char was sent, or about to be sent...
    if ((xon_xoff_state & XON_XOFF_CHAR_MASK) == XOFF_CHAR) {
      // Get count of bytes in the RX buffer
      const ring_buffer_pos_t rx_count = (ring_buffer_pos_t)(h - t) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);
      if (rx_count < (Cfg::RX_SIZE) / 10) {
        if (Cfg::TX_SIZE > 0) {
          // Signal we want an XON character to be sent.
          xon_xoff_state = XON_CHAR;
          // Enable TX ISR. Non atomic, but it will eventually enable them
          B_UDRIE = 1;
        }
        else {
          // If not using TX interrupts, we must send the XON char now
          xon_xoff_state = XON_CHAR | XON_XOFF_CHAR_SENT;
          while (!B_UDRE) sw_barrier();
          R_UDR = XON_CHAR;
        }
      }
    }
  }

  return v;
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::flush(void) {

  // Set the tail to the head:
  //  - Read the RX head index in a safe way. (See atomic_read_rx_head.)
  //  - Set the tail, making sure the RX ISR will always get a stable value, even
  //    if it interrupts the writing of the value of that variable in the middle.
  atomic_set_rx_tail(atomic_read_rx_head());

  if (Cfg::XONOFF) {
    // If the XOFF char was sent, or about to be sent...
    if ((xon_xoff_state & XON_XOFF_CHAR_MASK) == XOFF_CHAR) {
      if (Cfg::TX_SIZE > 0) {
        // Signal we want an XON character to be sent.
        xon_xoff_state = XON_CHAR;
        // Enable TX ISR. Non atomic, but it will eventually enable it.
        B_UDRIE = 1;
      }
      else {
        // If not using TX interrupts, we must send the XON char now
        xon_xoff_state = XON_CHAR | XON_XOFF_CHAR_SENT;
        while (!B_UDRE) sw_barrier();
        R_UDR = XON_CHAR;
      }
    }
  }
}

template<typename Cfg>
typename MKHardwareSerial<Cfg>::ring_buffer_pos_t MKHardwareSerial<Cfg>::available(void) {
  const ring_buffer_pos_t h = atomic_read_rx_head(), t = rx_buffer.tail;
  return (ring_buffer_pos_t)(Cfg::RX_SIZE + h - t) & (Cfg::RX_SIZE - 1);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::write(const uint8_t c) {
  if (Cfg::TX_SIZE == 0) {

    _written = true;
    while (!B_UDRE) sw_barrier();
    R_UDR = c;

  }
  else {

    _written = true;

    // If the TX interrupts are disabled and the data register
    // is empty, just write the byte to the data register and
    // be done. This shortcut helps significantly improve the
    // effective datarate at high (>500kbit/s) bitrates, where
    // interrupt overhead becomes a slowdown.
    // Yes, there is a race condition between the sending of the
    // XOFF char at the RX ISR, but it is properly handled there
    if (!B_UDRIE && B_UDRE) {
      R_UDR = c;

      // clear the TXC bit -- "can be cleared by writing a one to its bit
      // location". This makes sure flush() won't return until the bytes
      // actually got written
      B_TXC = 1;
      return;
    }

    const uint8_t i = (tx_buffer.head + 1) & (Cfg::TX_SIZE - 1);

    // If global interrupts are disabled (as the result of being called from an ISR)...
    if (!ISRS_ENABLED()) {

      // Make room by polling if it is possible to transmit, and do so!
      while (i == tx_buffer.tail) {

        // If we can transmit another byte, do it.
        if (B_UDRE) _tx_udr_empty_irq();

        // Make sure compiler rereads tx_buffer.tail
        sw_barrier();
      }
    }
    else {
      // Interrupts are enabled, just wait until there is space
      while (i == tx_buffer.tail) sw_barrier();
    }

    // Store new char. head is always safe to move
    tx_buffer.buffer[tx_buffer.head] = c;
    tx_buffer.head = i;

    // Enable TX ISR - Non atomic, but it will eventually enable TX ISR
    B_UDRIE = 1;
  }
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::flushTX(void) {

  if (Cfg::TX_SIZE == 0) {
    // No bytes written, no need to flush. This special case is needed since there's
    // no way to force the TXC (transmit complete) bit to 1 during initialization.
    if (!_written) return;

    // Wait until everything was transmitted
    while (!B_TXC) sw_barrier();

    // At this point nothing is queued anymore (DRIE is disabled) and
    // the hardware finished transmission (TXC is set).

  }
  else {

    // No bytes written, no need to flush. This special case is needed since there's
    // no way to force the TXC (transmit complete) bit to 1 during initialization.
    if (!_written) return;

    // If global interrupts are disabled (as the result of being called from an ISR)...
    if (!ISRS_ENABLED()) {

      // Wait until everything was transmitted - We must do polling, as interrupts are disabled
      while (tx_buffer.head != tx_buffer.tail || !B_TXC) {

        // If there is more space, send an extra character
        if (B_UDRE) _tx_udr_empty_irq();

        sw_barrier();
      }

    }
    else {
      // Wait until everything was transmitted
      while (tx_buffer.head != tx_buffer.tail || !B_TXC) sw_barrier();
    }

    // At this point nothing is queued anymore (DRIE is disabled) and
    // the hardware finished transmission (TXC is set).
  }
}

template<typename Cfg>
size_t MKHardwareSerial<Cfg>::readBytes(char* buffer, size_t size) {

  int c;
  size_t count = 0;
  const millis_l timeout = millis() + 1000UL;

  while (count < size) {

    do {
      c = read();
      if (c >= 0) break;
    } while (PENDING(millis(), timeout));

    if (c < 0) break;
    *buffer++ = (char)c;
    count++;
  }

  return count;

}

// (called with RX interrupts disabled)
template<typename Cfg>
FORCE_INLINE void MKHardwareSerial<Cfg>::store_rxd_char() {

  static EmergencyStateEnum emergency_state; // = EP_RESET

  // Get the tail - Nothing can alter its value while this ISR is executing, but there's
  // a chance that this ISR interrupted the main process while it was updating the index.
  // The backup mechanism ensures the correct value is always returned.
  const ring_buffer_pos_t t = atomic_read_rx_tail();

  // Get the head pointer - This ISR is the only one that modifies its value, so it's safe to read here
  ring_buffer_pos_t h = rx_buffer.head;

  // Get the next element
  ring_buffer_pos_t i = (ring_buffer_pos_t)(h + 1) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);

  // This must read the R_UCSRA register before reading the received byte to detect error causes
  if (Cfg::DROPPED_RX && B_DOR && !++rx_dropped_bytes) --rx_dropped_bytes;
  if (Cfg::RX_OVERRUNS && B_DOR && !++rx_buffer_overruns) --rx_buffer_overruns;
  if (Cfg::RX_FRAMING_ERRORS && B_FE && !++rx_framing_errors) --rx_framing_errors;

  // Read the character from the USART
  uint8_t c = R_UDR;

  if (Cfg::EMERGENCYPARSER) emergency_parser.update(emergency_state, c);

  // If the character is to be stored at the index just before the tail
  // (such that the head would advance to the current tail), the RX FIFO is
  // full, so don't write the character or advance the head.
  if (i != t) {
    rx_buffer.buffer[h] = c;
    h = i;
  }
  else if (Cfg::DROPPED_RX && !++rx_dropped_bytes)
    --rx_dropped_bytes;

  if (Cfg::MAX_RX_QUEUED) {
    // Calculate count of bytes stored into the RX buffer
    const ring_buffer_pos_t rx_count = (ring_buffer_pos_t)(h - t) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);

    // Keep track of the maximum count of enqueued bytes
    NOLESS(rx_max_enqueued, rx_count);
  }

  if (Cfg::XONOFF) {
    // If the last char that was sent was an XON
    if ((xon_xoff_state & XON_XOFF_CHAR_MASK) == XON_CHAR) {

      // Bytes stored into the RX buffer
      const ring_buffer_pos_t rx_count = (ring_buffer_pos_t)(h - t) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);

      // If over 12.5% of RX buffer capacity, send XOFF before running out of
      // RX buffer space .. 325 bytes @ 250kbits/s needed to let the host react
      // and stop sending bytes. This translates to 13mS propagation time.
      if (rx_count >= (Cfg::RX_SIZE) / 8) {

        // At this point, definitely no TX interrupt was executing, since the TX ISR can't be preempted.
        // Don't enable the TX interrupt here as a means to trigger the XOFF char, because if it happens
        // to be in the middle of trying to disable the RX interrupt in the main program, eventually the
        // enabling of the TX interrupt could be undone. The ONLY reliable thing this can do to ensure
        // the sending of the XOFF char is to send it HERE AND NOW.

        // About to send the XOFF char
        xon_xoff_state = XOFF_CHAR | XON_XOFF_CHAR_SENT;

        // Wait until the TX register becomes empty and send it - Here there could be a problem
        // - While waiting for the TX register to empty, the RX register could receive a new
        //   character. This must also handle that situation!
        while (!B_UDRE) {

          if (B_RXC) {
            // A char arrived while waiting for the TX buffer to be empty - Receive and process it!

            i = (ring_buffer_pos_t)(h + 1) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);

            // Read the character from the USART
            c = R_UDR;

            if (Cfg::EMERGENCYPARSER) emergency_parser.update(emergency_state, c);

            // If the character is to be stored at the index just before the tail
            // (such that the head would advance to the current tail), the FIFO is
            // full, so don't write the character or advance the head.
            if (i != t) {
              rx_buffer.buffer[h] = c;
              h = i;
            }
            else if (Cfg::DROPPED_RX && !++rx_dropped_bytes)
              --rx_dropped_bytes;
          }
          sw_barrier();
        }

        R_UDR = XOFF_CHAR;

        // Clear the TXC bit -- "can be cleared by writing a one to its bit
        // location". This makes sure flush() won't return until the bytes
        // actually got written
        B_TXC = 1;

        // At this point there could be a race condition between the write() function
        // and this sending of the XOFF char. This interrupt could happen between the
        // wait to be empty TX buffer loop and the actual write of the character. Since
        // the TX buffer is full because it's sending the XOFF char, the only way to be
        // sure the write() function will succeed is to wait for the XOFF char to be
        // completely sent. Since an extra character could be received during the wait
        // it must also be handled!
        while (!B_UDRE) {

          if (B_RXC) {
            // A char arrived while waiting for the TX buffer to be empty - Receive and process it!

            i = (ring_buffer_pos_t)(h + 1) & (ring_buffer_pos_t)(Cfg::RX_SIZE - 1);

            // Read the character from the USART
            c = R_UDR;

            if (Cfg::EMERGENCYPARSER) emergency_parser.update(emergency_state, c);

            // If the character is to be stored at the index just before the tail
            // (such that the head would advance to the current tail), the FIFO is
            // full, so don't write the character or advance the head.
            if (i != t) {
              rx_buffer.buffer[h] = c;
              h = i;
            }
            else if (Cfg::DROPPED_RX && !++rx_dropped_bytes)
              --rx_dropped_bytes;
          }
          sw_barrier();
        }

        // At this point everything is ready. The write() function won't
        // have any issues writing to the UART TX register if it needs to!
      }
    }
  }

  // Store the new head value - The main loop will retry until the value is stable
  rx_buffer.head = h;
}

// (called with TX irqs disabled)
template<typename Cfg>
FORCE_INLINE void MKHardwareSerial<Cfg>::_tx_udr_empty_irq(void) {
  if (Cfg::TX_SIZE > 0) {
    // Read positions
    uint8_t t = tx_buffer.tail;
    const uint8_t h = tx_buffer.head;

    if (Cfg::XONOFF) {
      // If an XON char is pending to be sent, do it now
      if (xon_xoff_state == XON_CHAR) {

        // Send the character
        R_UDR = XON_CHAR;

        // clear the TXC bit -- "can be cleared by writing a one to its bit
        // location". This makes sure flush() won't return until the bytes
        // actually got written
        B_TXC = 1;

        // Remember we sent it.
        xon_xoff_state = XON_CHAR | XON_XOFF_CHAR_SENT;

        // If nothing else to transmit, just disable TX interrupts.
        if (h == t) B_UDRIE = 0; // (Non-atomic, could be reenabled by the main program, but eventually this will succeed)

        return;
      }
    }

    // If nothing to transmit, just disable TX interrupts. This could
    // happen as the result of the non atomicity of the disabling of RX
    // interrupts that could end reenabling TX interrupts as a side effect.
    if (h == t) {
      B_UDRIE = 0; // (Non-atomic, could be reenabled by the main program, but eventually this will succeed)
      return;
    }

    // There is something to TX, Send the next byte
    const uint8_t c = tx_buffer.buffer[t];
    t = (t + 1) & (Cfg::TX_SIZE - 1);
    R_UDR = c;
    tx_buffer.tail = t;

    // Clear the TXC bit (by writing a one to its bit location).
    // Ensures flush() won't return until the bytes are actually written/
    B_TXC = 1;

    // Disable interrupts if there is nothing to transmit following this byte
    if (h == t) B_UDRIE = 0; // (Non-atomic, could be reenabled by the main program, but eventually this will succeed)
  }
}

/**
 * Imports from print.h
 */
template<typename Cfg>
void MKHardwareSerial<Cfg>::print(char c, int base) {
  print((long)c, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(unsigned char b, int base) {
  print((unsigned long)b, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(int n, int base) {
  print((long)n, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(unsigned int n, int base) {
  print((unsigned long)n, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(long n, int base) {
  if (base == 0) write(n);
  else if (base == 10) {
    if (n < 0) { print('-'); n = -n; }
    printNumber(n, 10);
  }
  else
    printNumber(n, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(unsigned long n, int base) {
  if (base == 0) write(n);
  else printNumber(n, base);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::print(double n, int digits) {
  printFloat(n, digits);
}

template<typename Cfg>
void MKHardwareSerial<Cfg>::println(void) {
  print('\r');
  print('\n');
}

/** Private Function */
template<typename Cfg>
void MKHardwareSerial<Cfg>::printNumber(unsigned long n, uint8_t base) {

  if (n) {
    unsigned char buf[8 * sizeof(long)]; // Enough space for base 2
    int8_t i = 0;
    while (n) {
      buf[i++] = n % base;
      n /= base;
    }
    while (i--)
      print((char)(buf[i] + (buf[i] < 10 ? '0' : 'A' - 10)));
  }
  else
    print('0');

}

template<typename Cfg>
void MKHardwareSerial<Cfg>::printFloat(double number, uint8_t digits) {

  // Handle negative numbers
  if (number < 0.0) {
    print('-');
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i) rounding *= 0.1;
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits) {
    print('.');
    // Extract digits from the remainder one at a time
    while (digits--) {
      remainder *= 10.0;
      int toPrint = int(remainder);
      print(toPrint);
      remainder -= toPrint;
    }
  }

}

// Hookup ISR handlers
ISR(SERIAL_REGNAME(USART,SERIAL_PORT_1,_RX_vect)) {
  MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_1>>::store_rxd_char();
}

ISR(SERIAL_REGNAME(USART,SERIAL_PORT_1,_UDRE_vect)) {
  MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_1>>::_tx_udr_empty_irq();
}

// Preinstantiate
template class MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_1>>;

// Instantiate
MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_1>> MKSerial1;

#if SERIAL_PORT_2 >= -1

  // Hookup ISR handlers
  ISR(SERIAL_REGNAME(USART,SERIAL_PORT_2,_RX_vect)) {
    MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_2>>::store_rxd_char();
  }

  ISR(SERIAL_REGNAME(USART,SERIAL_PORT_2,_UDRE_vect)) {
    MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_2>>::_tx_udr_empty_irq();
  }

  // Preinstantiate
  template class MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_2>>;

  // Instantiate
  MKHardwareSerial<MK4duoSerialHostCfg<SERIAL_PORT_2>> MKSerial2;

#endif // SERIAL_PORT_2 >= -1

#if ENABLED(NEXTION) && NEXTION_SERIAL > 0

  // Hookup ISR handlers
  ISR(SERIAL_REGNAME(USART,NEXTION_SERIAL,_RX_vect)) {
    MKHardwareSerial<MK4duoSerialCfg<NEXTION_SERIAL>>::store_rxd_char();
  }

  ISR(SERIAL_REGNAME(USART,NEXTION_SERIAL,_UDRE_vect)) {
    MKHardwareSerial<MK4duoSerialCfg<NEXTION_SERIAL>>::_tx_udr_empty_irq();
  }

  // Preinstantiate
  template class MKHardwareSerial<MK4duoSerialCfg<NEXTION_SERIAL>>;

  // Instantiate
  MKHardwareSerial<MK4duoSerialCfg<NEXTION_SERIAL>> nexSerial;

#endif // ENABLED(NEXTION) && NEXTION_SERIAL > 0

#if HAS_MMU2 && MMU2_SERIAL > 0

  // Hookup ISR handlers
  ISR(SERIAL_REGNAME(USART,MMU2_SERIAL,_RX_vect)) {
    MKHardwareSerial<MK4duoSerialCfg<MMU2_SERIAL>>::store_rxd_char();
  }

  ISR(SERIAL_REGNAME(USART,MMU2_SERIAL,_UDRE_vect)) {
    MKHardwareSerial<MK4duoSerialCfg<MMU2_SERIAL>>::_tx_udr_empty_irq();
  }

  // Preinstantiate
  template class MKHardwareSerial<MK4duoSerialCfg<MMU2_SERIAL>>;

  // Instantiate
  MKHardwareSerial<MK4duoSerialCfg<MMU2_SERIAL>> mmuSerial;

#endif // HAS_MMU2 && MMU2_SERIAL > 0

#endif // (UBRRH || UBRR0H || UBRR1H || UBRR2H || UBRR3H)

#endif // __AVR__
