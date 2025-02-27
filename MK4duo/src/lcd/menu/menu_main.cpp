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
// Main Menu
//

#include "../../../MK4duo.h"

#if HAS_LCD_MENU

#if HAS_GAMES
  #include "game/game.h"
#endif

bool stop_print_file;

void menu_stop_print() {
  do_select_screen_yn(lcdui.stop_print, lcdui.goto_previous_screen, PSTR(MSG_ARE_YOU_SURE), nullptr, PSTR("?"));
}

#if HAS_EEPROM
  void menu_eeprom() {
    lcdui.defer_status_screen();
    if (lcdui.use_click()) return lcdui.return_to_status();
    START_SCREEN();
    STATIC_ITEM(MSG_EEPROM_CHANGED_ALLERT_1);
    STATIC_ITEM(MSG_EEPROM_CHANGED_ALLERT_2);
    STATIC_ITEM(MSG_EEPROM_CHANGED_ALLERT_3);
    STATIC_ITEM(MSG_EEPROM_CHANGED_ALLERT_4);
    END_SCREEN();
  }
#endif

#if HAS_NEXTION_LCD

  void menu_nextion() {
    lcdui.defer_status_screen();
    if (lcdui.use_click()) return lcdui.return_to_status();
    START_SCREEN();
    STATIC_ITEM(MSG_NEXTION_CHANGED_ALLERT_1);
    STATIC_ITEM(MSG_NEXTION_CHANGED_ALLERT_2);
    STATIC_ITEM(MSG_NEXTION_CHANGED_ALLERT_3);
    STATIC_ITEM(MSG_NEXTION_CHANGED_ALLERT_4);
    STATIC_ITEM(MSG_NEXTION_CHANGED_ALLERT_5);
    END_SCREEN();
  }

  void menu_m0() {
    lcdui.defer_status_screen();
    if (lcdui.use_click()) {
      printer.setWaitForUser(false);
      return;
    }
    START_SCREEN();
    STATIC_ITEM(MSG_NEXTION_M0_M1_1);
    STATIC_ITEM(MSG_NEXTION_M0_M1_2);
    END_SCREEN();
  }

  #if HAS_SD_SUPPORT

    void menu_firmware() {
      do_select_screen_yn(nexlcd.UploadNewFirmware, lcdui.goto_previous_screen, PSTR(MSG_ARE_YOU_SURE), nullptr, PSTR("?"));
    }

  #endif

#endif // HAS_NEXTION_LCD

void menu_tune();
void menu_motion();
void menu_temperature();
void menu_configuration();
void menu_user();
void menu_temp_e0_filament_change();
void menu_change_filament();
void menu_info();
void menu_led();

#if ENABLED(COLOR_MIXING_EXTRUDER) && DISABLED(NEXTION)
  void menu_mixer();
#endif

#if HAS_DHT
  void menu_dht();
#endif

#if ENABLED(SERVICE_TIME_1)
  void menu_service1();
#endif
#if ENABLED(SERVICE_TIME_2)
  void menu_service2();
#endif
#if ENABLED(SERVICE_TIME_3)
  void menu_service3();
#endif

