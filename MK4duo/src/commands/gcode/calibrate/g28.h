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
 * gcode.h
 *
 * Copyright (c) 2019 Alberto Cotronei @MagoKimbra
 */

#define CODE_G28

/**
 * G28: Home all axes according to settings
 *
 * Parameters
 *
 *  None  Home to all axes with no parameters.
 *        With QUICK_HOME enabled XY will home together, then Z.
 *
 *  O     Home only if position is unknown
 *
 *  B     Back, after home return to back position.
 *
 *  Rn    Raise by n mm/inches before homing
 *
 * Cartesian parameters
 *
 *  X     Home to the X endstop
 *  Y     Home to the Y endstop
 *  Z     Home to the Z endstop
 *
 */
inline void gcode_G28(void) { 

  if (printer.debugFeature()) {
    DEBUG_EM(">>> G28");
    DEBUG_LOG_INFO();
  }

  if (mechanics.isHomedAll() && parser.boolval('O')) { // home only if needed
    if (printer.debugFeature()) DEBUG_EM("> homing not needed, skip\n<<< G28");
    return;
  }

  #if IS_KINEMATIC

    mechanics.home();

  #else

    #if ENABLED(FORCE_HOME_XY_BEFORE_Z)
      const bool  homeZ = parser.seen('Z'),
                  homeX = homeZ || parser.seen('X'),
                  homeY = homeZ || parser.seen('Y');
    #else
      const bool  homeX = parser.seen('X'),
                  homeY = parser.seen('Y'),
                  homeZ = parser.seen('Z');
    #endif

    mechanics.home(homeX, homeY, homeZ);

  #endif

}
