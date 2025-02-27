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

#include "../../../MK4duo.h"

Nozzle nozzle;

/** Public Parameters */
nozzle_data_t Nozzle::data;

/** Public Function */
void Nozzle::factory_parameters() {

  #if HOTENDS > 1

    #if ENABLED(HOTEND_OFFSET_X) && ENABLED(HOTEND_OFFSET_Y) && ENABLED(HOTEND_OFFSET_Z)
      constexpr float HEoffset[XYZ][6] = {
        HOTEND_OFFSET_X,
        HOTEND_OFFSET_Y,
        HOTEND_OFFSET_Z
      };
    #else
      constexpr float HEoffset[XYZ][HOTENDS] = { 0.0f };
    #endif

    static_assert(
      HEoffset[X_AXIS][0] == 0 && HEoffset[Y_AXIS][0] == 0 && HEoffset[Z_AXIS][0] == 0,
      "Offsets for the first hotend must be 0.0."
    );
    LOOP_XYZ(i) {
      LOOP_HOTEND() data.hotend_offset[i][h] = HEoffset[i][h];
    }

  #endif // HOTENDS > 1

  #if ENABLED(NOZZLE_PARK_FEATURE)
    data.park_point = NOZZLE_PARK_POINT;
  #elif EXTRUDERS > 1
    data.park_point = { 0, 0, TOOL_CHANGE_Z_RAISE };
  #endif

}

#if HAS_LCD
  void Nozzle::set_heating_message() {
    const bool heating = hotends[TARGET_HOTEND].isHeating();
    #if HOTENDS > 1
      lcdui.status_printf_P(0, heating ? PSTR("H%i " MSG_HEATING) : PSTR("H%i " MSG_COOLING), TARGET_HOTEND);
    #else
      lcdui.set_status_P(heating ? PSTR("H " MSG_HEATING) : PSTR("H " MSG_COOLING));
    #endif
  }
#endif

#if ENABLED(NOZZLE_PARK_FEATURE) || EXTRUDERS > 1

  void Nozzle::print_M217() {
    #if ENABLED(NOZZLE_PARK_FEATURE)
      SERIAL_LM(CFG, "Nozzle Park: X<point> Y<point> Z<point>");
      SERIAL_SM(CFG, "  M217");
      SERIAL_MV(" X", LINEAR_UNIT(data.park_point.x));
      SERIAL_MV(" Y", LINEAR_UNIT(data.park_point.y));
      SERIAL_MV(" Z", LINEAR_UNIT(data.park_point.z));
      SERIAL_EOL();
    #else
      SERIAL_LM(CFG, "Z raise: Z<point>:");
      SERIAL_SM(CFG, "  M217");
      SERIAL_MV(" Z", LINEAR_UNIT(data.park_point.z));
      SERIAL_EOL();
    #endif
  }

#endif // ENABLED(NOZZLE_PARK_FEATURE) || EXTRUDERS > 1

#if HOTENDS > 1

  void Nozzle::print_M218(const uint8_t h) {
    SERIAL_LM(CFG, "Hotend offset (unit): H<Hotend> X<offset> Y<offset> Z<offset>:");
    SERIAL_SMV(CFG, "  M218 H", (int)h);
    SERIAL_MV(" X", LINEAR_UNIT(data.hotend_offset[X_AXIS][h]), 3);
    SERIAL_MV(" Y", LINEAR_UNIT(data.hotend_offset[Y_AXIS][h]), 3);
    SERIAL_MV(" Z", LINEAR_UNIT(data.hotend_offset[Z_AXIS][h]), 3);
    SERIAL_EOL();
  }

#endif

#if ENABLED(NOZZLE_CLEAN_FEATURE)

  void Nozzle::clean(const uint8_t &pattern, const uint8_t &strokes, const float &radius, const uint8_t &objects, const uint8_t cleans) {

    point_t start = NOZZLE_CLEAN_START_POINT;
    point_t end   = NOZZLE_CLEAN_END_POINT;

    if (pattern == 2) {
      if (!(cleans & (_BV(X_AXIS) | _BV(Y_AXIS)))) {
        SERIAL_EM("Warning: Clean Circle requires XY");
        return;
      }
      end = NOZZLE_CLEAN_CIRCLE_MIDDLE;
    }
    else {
      if (!TEST(cleans, X_AXIS))  start.x = end.x = mechanics.current_position[X_AXIS];
      if (!TEST(cleans, X_AXIS))  start.y = end.y = mechanics.current_position[Y_AXIS];
    }
    if (!TEST(cleans, Z_AXIS))    start.z = end.z = mechanics.current_position[Z_AXIS];

    #if MECH(DELTA)
      if (mechanics.current_position[Z_AXIS] > mechanics.delta_clip_start_height)
        mechanics.do_blocking_move_to_z(mechanics.delta_clip_start_height);
    #endif

    switch (pattern) {
      case 1:
        zigzag(start, end, strokes, objects);
        break;

      case 2:
        circle(start, end, strokes, radius);
        break;

      default:
        stroke(start, end, strokes);
    }
  }

#endif // ENABLED(NOZZLE_CLEAN_FEATURE)

