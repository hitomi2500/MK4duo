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
// TMC Menu
//

#include "../../../MK4duo.h"

#if HAS_LCD_MENU && HAS_TRINAMIC

#define TMC_EDIT_STORED_I_RMS(ST) MENU_ITEM_EDIT_CALLBACK(uint16_4, MSG_##ST, &stepper##ST->val_mA, 100, 3000, refresh_stepper_current_##ST)

#if AXIS_HAS_TMC(X)
  void refresh_stepper_current_X()  { stepperX->refresh_stepper_current();  }
#endif
#if AXIS_HAS_TMC(Y)
  void refresh_stepper_current_Y()  { stepperY->refresh_stepper_current();  }
#endif
#if AXIS_HAS_TMC(Z)
  void refresh_stepper_current_Z()  { stepperZ->refresh_stepper_current();  }
#endif
#if AXIS_HAS_TMC(X2)
  void refresh_stepper_current_X2() { stepperX2->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(Y2)
  void refresh_stepper_current_Y2() { stepperY2->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(Z2)
  void refresh_stepper_current_Z2() { stepperZ2->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(Z3)
  void refresh_stepper_current_Z3() { stepperZ3->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E0)
  void refresh_stepper_current_E0() { stepperE0->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E1)
  void refresh_stepper_current_E1() { stepperE1->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E2)
  void refresh_stepper_current_E2() { stepperE2->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E3)
  void refresh_stepper_current_E3() { stepperE3->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E4)
  void refresh_stepper_current_E4() { stepperE4->refresh_stepper_current(); }
#endif
#if AXIS_HAS_TMC(E5)
  void refresh_stepper_current_E5() { stepperE5->refresh_stepper_current(); }
#endif

void menu_tmc_current() {
  START_MENU();
  MENU_BACK(MSG_TMC_DRIVERS);
  #if AXIS_HAS_TMC(X)
    TMC_EDIT_STORED_I_RMS(X);
  #endif
  #if AXIS_HAS_TMC(Y)
    TMC_EDIT_STORED_I_RMS(Y);
  #endif
  #if AXIS_HAS_TMC(Z)
    TMC_EDIT_STORED_I_RMS(Z);
  #endif
  #if AXIS_HAS_TMC(X2)
    TMC_EDIT_STORED_I_RMS(X2);
  #endif
  #if AXIS_HAS_TMC(Y2)
    TMC_EDIT_STORED_I_RMS(Y2);
  #endif
  #if AXIS_HAS_TMC(Z2)
    TMC_EDIT_STORED_I_RMS(Z2);
  #endif
  #if AXIS_HAS_TMC(Z3)
    TMC_EDIT_STORED_I_RMS(Z3);
  #endif
  #if AXIS_HAS_TMC(E0)
    TMC_EDIT_STORED_I_RMS(E0);
  #endif
  #if AXIS_HAS_TMC(E1)
    TMC_EDIT_STORED_I_RMS(E1);
  #endif
  #if AXIS_HAS_TMC(E2)
    TMC_EDIT_STORED_I_RMS(E2);
  #endif
  #if AXIS_HAS_TMC(E3)
    TMC_EDIT_STORED_I_RMS(E3);
  #endif
  #if AXIS_HAS_TMC(E4)
    TMC_EDIT_STORED_I_RMS(E4);
  #endif
  #if AXIS_HAS_TMC(E5)
    TMC_EDIT_STORED_I_RMS(E5);
  #endif
  END_MENU();
}

#define TMC_EDIT_STORED_MICROSTEPS(ST) MENU_ITEM_EDIT_CALLBACK(microstep, MSG_##ST, &stepper##ST->val_ms, 16, 128, refresh_stepper_microstep_##ST)

#if AXIS_HAS_TMC(X)
  void refresh_stepper_microstep_X()  { stepperX->refresh_stepper_microstep();  }
#endif
#if AXIS_HAS_TMC(Y)
  void refresh_stepper_microstep_Y()  { stepperY->refresh_stepper_microstep();  }
#endif
#if AXIS_HAS_TMC(Z)
  void refresh_stepper_microstep_Z()  { stepperZ->refresh_stepper_microstep();  }
#endif
#if AXIS_HAS_TMC(X2)
  void refresh_stepper_microstep_X2() { stepperX2->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(Y2)
  void refresh_stepper_microstep_Y2() { stepperY2->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(Z2)
  void refresh_stepper_microstep_Z2() { stepperZ2->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(Z3)
  void refresh_stepper_microstep_Z3() { stepperZ3->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E0)
  void refresh_stepper_microstep_E0() { stepperE0->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E1)
  void refresh_stepper_microstep_E1() { stepperE1->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E2)
  void refresh_stepper_microstep_E2() { stepperE2->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E3)
  void refresh_stepper_microstep_E3() { stepperE3->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E4)
  void refresh_stepper_microstep_E4() { stepperE4->refresh_stepper_microstep(); }
#endif
#if AXIS_HAS_TMC(E5)
  void refresh_stepper_microstep_E5() { stepperE5->refresh_stepper_microstep(); }
#endif

void menu_tmc_microstep() {
  START_MENU();
  MENU_BACK(MSG_TMC_DRIVERS);
  #if AXIS_HAS_TMC(X)
    TMC_EDIT_STORED_MICROSTEPS(X);
  #endif
  #if AXIS_HAS_TMC(Y)
    TMC_EDIT_STORED_MICROSTEPS(Y);
  #endif
  #if AXIS_HAS_TMC(Z)
    TMC_EDIT_STORED_MICROSTEPS(Z);
  #endif
  #if AXIS_HAS_TMC(X2)
    TMC_EDIT_STORED_MICROSTEPS(X2);
  #endif
  #if AXIS_HAS_TMC(Y2)
    TMC_EDIT_STORED_MICROSTEPS(Y2);
  #endif
  #if AXIS_HAS_TMC(Z2)
    TMC_EDIT_STORED_MICROSTEPS(Z2);
  #endif
  #if AXIS_HAS_TMC(Z3)
    TMC_EDIT_STORED_MICROSTEPS(Z3);
  #endif
  #if AXIS_HAS_TMC(E0)
    TMC_EDIT_STORED_MICROSTEPS(E0);
  #endif
  #if AXIS_HAS_TMC(E1)
    TMC_EDIT_STORED_MICROSTEPS(E1);
  #endif
  #if AXIS_HAS_TMC(E2)
    TMC_EDIT_STORED_MICROSTEPS(E2);
  #endif
  #if AXIS_HAS_TMC(E3)
    TMC_EDIT_STORED_MICROSTEPS(E3);
  #endif
  #if AXIS_HAS_TMC(E4)
    TMC_EDIT_STORED_MICROSTEPS(E4);
  #endif
  #if AXIS_HAS_TMC(E5)
    TMC_EDIT_STORED_MICROSTEPS(E5);
  #endif
  END_MENU();
}

#if ENABLED(HYBRID_THRESHOLD)

  #define TMC_EDIT_STORED_HYBRID_THRS(ST) MENU_ITEM_EDIT_CALLBACK(uint8, MSG_##ST, &stepper##ST->hybrid_thrs, 0, 255, refresh_hybrid_thrs_##ST);

  #if AXIS_HAS_TMC(X)
    void refresh_hybrid_thrs_X()  {  stepperX->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(Y)
    void refresh_hybrid_thrs_Y()  {  stepperY->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(Z)
    void refresh_hybrid_thrs_Z()  {  stepperZ->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(X2)
    void refresh_hybrid_thrs_X2() { stepperX2->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(Y2)
    void refresh_hybrid_thrs_Y2() { stepperY2->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(Z2)
    void refresh_hybrid_thrs_Z2() { stepperZ2->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(Z3)
    void refresh_hybrid_thrs_Z3() { stepperZ3->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E0)
    void refresh_hybrid_thrs_E0() { stepperE0->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E1)
    void refresh_hybrid_thrs_E1() { stepperE1->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E2)
    void refresh_hybrid_thrs_E2() { stepperE2->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E3)
    void refresh_hybrid_thrs_E3() { stepperE3->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E4)
    void refresh_hybrid_thrs_E4() { stepperE4->refresh_hybrid_thrs(); }
  #endif
  #if AXIS_HAS_TMC(E5)
    void refresh_hybrid_thrs_E5() { stepperE5->refresh_hybrid_thrs(); }
  #endif

  void menu_tmc_hybrid_thrs() {
    START_MENU();
    MENU_BACK(MSG_TMC_DRIVERS);
    #if AXIS_HAS_TMC(X)
      TMC_EDIT_STORED_HYBRID_THRS(X);
    #endif
    #if AXIS_HAS_TMC(Y)
      TMC_EDIT_STORED_HYBRID_THRS(Y);
    #endif
    #if AXIS_HAS_TMC(Z)
      TMC_EDIT_STORED_HYBRID_THRS(Z);
    #endif
    #if AXIS_HAS_TMC(X2)
      TMC_EDIT_STORED_HYBRID_THRS(X2);
    #endif
    #if AXIS_HAS_TMC(Y2)
      TMC_EDIT_STORED_HYBRID_THRS(Y2);
    #endif
    #if AXIS_HAS_TMC(Z2)
      TMC_EDIT_STORED_HYBRID_THRS(Z2);
    #endif
    #if AXIS_HAS_TMC(Z3)
      TMC_EDIT_STORED_HYBRID_THRS(Z3);
    #endif
    #if AXIS_HAS_TMC(E0)
      TMC_EDIT_STORED_HYBRID_THRS(E0);
    #endif
    #if AXIS_HAS_TMC(E1)
      TMC_EDIT_STORED_HYBRID_THRS(E1);
    #endif
    #if AXIS_HAS_TMC(E2)
      TMC_EDIT_STORED_HYBRID_THRS(E2);
    #endif
    #if AXIS_HAS_TMC(E3)
      TMC_EDIT_STORED_HYBRID_THRS(E3);
    #endif
    #if AXIS_HAS_TMC(E4)
      TMC_EDIT_STORED_HYBRID_THRS(E4);
    #endif
    #if AXIS_HAS_TMC(E5)
      TMC_EDIT_STORED_HYBRID_THRS(E5);
    #endif
    END_MENU();
  }

#endif

#if HAS_SENSORLESS

  #define TMC_EDIT_STORED_SGT(ST) MENU_ITEM_EDIT_CALLBACK(int4, MSG_##ST, &stepper##ST->homing_thrs, -64, 63, refresh_homing_thrs_##ST);

  #if X_HAS_SENSORLESS
    void refresh_homing_thrs_X()  { stepperX->refresh_homing_thrs(); }
  #endif
  #if Y_HAS_SENSORLESS
    void refresh_homing_thrs_Y()  { stepperY->refresh_homing_thrs(); }
  #endif
  #if Z_HAS_SENSORLESS
    void refresh_homing_thrs_Z()  { stepperZ->refresh_homing_thrs(); }
  #endif

  void menu_tmc_homing_thrs() {
    START_MENU();
    MENU_BACK(MSG_TMC_DRIVERS);
    #if X_HAS_SENSORLESS
      TMC_EDIT_STORED_SGT(X);
    #endif
    #if Y_HAS_SENSORLESS
      TMC_EDIT_STORED_SGT(Y);
    #endif
    #if Z_HAS_SENSORLESS
      TMC_EDIT_STORED_SGT(Z);
    #endif
    END_MENU();
  }

#endif

#if TMC_HAS_STEALTHCHOP

  #define TMC_EDIT_STEP_MODE(ST) MENU_ITEM_EDIT_CALLBACK(bool, MSG_##ST, &stepper##ST->stealthChop_enabled, refresh_stepping_mode_##ST)

  #if AXIS_HAS_STEALTHCHOP(X)
    void refresh_stepping_mode_X()  { stepperX->refresh_stepping_mode();  }
  #endif
  #if AXIS_HAS_STEALTHCHOP(Y)
    void refresh_stepping_mode_Y()  { stepperY->refresh_stepping_mode();  }
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z)
    void refresh_stepping_mode_Z()  { stepperZ->refresh_stepping_mode();  }
  #endif
  #if AXIS_HAS_STEALTHCHOP(X2)
    void refresh_stepping_mode_X2() { stepperX2->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(Y2)
    void refresh_stepping_mode_Y2() { stepperY2->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z2)
    void refresh_stepping_mode_Z2() { stepperZ2->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(Z3)
    void refresh_stepping_mode_Z3() { stepperZ3->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E0)
    void refresh_stepping_mode_E0() { stepperE0->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E1)
    void refresh_stepping_mode_E1() { stepperE1->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E2)
    void refresh_stepping_mode_E2() { stepperE2->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E3)
    void refresh_stepping_mode_E3() { stepperE3->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E4)
    void refresh_stepping_mode_E4() { stepperE4->refresh_stepping_mode(); }
  #endif
  #if AXIS_HAS_STEALTHCHOP(E5)
    void refresh_stepping_mode_E5() { stepperE5->refresh_stepping_mode(); }
  #endif

  void menu_tmc_step_mode() {
    START_MENU();
    STATIC_ITEM(MSG_TMC_STEALTH_ENABLED);
    MENU_BACK(MSG_TMC_DRIVERS);
    #if AXIS_HAS_STEALTHCHOP(X)
      TMC_EDIT_STEP_MODE(X);
    #endif
    #if AXIS_HAS_STEALTHCHOP(Y)
      TMC_EDIT_STEP_MODE(Y);
    #endif
    #if AXIS_HAS_STEALTHCHOP(Z)
      TMC_EDIT_STEP_MODE(Z);
    #endif
    #if AXIS_HAS_STEALTHCHOP(X2)
      TMC_EDIT_STEP_MODE(X2);
    #endif
    #if AXIS_HAS_STEALTHCHOP(Y2)
      TMC_EDIT_STEP_MODE(Y2);
    #endif
    #if AXIS_HAS_STEALTHCHOP(Z2)
      TMC_EDIT_STEP_MODE(Z2);
    #endif
    #if AXIS_HAS_STEALTHCHOP(Z3)
      TMC_EDIT_STEP_MODE(Z3);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E0)
      TMC_EDIT_STEP_MODE(E0);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E1)
      TMC_EDIT_STEP_MODE(E1);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E2)
      TMC_EDIT_STEP_MODE(E2);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E3)
      TMC_EDIT_STEP_MODE(E3);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E4)
      TMC_EDIT_STEP_MODE(E4);
    #endif
    #if AXIS_HAS_STEALTHCHOP(E5)
      TMC_EDIT_STEP_MODE(E5);
    #endif
    END_MENU();
  }

#endif

void menu_tmc() {
  START_MENU();
  MENU_BACK(MSG_CONTROL);
  MENU_ITEM(submenu, MSG_TMC_CURRENT, menu_tmc_current);
  MENU_ITEM(submenu, MSG_TMC_MICROSTEP, menu_tmc_microstep);
  #if ENABLED(HYBRID_THRESHOLD)
    MENU_ITEM(submenu, MSG_TMC_HYBRID_THRS, menu_tmc_hybrid_thrs);
  #endif
  #if HAS_SENSORLESS
    MENU_ITEM(submenu, MSG_TMC_HOMING_THRS, menu_tmc_homing_thrs);
  #endif
  #if TMC_HAS_STEALTHCHOP
    MENU_ITEM(submenu, MSG_TMC_STEPPING_MODE, menu_tmc_step_mode);
  #endif
  END_MENU();
}

#endif // HAS_TRINAMIC
