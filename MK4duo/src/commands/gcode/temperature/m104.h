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

#if HAS_TEMP_HOTEND

#define CODE_M104

/**
 * M104: Set hotend temperature
 */
inline void gcode_M104(void) {

  if (commands.get_target_tool(104)) return;

  if (printer.debugDryrun() || printer.debugSimulation()) return;

  if (parser.seenval('S')) {
    const int16_t temp = parser.value_celsius();
    #if ENABLED(SINGLENOZZLE)
      tools.singlenozzle_temp[TARGET_EXTRUDER] = temp;
      if (TARGET_EXTRUDER != tools.extruder.active) return;
    #endif
    hotends[TARGET_HOTEND].set_target_temp(temp);

    #if ENABLED(DUAL_X_CARRIAGE)
      if (mechanics.dxc_is_duplicating() && TARGET_EXTRUDER == 0)
        hotends[1].set_target_temp(temp ? temp + mechanics.duplicate_extruder_temp_offset : 0);
    #endif
  }

  if (parser.seenval('R')) hotends[TARGET_HOTEND].setIdle(false, parser.value_celsius());

  #if ENABLED(AUTOTEMP)
    planner.autotemp_M104_M109();
  #endif

}

#endif
