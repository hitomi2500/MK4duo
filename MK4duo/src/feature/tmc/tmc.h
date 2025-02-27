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
 * tmc.h
 *
 * Copyright (c) 2019 Alberto Cotronei @MagoKimbra
 */

#if HAS_TRINAMIC

#include <TMCStepper.h>

#if TMCSTEPPER_VERSION < 0x000401
  #error "Update TMCStepper library to 0.4.1 or newer."
#endif

#if HAVE_DRV(TMC2130)
  #define TMC_MODEL_LIB TMC2130Stepper
#elif HAVE_DRV(TMC2160)
  #define TMC_MODEL_LIB TMC2160Stepper
#elif HAVE_DRV(TMC5130)
  #define TMC_MODEL_LIB TMC5130Stepper
#elif HAVE_DRV(TMC5160)
  #define TMC_MODEL_LIB TMC5160Stepper
#endif

#define TMC_X_LABEL "X", 0
#define TMC_Y_LABEL "Y", 1
#define TMC_Z_LABEL "Z", 2

#define TMC_X2_LABEL "X2", 0
#define TMC_Y2_LABEL "Y2", 1
#define TMC_Z2_LABEL "Z2", 2
#define TMC_Z3_LABEL "Z3", 2

#define TMC_E0_LABEL "E0", 3
#define TMC_E1_LABEL "E1", 4
#define TMC_E2_LABEL "E2", 5
#define TMC_E3_LABEL "E3", 6
#define TMC_E4_LABEL "E4", 7
#define TMC_E5_LABEL "E5", 8

#define TMC_AXIS 13

#if ENABLED(MONITOR_DRIVER_STATUS) && DISABLED(MONITOR_DRIVER_STATUS_INTERVAL_MS)
  #define MONITOR_DRIVER_STATUS_INTERVAL_MS 500U
#endif

struct TMC_driver_data {
  uint32_t  drv_status;
  bool      is_otpw:  1,
            is_ot:    1,
            is_s2g:   1,
            is_error: 1
            #if ENABLED(TMC_DEBUG)
              , is_stall:             1
              , is_stealth:           1
              , is_standstill:        1
              , sg_result_reasonable: 1
            #endif
      ;
  #if ENABLED(TMC_DEBUG)
    #if HAS_TMCX1X0 || HAVE_DRV(TMC2208)
      uint8_t cs_actual;
    #endif
    uint16_t sg_result;
  #endif
};

typedef struct {
  uint8_t toff;
  int8_t  hend;
  uint8_t hstrt;
} chopper_timing_t;

static constexpr chopper_timing_t chopper_timing = CHOPPER_TIMING;

static constexpr int8_t sgt_min = -64,
                        sgt_max =  63;

extern bool report_tmc_status;

constexpr uint16_t tmc_thrs(const uint16_t msteps, const uint32_t thrs, const uint32_t spmm) {
  return 12650000UL * msteps / (256 * thrs * spmm);
}

class TMCStorage {

  protected: /** Protected Parameters */

    TMCStorage() {}

  public: /** Public Parameters */

    uint16_t  val_mA  = 0,
              val_ms  = 0;

    uint8_t hybrid_thrs = 0;

    #if TMC_HAS_STEALTHCHOP
      bool stealthChop_enabled = false;
    #endif

    #if HAS_SENSORLESS
      int16_t homing_thrs = 0;
    #endif

    #if ENABLED(MONITOR_DRIVER_STATUS)
      uint8_t  otpw_count = 0,
              error_count = 0;
      bool flag_otpw = false;
    #endif

  public: /** Public Function */

    inline uint16_t getMilliamps()  { return val_mA; }
    inline uint16_t getMicrosteps() { return val_ms; }

    #if ENABLED(MONITOR_DRIVER_STATUS)
      inline bool getOTPW() { return flag_otpw; }
      inline void clear_otpw() { flag_otpw = 0; }
    #endif

};

