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

#if HOTENDS > 1

#define CODE_M218

/**
 * M218 - Set hotend offset (in linear units)
 *
 *   T<tools>
 *   X<xoffset>
 *   Y<yoffset>
 *   Z<zoffset>
 */
inline void gcode_M218(void) {

  if (!commands.get_target_tool(218)) return;

  #if DISABLED(DISABLE_M503)
    // No arguments? Show M218 report.
    if (parser.seen_any()) {
      nozzle.print_M218(TARGET_HOTEND);
      return;
    }
  #endif

  if (parser.seenval('X')) nozzle.data.hotend_offset[X_AXIS][TARGET_HOTEND] = parser.value_linear_units();
  if (parser.seenval('Y')) nozzle.data.hotend_offset[Y_AXIS][TARGET_HOTEND] = parser.value_linear_units();
  if (parser.seenval('Z')) nozzle.data.hotend_offset[Z_AXIS][TARGET_HOTEND] = parser.value_linear_units();

}

#endif // HOTENDS > 1
