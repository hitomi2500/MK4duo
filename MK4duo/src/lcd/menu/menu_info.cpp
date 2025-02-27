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

//
// Info Menu
//

#include "../../../MK4duo.h"

#if HAS_LCD_MENU && ENABLED(LCD_INFO_MENU)

void menu_info_stats() {
  if (lcdui.use_click()) return lcdui.goto_previous_screen();

  char buffer[21];
  duration_t elapsed;
  printStatistics stats = print_job_counter.getStats();

  START_SCREEN();
  STATIC_ITEM(MSG_INFO_PRINT_COUNT ":", false, false, i16tostr3left(stats.totalPrints));
  STATIC_ITEM(MSG_INFO_COMPLETED_PRINTS ":",  false, false, i16tostr3left(stats.finishedPrints));

  elapsed = stats.timePrint;
  elapsed.toString(buffer);
  STATIC_ITEM(MSG_INFO_PRINT_TIME ":", false, false);
  STATIC_ITEM(">", false, false, buffer);

  elapsed = stats.longestPrint;
  elapsed.toString(buffer);
  STATIC_ITEM(MSG_INFO_PRINT_LONGEST ":", false, false);
  STATIC_ITEM(">", false, false, buffer);

  elapsed = stats.timePowerOn;
  elapsed.toString(buffer);
  STATIC_ITEM(MSG_INFO_POWER_ON ":", false, false);
  STATIC_ITEM(">", false, false, buffer);

  ftostrlength(buffer, stats.filamentUsed);
  STATIC_ITEM(MSG_INFO_PRINT_FILAMENT ": ", false, false);
  STATIC_ITEM(">", false, false, buffer);

  #if HAS_POWER_CONSUMPTION_SENSOR
    sprintf_P(buffer, PSTR("%uWh"), stats.consumptionHour);
    STATIC_ITEM(MSG_INFO_PWRCONSUMED ":",  false, false);
    STATIC_ITEM(">", false, false, buffer);
  #endif

  #if ENABLED(SERVICE_TIME_1)
    elapsed = stats.ServiceTime1;
    elapsed.toString(buffer);
    STATIC_ITEM(SERVICE_NAME_1 " in: ", false, false);
    STATIC_ITEM(">", false, false, buffer);
  #endif

  #if ENABLED(SERVICE_TIME_2)
    elapsed = stats.ServiceTime2;
    elapsed.toString(buffer);
    STATIC_ITEM(SERVICE_NAME_2 " in: ", false, false);
    STATIC_ITEM("> ", false, false, buffer);
  #endif

  #if ENABLED(SERVICE_TIME_3)
    elapsed = stats.ServiceTime3;
    elapsed.toString(buffer);
    STATIC_ITEM(SERVICE_NAME_3 " in: ", false, false);
    STATIC_ITEM("> ", false, false, buffer);
  #endif

  END_SCREEN();
}

/**
 *
 * About Printer > Thermistors
 *
 */