#if HAVE_DRV(TMC2208)

  //
  // TMC2208 Driver Class
  //
  class MKTMC : public TMC2208Stepper, public TMCStorage {

    public: /** Constructor */

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, Stream * SerialPort, float RS, bool has_rx=true) :
        TMC2208Stepper(SerialPort, RS, has_rx=true),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, uint16_t RX, uint16_t TX, float RS, bool has_rx=true) :
        TMC2208Stepper(RX, TX, RS, has_rx=true),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

    public: /** Public Parameters */

      const char* axis_letter;
      const uint8_t id;

    public: /** Public Function */

      inline void printLabel() { SERIAL_TXT(axis_letter); }

      inline uint16_t rms_current() { return TMC2208Stepper::rms_current(); }

      inline void rms_current(const uint16_t mA) {
        this->val_mA = mA;
        TMC2208Stepper::rms_current(mA);
      }

      inline void rms_current(const uint16_t mA, const float mult) {
        this->val_mA = mA;
        TMC2208Stepper::rms_current(mA, mult);
      }

      inline uint16_t microsteps() { return TMC2208Stepper::microsteps(); }

      inline void microsteps(const uint16_t ms) {
        this->val_ms = ms;
        TMC2208Stepper::microsteps(ms);
      }

      inline void refresh_stepping_mode()   { this->en_spreadCycle(!this->stealthChop_enabled); }
      inline bool get_stealthChop_status()  { return !this->en_spreadCycle(); }

      #if ENABLED(HYBRID_THRESHOLD)
        uint32_t get_pwm_thrs() {
          return tmc_thrs(microsteps(), this->TPWMTHRS(), mechanics.data.axis_steps_per_mm[this->id]);
        }
        void set_pwm_thrs(const uint32_t thrs) {
          TMC2208Stepper::TPWMTHRS(tmc_thrs(microsteps(), thrs, mechanics.data.axis_steps_per_mm[this->id]));
          #if HAS_LCD_MENU
            this->hybrid_thrs = thrs;
          #endif
        }
      #endif

      #if HAS_LCD_MENU
        inline void refresh_stepper_current()   { rms_current(val_mA); }
        inline void refresh_stepper_microstep() { microsteps(val_ms); }

        #if ENABLED(HYBRID_THRESHOLD)
          inline void refresh_hybrid_thrs() { set_pwm_thrs(this->hybrid_thrs); }
        #endif
      #endif

  };

#elif HAVE_DRV(TMC2660)

  //
  // TMC2660 Driver Class
  //
  class MKTMC : public TMC2660Stepper, public TMCStorage {

    public: /** Constructor */

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, uint16_t cs_pin, float RS) :
        TMC2660Stepper(cs_pin, RS),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, uint16_t CS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK) :
        TMC2660Stepper(CS, RS, pinMOSI, pinMISO, pinSCK),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

    public: /** Public Parameters */

      const char* axis_letter;
      const uint8_t id;

    public: /** Public Function */

      inline void printLabel() { SERIAL_TXT(axis_letter); }

      inline uint16_t rms_current() { return TMC2660Stepper::rms_current(); }

      inline void rms_current(uint16_t mA) {
        this->val_mA = mA;
        TMC2660Stepper::rms_current(mA);
      }

      inline void microsteps(const uint16_t ms) {
        this->val_ms = ms;
        TMC2660Stepper::microsteps(ms);
      }

      #if USE_SENSORLESS
        inline int16_t homing_threshold() { return TMC2660Stepper::sgt(); }
        void homing_threshold(int16_t sgt_val) {
          sgt_val = (int16_t)constrain(sgt_val, sgt_min, sgt_max);
          TMC2660Stepper::sgt(sgt_val);
          #if HAS_LCD_MENU
            this->homing_thrs = sgt_val;
          #endif
        }
      #endif

      #if HAS_LCD_MENU
        inline void refresh_stepper_current()   { rms_current(this->val_mA); }
        inline void refresh_stepper_microstep() { microsteps(this->val_ms); }

        #if HAS_SENSORLESS
          inline void refresh_homing_thrs() { homing_threshold(this->homing_thrs); }
        #endif
      #endif

  };

