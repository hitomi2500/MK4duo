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
 * mcode
 *
 * Copyright (c) 2019 Alberto Cotronei @MagoKimbra
 */

#define CODE_M78

/**
 * M78: Show print statistics
 *  X1 - Force save statistics
 */
inline void gcode_M78(void) {

  if (parser.seenval('X')) {
    SERIAL_EM("Statistics stored");
    print_job_counter.saveStats();
    return;
  }

  // "M78 S78" will reset the statistics
  if (parser.intval('S') == 78) {
    print_job_counter.initStats();
    lcdui.reset_status();
    return;
  }

  #if HAS_SERVICE_TIMES
    if (parser.seenval('R')) {
      print_job_counter.resetServiceTime(parser.value_int());
      lcdui.reset_status();
      return;
    }
  #endif

  print_job_counter.showStats();
}