#if ENABLED(NOZZLE_PARK_FEATURE)

  void Nozzle::park(const uint8_t z_action, const point_t &park_p/*=data.park_point*/) {

    const float fr_xy = NOZZLE_PARK_XY_FEEDRATE;
    const float fr_z  = NOZZLE_PARK_Z_FEEDRATE;

    switch (z_action) {
      case 1: // force Z-park height
        mechanics.do_blocking_move_to_z(park_p.z, fr_z);
        break;

      case 2: // Raise by Z-park height
        mechanics.do_blocking_move_to_z(MIN(mechanics.current_position[Z_AXIS] + park_p.z, Z_MAX_BED), fr_z);
        break;

      default: // Raise to Z-park height if lower
        mechanics.do_blocking_move_to_z(MAX(park_p.z, mechanics.current_position[Z_AXIS]), fr_z);
    }

    mechanics.do_blocking_move_to_xy(park_p.x, park_p.y, fr_xy);
  }

#endif // ENABLED(NOZZLE_PARK_FEATURE)

/** Private Function */
#if ENABLED(NOZZLE_CLEAN_FEATURE)

  void Nozzle::stroke(const point_t &start, const point_t &end, const uint8_t &strokes) {

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Store the current coords
      const float initial[XYZ] = {
        mechanics.current_position[X_AXIS],
        mechanics.current_position[Y_AXIS],
        mechanics.current_position[Z_AXIS]
      };
    #endif

    // Move to the starting point
    #if ENABLED(NOZZLE_CLEAN_NO_Z)
      mechanics.do_blocking_move_to_xy(start.x, start.y);
    #else
      mechanics.do_blocking_move_to(start.x, start.y, start.z);
    #endif

    // Start the stroke pattern
    for (uint8_t i = 0; i < (strokes >>1); i++) {
      mechanics.do_blocking_move_to_xy(end.x, end.y);
      mechanics.do_blocking_move_to_xy(start.x, start.y);
    }

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Move the nozzle to the initial point
      mechanics.do_blocking_move_to(initial);
    #endif
  }

  void Nozzle::zigzag(const point_t &start, const point_t &end, const uint8_t &strokes, const uint8_t &objects) {

    const float diffx = end.x - start.x,
                diffy = end.y - start.y;

    if (!diffx || !diffy) return;

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Store the current coords
      const float initial[XYZ] = {
        mechanics.current_position[X_AXIS],
        mechanics.current_position[Y_AXIS],
        mechanics.current_position[Z_AXIS]
      };
    #endif

    #if ENABLED(NOZZLE_CLEAN_NO_Z)
      mechanics.do_blocking_move_to_xy(start.x, start.y);
    #else
      mechanics.do_blocking_move_to(start.x, start.y, start.z);
    #endif

    const uint8_t zigs = objects << 1;
    const bool horiz = ABS(diffx) >= ABS(diffy);    // Do a horizontal wipe?
    const float P = (horiz ? diffx : diffy) / zigs;   // Period of each zig / zag
    const point_t *side;

    for (uint8_t j = 0; j < strokes; j++) {
      for (int8_t i = 0; i < zigs; i++) {
        side = (i & 1) ? &end : &start;
        if (horiz)
          mechanics.do_blocking_move_to_xy(start.x + i * P, side->y);
        else
          mechanics.do_blocking_move_to_xy(side->x, start.y + i * P);
      }

      for (int8_t i = zigs; i >= 0; i--) {
        side = (i & 1) ? &end : &start;
        if (horiz)
          mechanics.do_blocking_move_to_xy(start.x + i * P, side->y);
        else
          mechanics.do_blocking_move_to_xy(side->x, start.y + i * P);
      }
    }

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Move the nozzle to the initial point
      mechanics.do_blocking_move_to(initial);
    #endif
  }

  void Nozzle::circle(const point_t &start, const point_t &middle, const uint8_t &strokes, const float &radius) {

    if (strokes == 0) return;

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Store the current coords
      const float initial[XYZ] = {
        mechanics.current_position[X_AXIS],
        mechanics.current_position[Y_AXIS],
        mechanics.current_position[Z_AXIS]
      };
    #endif

    #if ENABLED(NOZZLE_CLEAN_NO_Z)
      mechanics.do_blocking_move_to_xy(start.x, start.y);
    #else
      mechanics.do_blocking_move_to(start.x, start.y, start.z);
    #endif

    for (uint8_t s = 0; s < strokes; s++) {
      for (uint8_t i = 0; i < NOZZLE_CLEAN_CIRCLE_FN; i++) {
        mechanics.do_blocking_move_to_xy(
          middle.x + SIN((RADIANS(360) / NOZZLE_CLEAN_CIRCLE_FN) * i) * radius,
          middle.y + COS((RADIANS(360) / NOZZLE_CLEAN_CIRCLE_FN) * i) * radius
        );
      }
    }

    // Let's be safe
    mechanics.do_blocking_move_to_xy(start.x, start.y);

    #if ENABLED(NOZZLE_CLEAN_GOBACK)
      // Move the nozzle to the initial point
      mechanics.do_blocking_move_to_xy(initial[X_AXIS], initial[Y_AXIS], initial[Z_AXIS]);
    #endif
  }

#endif // ENABLED(NOZZLE_CLEAN_FEATURE)
