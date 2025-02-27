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

#pragma once

/**
 * @brief Cartesian Point
 * @details Represents a three dimensional point on Cartesian coordinate system,
 *          using an additional fourth dimension for the extrusion length.
 *
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 * @param z The z-coordinate of the point.
 */
struct point_t {

  float x, y, z;

  /**
   * @brief Three dimensional point constructor
   *
   * @param x The x-coordinate of the point.
   * @param y The y-coordinate of the point.
   * @param z The z-coordinate of the point.
   */
  point_t(const float x, const float y, const float z) : x(x), y(y), z(z) {}

  /**
   * @brief Two dimensional point constructor
   *
   * @param x The x-coordinate of the point.
   * @param y The y-coordinate of the point.
   */
  point_t(const float x, const float y) : point_t(x, y, NAN) {}

};