#elif HAS_TMCX1X0

  //
  // TMC Driver Class
  //
  class MKTMC : public TMC_MODEL_LIB, public TMCStorage {

    public: /** Constructor */

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, uint16_t cs_pin, float RS) :
        TMC_MODEL_LIB(cs_pin, RS),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

      MKTMC(const char* AXIS_LETTER, uint8_t DRIVER_ID, uint16_t CS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK) :
        TMC_MODEL_LIB(CS, RS, pinMOSI, pinMISO, pinSCK),
        axis_letter(AXIS_LETTER),
        id(DRIVER_ID)
        {}

    public: /** Public Parameters */

      const char* axis_letter;
      const uint8_t id;

    public: /** Public Function */

      inline void printLabel() { SERIAL_TXT(axis_letter); }

      inline uint16_t rms_current() { return TMC_MODEL_LIB::rms_current(); }

      inline void rms_current(const uint16_t mA) {
        this->val_mA = mA;
        TMC_MODEL_LIB::rms_current(mA);
      }

      inline void rms_current(const uint16_t mA, const float mult) {
        this->val_mA = mA;
        TMC_MODEL_LIB::rms_current(mA, mult);
      }

      inline uint16_t microsteps() { return TMC_MODEL_LIB::microsteps(); }

      inline void microsteps(const uint16_t ms) {
        this->val_ms = ms;
        TMC_MODEL_LIB::microsteps(ms);
      }

      inline void refresh_stepping_mode()   { this->en_pwm_mode(this->stealthChop_enabled); }
      inline bool get_stealthChop_status()  { return this->en_pwm_mode(); }

      #if ENABLED(HYBRID_THRESHOLD)
        uint32_t get_pwm_thrs() {
          return tmc_thrs(microsteps(), this->TPWMTHRS(), mechanics.data.axis_steps_per_mm[this->id]);
        }
        void set_pwm_thrs(const uint32_t thrs) {
          TMC_MODEL_LIB::TPWMTHRS(tmc_thrs(microsteps(), thrs, mechanics.data.axis_steps_per_mm[this->id]));
          #if HAS_LCD_MENU
            this->hybrid_thrs = thrs;
          #endif
        }
      #endif
      #if HAS_SENSORLESS
        inline int16_t homing_threshold() { return TMC_MODEL_LIB::sgt(); }
        void homing_threshold(int16_t sgt_val) {
          sgt_val = (int16_t)constrain(sgt_val, sgt_min, sgt_max);
          TMC_MODEL_LIB::sgt(sgt_val);
          #if HAS_LCD_MENU
            this->homing_thrs = sgt_val;
          #endif
        }
      #endif

      #if ENABLED(SPI_ENDSTOPS)

        bool test_stall_status() {
          uint16_t sg_result = 0;

          this->switchCSpin(LOW);

          if (this->TMC_SW_SPI != nullptr) {
            this->TMC_SW_SPI->transfer(TMC2130_n::DRV_STATUS_t::address);
            this->TMC_SW_SPI->transfer16(0);
            // We only care about the last 10 bits
            sg_result = this->TMC_SW_SPI->transfer(0);
            sg_result <<= 8;
            sg_result |= this->TMC_SW_SPI->transfer(0);
          }
          else {
            SPI.beginTransaction(SPISettings(16000000/8, MSBFIRST, SPI_MODE3));
            // Read DRV_STATUS
            SPI.transfer(TMC2130_n::DRV_STATUS_t::address);
            SPI.transfer16(0);
            // We only care about the last 10 bits
            sg_result = SPI.transfer(0);
            sg_result <<= 8;
            sg_result |= SPI.transfer(0);
            SPI.endTransaction();
          }
          this->switchCSpin(HIGH);

          return (sg_result & 0x3FF) == 0;
        }

      #endif // SPI_ENDSTOPS

      #if HAS_LCD_MENU
        inline void refresh_stepper_current()   { rms_current(this->val_mA); }
        inline void refresh_stepper_microstep() { microsteps(this->val_ms); }

        #if ENABLED(HYBRID_THRESHOLD)
          inline void refresh_hybrid_thrs() { set_pwm_thrs(this->hybrid_thrs); }
        #endif
        #if HAS_SENSORLESS
          inline void refresh_homing_thrs() { homing_threshold(this->homing_thrs); }
        #endif
      #endif

  };

#endif

#if AXIS_HAS_TMC(X)
  extern MKTMC* stepperX;
#endif
#if AXIS_HAS_TMC(X2)
  extern MKTMC* stepperX2;
#endif
#if AXIS_HAS_TMC(Y)
  extern MKTMC* stepperY;
#endif
#if AXIS_HAS_TMC(Y2)
  extern MKTMC* stepperY2;
