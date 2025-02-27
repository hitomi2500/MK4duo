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

#if ENABLED(ARC_SUPPORT)

#if N_ARC_CORRECTION < 1
  #undef N_ARC_CORRECTION
  #define N_ARC_CORRECTION 1
#endif

/**
 * Plan an arc in 2 dimensions
 *
 * The arc is approximated by generating many small linear segments.
 * The length of each segment is configured in MM_PER_ARC_SEGMENT (Default 1mm)
 * Arcs should only be made relatively large (over 5mm), as larger arcs with
 * larger segments will tend to be more efficient. Your slicer should have
 * options for G2/G3 arc generation. In future these options may be GCode tunable.
 */
void plan_arc(const float (&cart)[XYZE], const float (&offset)[2], const uint8_t clockwise) {

  #if ENABLED(CNC_WORKSPACE_PLANES)
    AxisEnum p_axis, q_axis, l_axis;
    switch (mechanics.workspace_plane) {
      default:
      case mechanics.PLANE_XY: p_axis = X_AXIS; q_axis = Y_AXIS; l_axis = Z_AXIS; break;
      case mechanics.PLANE_ZX: p_axis = Z_AXIS; q_axis = X_AXIS; l_axis = Y_AXIS; break;
      case mechanics.PLANE_YZ: p_axis = Y_AXIS; q_axis = Z_AXIS; l_axis = X_AXIS; break;
    }
  #else
    constexpr AxisEnum p_axis = X_AXIS, q_axis = Y_AXIS, l_axis = Z_AXIS;
  #endif

  // Radius vector from center to current location
  float r_P = -offset[0], r_Q = -offset[1];

  const float radius = HYPOT(r_P, r_Q),
              #if ENABLED(AUTO_BED_LEVELING_UBL)
                start_L  = mechanics.current_position[l_axis],
              #endif
              center_P = mechanics.current_position[p_axis] - r_P,
              center_Q = mechanics.current_position[q_axis] - r_Q,
              rt_X = cart[p_axis] - center_P,
              rt_Y = cart[q_axis] - center_Q,
              linear_travel = cart[l_axis] - mechanics.current_position[l_axis],
              extruder_travel = cart[E_AXIS] - mechanics.current_position[E_AXIS];

  // CCW angle of rotation between position and target from the circle center. Only one atan2() trig computation required.
  float angular_travel = ATAN2(r_P * rt_Y - r_Q * rt_X, r_P * rt_X + r_Q * rt_Y);
  if (angular_travel < 0) angular_travel += RADIANS(360);
  #if ENABLED(MIN_ARC_SEGMENTS)
    uint16_t min_segments = CEIL((MIN_ARC_SEGMENTS) * (angular_travel / RADIANS(360)));
    NOLESS(min_segments, 1u);
  #else
    constexpr uint16_t min_segments = 1;
  #endif
  if (clockwise) angular_travel -= RADIANS(360);

  // Make a circle if the angular rotation is 0
  if (angular_travel == 0 && mechanics.current_position[p_axis] == cart[p_axis] && mechanics.current_position[q_axis] == cart[q_axis]) {
    angular_travel = RADIANS(360);
    #if ENABLED(MIN_ARC_SEGMENTS)
      min_segments = MIN_ARC_SEGMENTS;
    #endif
  }

  const float flat_mm = radius * angular_travel,
              mm_of_travel = linear_travel ? HYPOT(flat_mm, linear_travel) : ABS(flat_mm);
  if (mm_of_travel < 0.001f) return;

  uint16_t segments = FLOOR(mm_of_travel / (MM_PER_ARC_SEGMENT));
  if (segments == 0) segments = 1;

  /**
   * Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
   * and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
   *     r_T = [cos(phi) -sin(phi);
   *            sin(phi)  cos(phi] * r ;
   *
   * For arc generation, the center of the circle is the axis of rotation and the radius vector is
   * defined from the circle center to the initial position. Each line segment is formed by successive
   * vector rotations. This requires only two cos() and sin() computations to form the rotation
   * matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
   * all double numbers are single precision on the Arduino. (True double precision will not have
   * round off issues for CNC applications.) Single precision error can accumulate to be greater than
   * tool precision in some cases. Therefore, arc path correction is implemented.
   *
   * Small angle approximation may be used to reduce computation overhead further. This approximation
   * holds for everything, but very small circles and large MM_PER_ARC_SEGMENT values. In other words,
   * theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
   * to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
   * numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
   * issue for CNC machines with the single precision Arduino calculations.
   *
   * This approximation also allows plan_arc to immediately insert a line segment into the planner
   * without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
   * a correction, the planner should have caught up to the lag caused by the initial plan_arc overhead.
   * This is important when there are successive arc motions.
   */
  // Vector rotation matrix values
  float raw[XYZE];
  const float theta_per_segment = angular_travel / segments,
              linear_per_segment = linear_travel / segments,
              extruder_per_segment = extruder_travel / segments,
              sin_T = theta_per_segment,
              cos_T = 1 - 0.5f * sq(theta_per_segment); // Small angle approximation

  // Initialize the linear axis
  raw[l_axis] = mechanics.current_position[l_axis];

  // Initialize the extruder axis
  raw[E_AXIS] = mechanics.current_position[E_AXIS];

  const float fr_mm_s = MMS_SCALED(mechanics.feedrate_mm_s);

  #if ENABLED(SCARA_FEEDRATE_SCALING)
    const float inv_duration = fr_mm_s / MM_PER_ARC_SEGMENT;
  #endif

  millis_s next_idle_ms = millis();

  #if N_ARC_CORRECTION > 1
    int8_t arc_recalc_count = N_ARC_CORRECTION;
  #endif

  for (uint16_t i = 1; i < segments; i++) { // Iterate (segments-1) times

    if (expired(&next_idle_ms, 200U)) printer.idle();

    #if N_ARC_CORRECTION > 1
      if (--arc_recalc_count) {
        // Apply vector rotation matrix to previous r_P / 1
        const float r_new_Y = r_P * sin_T + r_Q * cos_T;
        r_P = r_P * cos_T - r_Q * sin_T;
        r_Q = r_new_Y;
      }
      else
    #endif
    {
      #if N_ARC_CORRECTION > 1
        arc_recalc_count = N_ARC_CORRECTION;
      #endif

      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      // To reduce stuttering, the sin and cos could be computed at different times.
      // For now, compute both at the same time.
      const float cos_Ti = cos(i * theta_per_segment),
                  sin_Ti = sin(i * theta_per_segment);
      r_P = -offset[0] * cos_Ti + offset[1] * sin_Ti;
      r_Q = -offset[0] * sin_Ti - offset[1] * cos_Ti;
    }

    // Update raw location
    raw[p_axis] = center_P + r_P;
    raw[q_axis] = center_Q + r_Q;
    #if ENABLED(AUTO_BED_LEVELING_UBL)
      raw[l_axis] = start_L;
      UNUSED(linear_per_segment);
    #else
      raw[l_axis] += linear_per_segment;
    #endif
    raw[E_AXIS] += extruder_per_segment;

    endstops.apply_motion_limits(raw);

    #if HAS_LEVELING && !PLANNER_LEVELING
      bedlevel.apply_leveling(raw);
    #endif

    if (!planner.buffer_line(raw, fr_mm_s, tools.extruder.active, MM_PER_ARC_SEGMENT
      #if ENABLED(SCARA_FEEDRATE_SCALING)
        , inv_duration
      #endif
    ))
      break;
  }

  // Ensure last segment arrives at target location.
  COPY_ARRAY(raw, cart);
  #if ENABLED(AUTO_BED_LEVELING_UBL)
    raw[l_axis] = start_L;
  #endif

  #if HAS_LEVELING && !PLANNER_LEVELING
    bedlevel.apply_leveling(raw);
  #endif

  planner.buffer_line(raw, fr_mm_s, tools.extruder.active, MM_PER_ARC_SEGMENT
    #if ENABLED(SCARA_FEEDRATE_SCALING)
      , inv_duration
    #endif
  );

  #if ENABLED(AUTO_BED_LEVELING_UBL)
    raw[l_axis] = start_L;
  #endif
  COPY_ARRAY(mechanics.current_position, raw);

}