void menu_info_thermistors() {
  if (lcdui.use_click()) return lcdui.goto_previous_screen();
  START_SCREEN();

  #if HAS_TEMP_HE0
    STATIC_ITEM("T0: " HOT0_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_0_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_0_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE1
    STATIC_ITEM("T1: " HOT1_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_1_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_1_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE2
    STATIC_ITEM("T2: " HOT2_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_2_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_2_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE3
    STATIC_ITEM("T3: " HOT3_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_3_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_3_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE4
    STATIC_ITEM("T4: " HOT4_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_4_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_4_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE5
    STATIC_ITEM("T5: " HOT5_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_5_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_5_MAXTEMP), false);
  #endif

  #if HAS_TEMP_HE6
    STATIC_ITEM("T6: " HOT6_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(HOTEND_6_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(HOTEND_6_MAXTEMP), false);
  #endif

  #if HAS_TEMP_BED0
    STATIC_ITEM("Bed:" BED0_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(BED_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(BED_MAXTEMP), false);
  #endif

  #if HAS_TEMP_BED1
    STATIC_ITEM("Bed1:" BED1_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(BED_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(BED_MAXTEMP), false);
  #endif

  #if HAS_TEMP_BED2
    STATIC_ITEM("Bed2:" BED2_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(BED_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(BED_MAXTEMP), false);
  #endif

  #if HAS_TEMP_BED3
    STATIC_ITEM("Bed3:" BED3_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(BED_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(BED_MAXTEMP), false);
  #endif

  #if HAS_TEMP_CHAMBER0
    STATIC_ITEM("Chamber:" CHAMBER0_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(CHAMBER_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(CHAMBER_MAXTEMP), false);
  #endif

  #if HAS_TEMP_CHAMBER1
    STATIC_ITEM("Chamber1:" CHAMBER1_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(CHAMBER_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(CHAMBER_MAXTEMP), false);
  #endif

  #if HAS_TEMP_CHAMBER2
    STATIC_ITEM("Chamber2:" CHAMBER2_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(CHAMBER_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(CHAMBER_MAXTEMP), false);
  #endif

  #if HAS_TEMP_CHAMBER3
    STATIC_ITEM("Chamber3:" CHAMBER3_NAME, false, true);
    STATIC_ITEM(MSG_INFO_MIN_TEMP ": " STRINGIFY(CHAMBER_MINTEMP), false);
    STATIC_ITEM(MSG_INFO_MAX_TEMP ": " STRINGIFY(CHAMBER_MAXTEMP), false);
  #endif

  END_SCREEN();
}

/**
 *
 * About Printer > Board Info
 *
 */
void menu_info_board() {
  if (lcdui.use_click()) return lcdui.goto_previous_screen();
  START_SCREEN();
  STATIC_ITEM(BOARD_NAME, true, true);                              // Board
  STATIC_ITEM(MSG_INFO_BAUDRATE ": " STRINGIFY(BAUDRATE_1), true);  // Baud: 250000
  STATIC_ITEM(MSG_INFO_PROTOCOL ": " PROTOCOL_VERSION, true);       // Protocol: 2.0
  STATIC_ITEM(MSG_INFO_PSU ": " POWER_NAME, true);                  // Power Supply: Normal
  END_SCREEN();
}

/**
 *
 * About Printer > Firmware Info
 *
 */
void menu_info_firmware() {
  if (lcdui.use_click()) return lcdui.goto_previous_screen();
  START_SCREEN();
  STATIC_ITEM(FIRMWARE_NAME, true, true);
  STATIC_ITEM(SHORT_BUILD_VERSION, true);
  STATIC_ITEM(STRING_REVISION_DATE, true);
  STATIC_ITEM(MACHINE_NAME, true);
  STATIC_ITEM(FIRMWARE_URL, true);
  STATIC_ITEM(MSG_INFO_EXTRUDERS ": " STRINGIFY(EXTRUDERS), true);
  STATIC_ITEM(MSG_INFO_HOTENDS ": " STRINGIFY(HOTENDS), true);
  #if ENABLED(AUTO_BED_LEVELING_3POINT)
    STATIC_ITEM(MSG_3POINT_LEVELING, true);     // 3-Point Leveling
  #elif ENABLED(AUTO_BED_LEVELING_LINEAR)
    STATIC_ITEM(MSG_LINEAR_LEVELING, true);     // Linear Leveling
  #elif ENABLED(AUTO_BED_LEVELING_BILINEAR)
    STATIC_ITEM(MSG_BILINEAR_LEVELING, true);   // Bi-linear Leveling
  #elif ENABLED(AUTO_BED_LEVELING_UBL)
    STATIC_ITEM(MSG_UBL_LEVELING, true);        // Unified Bed Leveling
  #elif ENABLED(MESH_BED_LEVELING)
    STATIC_ITEM(MSG_MESH_LEVELING, true);       // Mesh Leveling
  #endif
  END_SCREEN();
}

/**
 *
 * "About Printer" submenu
 *
 */
void menu_info() {
  START_MENU();
  MENU_BACK(MSG_MAIN);
  MENU_ITEM(submenu, MSG_INFO_FIRMWARE_MENU, menu_info_firmware);       // Printer Info >
  MENU_ITEM(submenu, MSG_INFO_BOARD_MENU, menu_info_board);             // Board Info >
  MENU_ITEM(submenu, MSG_INFO_THERMISTOR_MENU, menu_info_thermistors);  // Thermistors >
  MENU_ITEM(submenu, MSG_INFO_STATS_MENU, menu_info_stats);             // Printer Statistics >
  END_MENU();
}

#endif // HAS_LCD_MENU && LCD_INFO_MENU