void menu_main() {
  START_MENU();
  MENU_BACK(MSG_WATCH);

  const bool busy = printer.isPrinting();

  if (busy) {
    MENU_ITEM(function, MSG_PAUSE_PRINT, lcdui.pause_print);
    MENU_ITEM(submenu, MSG_STOP_PRINT, menu_stop_print);
    MENU_ITEM(submenu, MSG_TUNE, menu_tune);
  }
  else {
    #if !HAS_ENCODER_WHEEL && HAS_SD_SUPPORT
      //
      // Autostart
      //
      #if ENABLED(MENU_ADDAUTOSTART)
        if (!busy) MENU_ITEM(function, MSG_AUTOSTART, card.beginautostart);
      #endif

      if (card.isDetected()) {
        if (!card.isFileOpen()) {
          #if PIN_EXISTS(SD_DETECT)
            MENU_ITEM(gcode, MSG_CHANGE_SDCARD, PSTR("M21"));
          #else
            MENU_ITEM(gcode, MSG_RELEASE_SDCARD, PSTR("M22"));
          #endif
          MENU_ITEM(submenu, MSG_CARD_MENU, menu_sdcard);
        }
      }
      else {
        #if PIN_EXISTS(SD_DETECT)
          MENU_ITEM(function, MSG_NO_CARD, nullptr);
        #else
          MENU_ITEM(gcode, MSG_INIT_SDCARD, PSTR("M21"));
          MENU_ITEM(function, MSG_SD_RELEASED, nullptr);
        #endif
        
      }
    #endif // !HAS_ENCODER_WHEEL && HAS_SD_SUPPORT

    if (printer.isPaused())
      MENU_ITEM(function, MSG_RESUME_PRINT, lcdui.resume_print);

    MENU_ITEM(submenu, MSG_MOTION, menu_motion);
  }

  if (printer.mode == PRINTER_MODE_FFF) {
    MENU_ITEM(submenu, MSG_TEMPERATURE, menu_temperature);
    #if ENABLED(COLOR_MIXING_EXTRUDER) && DISABLED(NEXTION)
      MENU_ITEM(submenu, MSG_MIXER, menu_mixer);
    #endif
    #if HAS_MMU2
      if (!busy) MENU_ITEM(submenu, MSG_MMU2_MENU, menu_mmu2);
    #endif
  }

  #if HAS_DHT
    MENU_ITEM(submenu, MSG_DHT, menu_dht);
  #endif

  MENU_ITEM(submenu, MSG_CONFIGURATION, menu_configuration);

  #if ENABLED(CUSTOM_USER_MENUS)
    MENU_ITEM(submenu, MSG_USER_MENU, menu_user);
  #endif

  if (printer.mode == PRINTER_MODE_FFF) {
    #if ENABLED(ADVANCED_PAUSE_FEATURE)
      #if DRIVER_EXTRUDERS == 1 && DISABLED(FILAMENT_LOAD_UNLOAD_GCODES)
        if (thermalManager.targetHotEnoughToExtrude(tools.extruder.active))
          MENU_ITEM(gcode, MSG_FILAMENTCHANGE, PSTR("M600 B0"));
        else
          MENU_ITEM(submenu, MSG_FILAMENTCHANGE, menu_temp_e0_filament_change);
      #else
        MENU_ITEM(submenu, MSG_FILAMENTCHANGE, menu_change_filament);
      #endif
    #endif
  }

  #if ENABLED(LCD_INFO_MENU)
    MENU_ITEM(submenu, MSG_INFO_MENU, menu_info);
  #endif

  #if ENABLED(LED_CONTROL_MENU)
    MENU_ITEM(submenu, MSG_LED_CONTROL, menu_led);
  #endif

  //
  // Switch power on/off
  //
  #if HAS_POWER_SWITCH
    if (powerManager.is_on())
      MENU_ITEM(gcode, MSG_SWITCH_PS_OFF, PSTR("M81"));
    else
      MENU_ITEM(gcode, MSG_SWITCH_PS_ON, PSTR("M80"));
  #endif

  #if HAS_ENCODER_WHEEL && HAS_SD_SUPPORT
    //
    // Autostart
    //
    #if ENABLED(MENU_ADDAUTOSTART)
      if (!busy) MENU_ITEM(function, MSG_AUTOSTART, card.beginautostart);
    #endif

    if (card.isDetected()) {
      if (!card.isFileOpen()) {
        MENU_ITEM(submenu, MSG_CARD_MENU, menu_sdcard);
        #if !PIN_EXISTS(SD_DETECT)
          MENU_ITEM(gcode, MSG_CHANGE_SDCARD, PSTR("M21"));  // SD-card changed by user
        #endif
      }
    }
    else {
      #if !PIN_EXISTS(SD_DETECT)
        MENU_ITEM(gcode, MSG_INIT_SDCARD, PSTR("M21")); // Manually initialize the SD-card via user interface
      #endif
      MENU_ITEM(function, MSG_NO_CARD, nullptr);
    }
  #endif // HAS_ENCODER_WHEEL && HAS_SD_SUPPORT

  #if ENABLED(SERVICE_TIME_1)
    MENU_ITEM(submenu, SERVICE_NAME_1, menu_service1);
  #endif
  #if ENABLED(SERVICE_TIME_2)
    MENU_ITEM(submenu, SERVICE_NAME_2, menu_service2);
  #endif
  #if ENABLED(SERVICE_TIME_3)
    MENU_ITEM(submenu, SERVICE_NAME_3, menu_service3);
  #endif

  #if HAS_GAMES
    MENU_ITEM(submenu, "Game", (
      #if HAS_GAME_MENU
        menu_game
      #elif ENABLED(GAME_BRICKOUT)
        brickout.enter_game
      #elif ENABLED(GAME_INVADERS)
        invaders.enter_game
      #elif ENABLED(GAME_MAZE)
        maze.enter_game
      #elif ENABLED(GAME_SNAKE)
        snake.enter_game
      #endif
    ));
  #endif

  END_MENU();
}

#endif // HAS_LCD_MENU