/**
 * G2: Clockwise Arc
 * G3: Counterclockwise Arc
 *
 * This command has two forms: IJ-form and R-form.
 *
 *  - I specifies an X offset. J specifies a Y offset.
 *    At least one of the IJ parameters is required.
 *    X and Y can be omitted to do a complete circle.
 *    The given XY is not error-checked. The arc ends
 *     based on the angle of the mechanics.destination.
 *    Mixing I or J with R will throw an error.
 *
 *  - R specifies the radius. X or Y is required.
 *    Omitting both X and Y will throw an error.
 *    X or Y must differ from the current XY.
 *    Mixing R with I or J will throw an error.
 *
 *  - P specifies the number of full circles to do
 *    before the specified arc move.
 *
 *  Examples:
 *
 *    G2 I10           ; CW circle centered at X+10
 *    G3 X20 Y12 R14   ; CCW circle with r=14 ending at X20 Y12
 */

#define CODE_G2
#define CODE_G3

void gcode_G2_G3(const bool clockwise) {

  if (printer.isRunning()) {

    #if ENABLED(SF_ARC_FIX)
      const bool relative_mode_backup = printer.isRelativeMode();
      printer.setRelativeMode(true);
    #endif

    commands.get_destination();

    #if ENABLED(LASER) && ENABLED(LASER_FIRE_G1)
      #if ENABLED(INTENSITY_IN_BYTE)
        if (parser.seenval('S')) laser.intensity = ((float)parser.value_byte() / 255.0) * 100.0;
      #else
        if (parser.seenval('S')) laser.intensity = parser.value_float();
      #endif
      if (parser.seenval('L')) laser.duration = parser.value_ulong();
      if (parser.seenval('P')) laser.ppm = parser.value_float();
      if (parser.seenval('D')) laser.diagnostics = parser.value_bool();
      if (parser.seenval('B')) laser.set_mode(parser.value_int());

      laser.status = LASER_ON;
    #endif

    #if ENABLED(SF_ARC_FIX)
      printer.setRelativeMode(relative_mode_backup);
    #endif

    float arc_offset[2] = { 0.0, 0.0 };
    if (parser.seenval('R')) {
      const float r = parser.value_linear_units(),
                  p1 = mechanics.current_position[X_AXIS], q1 = mechanics.current_position[Y_AXIS],
                  p2 = mechanics.destination[X_AXIS],      q2 = mechanics.destination[Y_AXIS];
      if (r && (p2 != p1 || q2 != q1)) {
        const float e = clockwise ^ (r < 0) ? -1 : 1,             // clockwise -1/1, counterclockwise 1/-1
                    dx = p2 - p1, dy = q2 - q1,                   // X and Y differences
                    d = HYPOT(dx, dy),                            // Linear distance between the points
                    dinv = 1/d,                                   // Inverse of d
                    h = SQRT(sq(r) - sq(d * 0.5f)),               // Distance to the arc pivot-point
                    mx = (p1 + p2) * 0.5f, my = (q1 + q2) * 0.5f, // Point between the two points
                    sx = -dy * dinv, sy = dx * dinv,              // Slope of the perpendicular bisector
                    cx = mx + e * h * sx, cy = my + e * h * sy;   // Pivot-point of the arc
        arc_offset[0] = cx - p1;
        arc_offset[1] = cy - q1;
      }
    }
    else {
      if (parser.seenval('I')) arc_offset[0] = parser.value_linear_units();
      if (parser.seenval('J')) arc_offset[1] = parser.value_linear_units();
    }

    if (arc_offset[0] || arc_offset[1]) {

      #if ENABLED(ARC_P_CIRCLES)
        // P indicates number of circles to do
        int8_t circles_to_do = parser.byteval('P');
        if (!WITHIN(circles_to_do, 0, 100))
          SERIAL_LM(ER, MSG_ERR_ARC_ARGS);
        while (circles_to_do--)
          plan_arc(mechanics.current_position, arc_offset, clockwise);
      #endif

      // Send an arc to the planner
      plan_arc(mechanics.destination, arc_offset, clockwise);
      printer.reset_move_ms();
    }
    else {
      // Bad arguments
      SERIAL_LM(ER, MSG_ERR_ARC_ARGS);
    }

    #if ENABLED(LASER) && ENABLED(LASER_FIRE_G1)
      laser.status = LASER_OFF;
    #endif
  }
}

inline void gcode_G2(void) { gcode_G2_G3(true); }
inline void gcode_G3(void) { gcode_G2_G3(false); }

#endif // ARC_SUPPORT