#endif
#if AXIS_HAS_TMC(Z)
  extern MKTMC* stepperZ;
#endif
#if AXIS_HAS_TMC(Z2)
  extern MKTMC* stepperZ2;
#endif
#if AXIS_HAS_TMC(Z3)
  extern MKTMC* stepperZ3;
#endif
#if AXIS_HAS_TMC(E0)
  extern MKTMC* stepperE0;
#endif
#if AXIS_HAS_TMC(E1)
  extern MKTMC* stepperE1;
#endif
#if AXIS_HAS_TMC(E2)
  extern MKTMC* stepperE2;
#endif
#if AXIS_HAS_TMC(E3)
  extern MKTMC* stepperE3;
#endif
#if AXIS_HAS_TMC(E4)
  extern MKTMC* stepperE4;
#endif
#if AXIS_HAS_TMC(E5)
  extern MKTMC* stepperE5;
#endif

class TMC_Stepper {

  public: /** Constructor */

    TMC_Stepper() {}

  public: /** Public Parameters */

    #if HAS_SENSORLESS && ENABLED(IMPROVE_HOMING_RELIABILITY)
      static millis_l sg_guard_period;
      static constexpr uint16_t default_sg_guard_duration = 400;
    #endif

  private: /** Private Parameters */

    static uint16_t report_status_interval;

  public: /** Public Function */

    static void init();
    static void factory_parameters();

    static void restore();

    static void test_connection(const bool test_x, const bool test_y, const bool test_z, const bool test_e);

    #if ENABLED(MONITOR_DRIVER_STATUS)
      static void monitor_driver();
    #endif

    #if HAS_SENSORLESS
      static bool enable_stallguard(MKTMC* st);
      static void disable_stallguard(MKTMC* st, const bool enable);
    #endif

    #if ENABLED(TMC_DEBUG)
      #if ENABLED(MONITOR_DRIVER_STATUS)
        static void set_report_interval(const uint16_t update_interval);
      #endif
      static void report_all(const bool print_x, const bool print_y, const bool print_z, const bool print_e);
      static void get_registers(const bool print_x, const bool print_y, const bool print_z, const bool print_e);
    #endif

    MKTMC* driver_by_index(const uint8_t index);

    #if DISABLED(DISABLE_M503)
      static void print_M350();
      static void print_M906();
      static void print_M913();
      static void print_M914();
      static void print_M940();
    #endif

    #if ENABLED(MONITOR_DRIVER_STATUS)

      FORCE_INLINE static void report_otpw(MKTMC* st) {
        st->printLabel();
        SERIAL_ELOGIC(" temperature prewarn triggered:", st->getOTPW());
      }

      FORCE_INLINE static void clear_otpw(MKTMC* st) {
        st->clear_otpw();
        st->printLabel();
        SERIAL_EM(" prewarn flag cleared");
      }

    #endif

    FORCE_INLINE static void get_off_time(MKTMC* st) {
      st->printLabel();
      SERIAL_EMV(" off_time: ", st->toff());
    }

    FORCE_INLINE static void set_off_time(MKTMC* st, const uint8_t off_time_val) {
      st->toff(off_time_val);
    }

    #if HAS_TMCX1X0

