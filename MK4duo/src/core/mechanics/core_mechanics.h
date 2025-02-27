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
 * core_mechanics.h
 *
 * Copyright (c) 2019 Alberto Cotronei @MagoKimbra
 */

#pragma once

// Struct Core Settings
typedef struct : public generic_data_t {
  axis_limits_t base_pos[XYZ];
} mechanics_data_t;

class Core_Mechanics: public Mechanics {

  public: /** Constructor */

    Core_Mechanics() {}

  public: /** Public Parameters */

    static mechanics_data_t data;

  public: /** Public Function */

    /**
     * Initialize Factory parameters
     */
    static void factory_parameters();

    /**
     * Get the stepper positions in the cartesian_position[] array.
     *
     * The result is in the current coordinate space with
     * leveling applied. The coordinates need to be run through
     * unapply_leveling to obtain the "ideal" coordinates
     * suitable for current_position, etc.
     */
    static void get_cartesian_from_steppers();

    /**
     *  Plan a move to (X, Y, Z) and set the current_position
     *  The final current_position may not be the one that was requested
     */
    static void do_blocking_move_to(const float rx, const float ry, const float rz, const float &fr_mm_s=0.0);
    static void do_blocking_move_to_x(const float &rx, const float &fr_mm_s=0.0);
    static void do_blocking_move_to_z(const float &rz, const float &fr_mm_s=0.0);
    static void do_blocking_move_to_xy(const float &rx, const float &ry, const float &fr_mm_s=0.0);

    FORCE_INLINE static void do_blocking_move_to(const float (&raw)[XYZ], const float &fr_mm_s=0.0) {
      do_blocking_move_to(raw[X_AXIS], raw[Y_AXIS], raw[Z_AXIS], fr_mm_s);
    }

    FORCE_INLINE static void do_blocking_move_to(const float (&raw)[XYZE], const float &fr_mm_s=0.0) {
      do_blocking_move_to(raw[X_AXIS], raw[Y_AXIS], raw[Z_AXIS], fr_mm_s);
    }

    /**
     * Home all axes according to settings
     */
    static void home(const bool homeX=false, const bool homeY=false, const bool homeZ=false);

    /**
     * Home an individual linear axis
     */
    static void do_homing_move(const AxisEnum axis, const float distance, const float fr_mm_s=0.0);

    /**
     * Prepare a linear move in a Cartesian setup.
     *
     * When a mesh-based leveling system is active, moves are segmented
     * according to the configuration of the leveling system.
     *
     * Returns true if current_position[] was set to destination[]
     */
    static bool prepare_move_to_destination_mech_specific();

    /**
     * Set an axis' current position to its home position (after homing).
     *
     * For Core robots this applies one-to-one when an
     * individual axis has been homed.
     *
     * Callers must sync the planner position after calling this!
     */
    static void set_axis_is_at_home(const AxisEnum axis);

    /**
     * Return Home position
     */
    static float axis_home_pos(const AxisEnum axis);
    static float x_home_pos();
    static float y_home_pos();
    static float z_home_pos();

    /**
     * Check position is reachable
     */
    static bool position_is_reachable(const float &rx, const float &ry);
    static bool position_is_reachable_by_probe(const float &rx, const float &ry);

    /**
     * Report current position to host
     */
    static void report_current_position_detail();

    /**
     * Print mechanics parameters in memory
     */
    #if DISABLED(DISABLE_M503)
      static void print_parameters();
      static void print_M92();
      static void print_M201();
      static void print_M203();
      static void print_M204();
      static void print_M205();
      static void print_M206();
      static void print_M228();
    #endif

    #if HAS_NEXTION_LCD && ENABLED(NEXTION_GFX)
      static void nextion_gfx_clear();
    #endif

  private: /** Private Function */

    /**
     *  Home axis
     */
    static void homeaxis(const AxisEnum axis);

    #if ENABLED(QUICK_HOME)
      static void quick_home_xy();
    #endif

    #if ENABLED(Z_SAFE_HOMING)
      static void home_z_safely();
    #endif

    #if ENABLED(DOUBLE_Z_HOMING)
      static void double_home_z();
    #endif

};

extern Core_Mechanics mechanics;