      FORCE_INLINE static void get_blank_time(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" blank_time: ", st->blank_time());
      }

      FORCE_INLINE static void set_blank_time(MKTMC* st, const uint8_t blank_time_val) {
        st->blank_time(blank_time_val);
      }

      FORCE_INLINE static void get_hysteresis_end(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" hysteresis_end: ", st->hysteresis_end());
      }

      FORCE_INLINE static void set_hysteresis_end(MKTMC* st, const int8_t hysteresis_end_val) {
        st->hysteresis_end(hysteresis_end_val);
      }

      FORCE_INLINE static void get_hysteresis_start(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" hysteresis_start: ", st->hysteresis_start());
      }

      FORCE_INLINE static void set_hysteresis_start(MKTMC* st, const uint8_t hysteresis_start_val) {
        st->hysteresis_start(hysteresis_start_val);
      }

      FORCE_INLINE static void get_disable_I_comparator(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" disable_I_comparator: ", st->disfdcc());
      }

      FORCE_INLINE static void set_disable_I_comparator(MKTMC* st, const bool onoff) {
        st->disfdcc(onoff);
      }

      FORCE_INLINE static void get_stealth_gradient(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" stealth_gradient: ", st->pwm_grad());
      }

      FORCE_INLINE static void set_stealth_gradient(MKTMC* st, const uint8_t stealth_gradient_val) {
        st->pwm_grad(stealth_gradient_val);
      }

      FORCE_INLINE static void get_stealth_amplitude(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" stealth_amplitude: ", st->pwm_ampl());
      }

      FORCE_INLINE static void set_stealth_amplitude(MKTMC* st, const uint8_t stealth_amplitude_val) {
        st->pwm_ampl(stealth_amplitude_val);
      }

      FORCE_INLINE static void get_stealth_freq(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" stealth_freq: ", st->pwm_freq());
      }

      FORCE_INLINE static void set_stealth_freq(MKTMC* st, const uint8_t stealth_freq_val) {
        st->pwm_freq(stealth_freq_val);
      }

      FORCE_INLINE static void get_stealth_autoscale(MKTMC* st) {
        st->printLabel();
        SERIAL_EMV(" stealth_autoscale: ", st->pwm_autoscale());
      }

      FORCE_INLINE static void set_stealth_autoscale(MKTMC* st, const bool onoff) {
        st->pwm_autoscale(onoff);
      }

    #endif // HAS_TMCX1X0

  private: /** Private Function */

    static bool test_connection(MKTMC* st);

    #if TMC_HAS_SPI
      static void init_cs_pins();
    #endif

    static void config(MKTMC* st, const bool stealth=false);

    #if ENABLED(MONITOR_DRIVER_STATUS)

      #if HAVE_DRV(TMC2208)
        #if ENABLED(TMC_DEBUG)
          FORCE_INLINE static uint32_t get_pwm_scale(MKTMC* st) { return st->pwm_scale_sum(); }
          static uint8_t get_status_response(MKTMC* st, uint32_t drv_status); 
        #endif
      #elif HAVE_DRV(TMC2660)
        #if ENABLED(TMC_DEBUG)
          FORCE_INLINE static uint32_t get_pwm_scale(MKTMC* st) { return 0; }
          FORCE_INLINE static uint8_t get_status_response(MKTMC* st, uint32_t drv_status) { UNUSED(st); return drv_status & 0xFF; }
        #endif
      #elif HAS_TMCX1X0
        #if ENABLED(TMC_DEBUG)
          FORCE_INLINE static uint32_t get_pwm_scale(MKTMC* st) { return st->PWM_SCALE(); }
          FORCE_INLINE static uint8_t get_status_response(MKTMC* st, uint32_t drv_status) { UNUSED(drv_status); return st->status_response & 0xF; }
        #endif
      #endif

      static TMC_driver_data get_driver_data(MKTMC* st);
      static void monitor_driver(MKTMC* st, const bool need_update_error_counters, const bool need_debug_reporting);

    #endif

    #if ENABLED(TMC_DEBUG)

      #if HAVE_DRV(TMC2160) || HAVE_DRV(TMC5160)
        FORCE_INLINE static void print_vsense(MKTMC* st) {}
      #else
        FORCE_INLINE static void print_vsense(MKTMC* st) { SERIAL_STR(st->vsense() ? PSTR("1=.18") : PSTR("0=.325")); }
      #endif

      static void status(MKTMC* st, const TMCdebugEnum i);
      static void status(MKTMC* st, const TMCdebugEnum i, const float spmm);
      static void parse_type_drv_status(MKTMC* st, const TMCdrvStatusEnum i);
      static void parse_drv_status(MKTMC* st, const TMCdrvStatusEnum i);
      static void debug_loop(const TMCdebugEnum i, const bool print_x, const bool print_y, const bool print_z, const bool print_e);
      static void status_loop(const TMCdrvStatusEnum i, const bool print_x, const bool print_y, const bool print_z, const bool print_e);
      static void get_ic_registers(MKTMC* st, const TMCgetRegistersEnum i);
      static void get_registers(MKTMC* st, const TMCgetRegistersEnum i);
      static void get_registers(const TMCgetRegistersEnum i, const bool print_x, const bool print_y, const bool print_z, const bool print_e);

    #endif

};

extern TMC_Stepper tmc;

#endif // HAS_TRINAMIC
