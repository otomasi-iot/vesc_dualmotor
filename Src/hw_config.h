/**
  ******************************************************************************
  * @file    hw_config.h
  * @brief   Hardware configuration for Hoverboard ESC dual motor controller
  *          Pin mapping, motor parameters, ADC channels, timer assignments.
  ******************************************************************************
  */

#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * PIN MAPPING — Based on the standard hoverboard mainboard
 * (STM32F103RCT6, TIM1=Right motor, TIM8=Left motor)
 *============================================================================*/

/* ---- Hall Sensors ---- */
#define LEFT_HALL_U_PIN    GPIO_PIN_5
#define LEFT_HALL_V_PIN    GPIO_PIN_6
#define LEFT_HALL_W_PIN    GPIO_PIN_7
#define LEFT_HALL_U_PORT   GPIOB
#define LEFT_HALL_V_PORT   GPIOB
#define LEFT_HALL_W_PORT   GPIOB

#define RIGHT_HALL_U_PIN   GPIO_PIN_10
#define RIGHT_HALL_V_PIN   GPIO_PIN_11
#define RIGHT_HALL_W_PIN   GPIO_PIN_12
#define RIGHT_HALL_U_PORT  GPIOC
#define RIGHT_HALL_V_PORT  GPIOC
#define RIGHT_HALL_W_PORT  GPIOC

/* ---- PWM Motor — Right (TIM1) ---- */
#define RIGHT_TIM          TIM1
#define RIGHT_TIM_UH_PIN   GPIO_PIN_8
#define RIGHT_TIM_UH_PORT  GPIOA
#define RIGHT_TIM_UL_PIN   GPIO_PIN_13
#define RIGHT_TIM_UL_PORT  GPIOB
#define RIGHT_TIM_VH_PIN   GPIO_PIN_9
#define RIGHT_TIM_VH_PORT  GPIOA
#define RIGHT_TIM_VL_PIN   GPIO_PIN_14
#define RIGHT_TIM_VL_PORT  GPIOB
#define RIGHT_TIM_WH_PIN   GPIO_PIN_10
#define RIGHT_TIM_WH_PORT  GPIOA
#define RIGHT_TIM_WL_PIN   GPIO_PIN_15
#define RIGHT_TIM_WL_PORT  GPIOB

/* ---- PWM Motor — Left (TIM8) ---- */
#define LEFT_TIM           TIM8
#define LEFT_TIM_UH_PIN    GPIO_PIN_6
#define LEFT_TIM_UH_PORT   GPIOC
#define LEFT_TIM_UL_PIN    GPIO_PIN_7
#define LEFT_TIM_UL_PORT   GPIOA
#define LEFT_TIM_VH_PIN    GPIO_PIN_7
#define LEFT_TIM_VH_PORT   GPIOC
#define LEFT_TIM_VL_PIN    GPIO_PIN_0
#define LEFT_TIM_VL_PORT   GPIOB
#define LEFT_TIM_WH_PIN    GPIO_PIN_8
#define LEFT_TIM_WH_PORT   GPIOC
#define LEFT_TIM_WL_PIN    GPIO_PIN_1
#define LEFT_TIM_WL_PORT   GPIOB

/* ---- Current Sensing (ADC) ---- */
#define RIGHT_DC_CUR_PIN   GPIO_PIN_1
#define RIGHT_DC_CUR_PORT  GPIOC
#define RIGHT_U_CUR_PIN    GPIO_PIN_4
#define RIGHT_U_CUR_PORT   GPIOC
#define RIGHT_V_CUR_PIN    GPIO_PIN_5
#define RIGHT_V_CUR_PORT   GPIOC

#define LEFT_DC_CUR_PIN    GPIO_PIN_0
#define LEFT_DC_CUR_PORT   GPIOC
#define LEFT_U_CUR_PIN     GPIO_PIN_0
#define LEFT_U_CUR_PORT    GPIOA
#define LEFT_V_CUR_PIN     GPIO_PIN_3
#define LEFT_V_CUR_PORT    GPIOC

/* ---- Battery Voltage ---- */
#define DCLINK_PIN         GPIO_PIN_2
#define DCLINK_PORT        GPIOC

/* ---- Control & Status ---- */
#define LED_PIN            GPIO_PIN_2
#define LED_PORT           GPIOB
#define BUZZER_PIN         GPIO_PIN_4
#define BUZZER_PORT        GPIOA
#define OFF_PIN            GPIO_PIN_5
#define OFF_PORT           GPIOA
#define BUTTON_PIN         GPIO_PIN_1
#define BUTTON_PORT        GPIOA
#define CHARGER_PIN        GPIO_PIN_12
#define CHARGER_PORT       GPIOA

/* ---- USART3 — VESC Communication (PB10=TX, PB11=RX) ---- */
#define VESC_USART         USART3
#define VESC_USART_BAUD    115200

/*============================================================================
 * MOTOR CONTROL PARAMETERS
 *============================================================================*/

/* Motor */
#define POLE_PAIR_NUM          15
#define HALL_PHASE_SHIFT       90
#define HALL_SENSORS_PLACEMENT 120  /* degrees between Hall sensors */
#define MAX_MOTOR_CURRENT_A    4.0f
#define MAX_INPUT_CURRENT_A    4.0f
#define MAX_BRAKE_CURRENT_A    4.0f
#define MAX_ERPM_COMMAND       25000.0f
#define POSITION_KP_ERPM_PER_DEG 80.0f
#define POSITION_MAX_ERPM      5000.0f

/* Reference FOC parameters adopted from smartesc/VESC defaults for F103 hoverboard */
#define FOC_MOTOR_L_H          0.0001f
#define FOC_MOTOR_R_OHM        0.1f
#define FOC_MOTOR_FLUX_LINKAGE 0.012f
#define FOC_OBSERVER_GAIN      2000.0f

/* PWM */
#define PWM_FREQUENCY          16000   /* Hz */
#define PWM_FREQ_SCALING       1
#define DEAD_TIME_NS           800     /* ns */
#define REGULATION_EXECUTION_RATE 1    /* FOC every PWM cycle */

/* Deadtime compensation (VESC-style):
   Compensates for FET switching dead-time distortion.
   DT_COMP_COUNTS = DEAD_TIME_NS * (SYSCLK_FREQ/1e9)   */
#define DT_COMP_COUNTS         ((uint16_t)(DEAD_TIME_NS * (SYSCLK_FREQ / 1000000UL) / 1000))
/* Current threshold (in ADC counts) below which we skip DT compensation
   to avoid noise amplification near zero crossing */
#define DT_COMP_THRESHOLD      100

/* Clock */
#define SYSCLK_FREQ            64000000UL
#define ADV_TIM_CLK_MHz        64
#define TIM_CLOCK_DIVIDER      1
#define ADC_CLK_MHz            (ADV_TIM_CLK_MHz / 6)  /* ~10.67 MHz */

/* Current sensing */
#define RSHUNT                 0.003f   /* Ohms — typical hoverboard */
#define AMPLIFICATION_GAIN     10.0f
#define NOMINAL_CURRENT        4000     /* mA peak */

/* Bus voltage */
#define VBUS_PARTITIONING_FACTOR  0.0535f
#define NOMINAL_BUS_VOLTAGE_V    36
#define OV_VOLTAGE_THRESHOLD_V   42
#define UD_VOLTAGE_THRESHOLD_V   10

/* Temperature */
#define V0_V                   1.650f
#define T0_C                   25
#define dV_dT                  0.031f
#define OV_TEMPERATURE_THRESHOLD_C  70
#define OV_TEMPERATURE_HYSTERESIS_C 5

/* PID - Torque/Flux */
#define PID_TORQUE_KP_DEFAULT  500
#define PID_TORQUE_KI_DEFAULT  300
#define PID_TORQUE_KD_DEFAULT  100
#define PID_FLUX_KP_DEFAULT    500
#define PID_FLUX_KI_DEFAULT    300
#define PID_FLUX_KD_DEFAULT    100
#define TF_KPDIV               1024
#define TF_KIDIV               16384
#define TF_KDDIV               8192
#define TF_KPDIV_POW2          10    /* 2^10 = 1024 */
#define TF_KIDIV_POW2          14    /* 2^14 = 16384 */
#define TF_KDDIV_POW2          13    /* 2^13 = 8192 */

/* FOC current loop PI (float, VESC foc_current_kp / foc_current_ki) */
#define FOC_CURRENT_KP_DEFAULT 0.030f
#define FOC_CURRENT_KI_DEFAULT 40.0f

/* PID - Speed */
#define SPEED_LOOP_FREQUENCY_HZ 1000
#define PID_SPEED_KP_DEFAULT   400
#define PID_SPEED_KI_DEFAULT   50
#define PID_SPEED_KD_DEFAULT   0
#define SP_KPDIV               16
#define SP_KIDIV               256
#define SP_KDDIV               16
#define SP_KPDIV_POW2          4     /* 2^4 = 16 */
#define SP_KIDIV_POW2          8     /* 2^8 = 256 */
#define SP_KDDIV_POW2          4     /* 2^4 = 16 */
#define IQMAX                  4766

/* Speed PID float defaults (VESC s_pid_*) */
#define S_PID_KP_DEFAULT       0.004f
#define S_PID_KI_DEFAULT       0.004f
#define S_PID_KD_DEFAULT       0.0f
#define S_PID_RAMP_ERPMS_S_DEFAULT 5000.0f

/* Position PID float defaults (VESC p_pid_*) */
#define P_PID_KP_DEFAULT       0.0020f
#define P_PID_KI_DEFAULT       0.0f
#define P_PID_KD_DEFAULT       0.0f

/* Noise parameters */
#define TNOISE_NS              2550
#define TRISE_NS               2550
#define MAX_TNTR_NS            TRISE_NS

/* Default mode */
#define DEFAULT_CONTROL_MODE   0  /* 0 = torque mode */

/* Misc conversion */
#define ADC_REFERENCE_VOLTAGE  3.30f

/* Flux weakening */
#define FW_VOLTAGE_REF         985
#define FW_KP_GAIN             2000
#define FW_KI_GAIN             5000
#define FW_KPDIV               32768
#define FW_KIDIV               32768
#define FW_KPDIV_POW2          15   /* 2^15 = 32768 */
#define FW_KIDIV_POW2          15   /* 2^15 = 32768 */
#define FW_MAX_DEMAGCURRENT    (-IQMAX)
#define FW_VQD_LP_SHIFT        4    /* Low-pass filter shift: α=1/16 */

/*============================================================================
 * BEMF DECOUPLING (VESC-inspired cross-coupling compensation)
 * Vd_corr = -ω_el * L * Iq
 * Vq_corr = +ω_el * L * Id + ω_el * Ψ
 *
 * Integer scaling (avgSpeed in 0.1 Hz mech, Iqd in Q15 rel. to NOMINAL_CURRENT,
 * result in halfPwmPeriod counts, Vbus=36 V, PP=15, hp=2000):
 *   L-term:   (-elSpeed * BEMF_L_SCALE * iq_or_id) >> BEMF_L_SHIFT
 *             BEMF_L_SCALE=728, BEMF_L_SHIFT=27 → ≈−ωₑ·L·I in hp counts
 *   Ψ-term:   (+elSpeed * BEMF_PSI_SCALE) >> BEMF_PSI_SHIFT
 *             BEMF_PSI_SCALE=201, BEMF_PSI_SHIFT=5 → ≈+ωₑ·Ψ in hp counts
 * NOTE: L-term multiplication must use int64 to avoid int32 overflow.
 *============================================================================*/
#define BEMF_L_SCALE           728   /* L-based coefficient (see note above) */
#define BEMF_L_SHIFT           27    /* right-shift for L-term result */
#define BEMF_PSI_SCALE         201   /* Ψ-based coefficient (was 273 — wrong) */
#define BEMF_PSI_SHIFT         5     /* right-shift for Ψ-term result */

/*============================================================================
 * SPEED / CURRENT RAMP (VESC-style)
 *============================================================================*/
#define SPEED_RAMP_ERPMS_S     5000.0f  /* ERPM/s acceleration ramp */
#define CURRENT_RAMP_A_S       20.0f    /* A/s current ramp */

/*============================================================================
 * ENERGY TRACKING (VESC-style)
 * Both constants equal 1/3,600,000 — the number of Ampere-hours (or Watt-hours)
 * accumulated per Ampere (or Watt) per millisecond.
 * Usage: ampHours += current_A * AH_PER_A_PER_MS  (1 kHz task, dt = 1 ms implicit)
 *============================================================================*/
#define AH_PER_A_PER_MS        (1.0f / 3600000.0f)
#define WH_PER_W_PER_MS        (1.0f / 3600000.0f)

/*============================================================================
 * TACHOMETER / ODOMETER (VESC-style)
 *============================================================================*/
#define TACH_PER_ELEC_REV      6
#define WHEEL_DIAMETER_M       0.085f
#define GEAR_RATIO             1.0f
#define TACHO_SCALE            (WHEEL_DIAMETER_M * 3.14159265f / \
                                (float)(TACH_PER_ELEC_REV * POLE_PAIR_NUM * GEAR_RATIO))

/*============================================================================
 * BATTERY (VESC-style si_battery_*)
 *============================================================================*/
#define SI_MOTOR_POLES         (POLE_PAIR_NUM * 2)
#define SI_BATTERY_CELLS       10
#define SI_BATTERY_AH          4.5f
#define BATT_CELL_FULL_V       4.20f
#define BATT_CELL_EMPTY_V      3.20f
#define BAT_CUT_START_V        32.0f
#define BAT_CUT_END_V          28.0f

/*============================================================================
 * THERMAL DERATING
 *============================================================================*/
#define TEMP_CUR_START_C       55.0f   /* Start current derating */
#define TEMP_CUR_END_C         70.0f   /* Full current derating */

/* Motor resistance temperature coefficient (copper): ~0.393%/°C */
#define MOTOR_R_TEMP_COEFF     0.00393f
/* Reference temperature at which FOC_MOTOR_R_OHM was measured */
#define MOTOR_R_REF_TEMP_C     25.0f

/*============================================================================
 * MOTOR DETECTION PARAMETERS (VESC-style)
 *============================================================================*/
#define DETECT_CURRENT_A       2.0f    /* Detection test current */
#define DETECT_DUTY             0.05f   /* Detection duty cycle */
#define DETECT_SAMPLES         200      /* Samples for averaging */
#define DETECT_SETTLE_MS       100      /* Settle time ms */

/*============================================================================
 * BLDC TRAPEZOIDAL PARAMETERS
 *============================================================================*/
#define BLDC_DUTY_MAX          0.95f
#define BLDC_DUTY_MIN          0.02f

/*============================================================================
 * ADC / PPM INPUT
 *============================================================================*/
#define ADC_INPUT_PIN          GPIO_PIN_3  /* PA3 = ADC12_IN3 (throttle) */
#define ADC_INPUT_PORT         GPIOA
#define ADC_INPUT_MIN          1000        /* ADC raw min (released) */
#define ADC_INPUT_MAX          3000        /* ADC raw max (full throttle) */
#define ADC_INPUT_CENTER       2048        /* ADC center point */
#define ADC_INPUT_DEADZONE     100         /* Deadzone around center */
#define ADC_THROTTLE_CHANNEL   ADC_CHANNEL_3   /* PA3 = ADC12_IN3 */
#define PPM_INPUT_PIN          GPIO_PIN_3  /* PB3 */
#define PPM_INPUT_PORT         GPIOB

/*============================================================================
 * FLASH / EEPROM EMULATION
 * STM32F103RC: Flash pages 125-127 (1KB each) used for EEPROM emulation
 *   Page 127 (0x0803F800) — MC configuration
 *   Page 126 (0x0803F400) — App configuration
 *============================================================================*/
#define FLASH_CONFIG_PAGE_ADDR  0x0803F800UL  /* Page 127 (last page) — MC config */
#define FLASH_APP_PAGE_ADDR     0x0803F400UL  /* Page 126 — App config */
#define FLASH_CONFIG_PAGE_SIZE  1024
#define FLASH_CONFIG_MAGIC      0xDEADBEEFUL
#define FLASH_APP_MAGIC         0xCAFEBABEUL

/*============================================================================
 * TIMEOUT / WATCHDOG
 *============================================================================*/
#define TIMEOUT_MS              1000    /* Motor stop timeout (ms) */
#define TIMEOUT_BRAKE_CURRENT   0.0f    /* Brake current on timeout */
#define IWDG_RELOAD_VALUE       140     /* IWDG reload (~12ms at LSI 40kHz) */

/*============================================================================
 * BOOTLOADER / FIRMWARE UPDATE
 *============================================================================*/
#define BOOTLOADER_ADDR         0x08000000UL
#define SYSTEM_MEMORY_ADDR      0x1FFFF000UL
/* New firmware area for UART firmware update.
   STM32F103RC has 256KB flash (0x08000000 - 0x0803FFFF).
   Main app:  0x08000000 - 0x0800FFFF (64KB reserved, actual size varies)
   New app:   0x08010000 - 0x0803F7FF (~190KB available)
   Config:    0x0803F800 - 0x0803FBFF (last page, EEPROM emulation)  */
#define NEW_APP_BASE_ADDR       0x08010000UL
#define NEW_APP_MAX_SIZE        (0x0803F800UL - NEW_APP_BASE_ADDR)
#define FLASH_PAGE_SIZE_F103    1024

/* Differential Drive (robotics) */
#define DIFF_DRIVE_WHEEL_DIST   0.52f   /* Wheel-to-wheel distance (m) */
#define DIFF_DRIVE_WHEEL_DIAM   0.165f  /* Wheel diameter (m) */

/*============================================================================
 * UTILITY MACROS
 *============================================================================*/
#define ABS(a)          (((a) < 0) ? -(a) : (a))
#define CLAMP(x, lo, hi) (((x) > (hi)) ? (hi) : (((x) < (lo)) ? (lo) : (x)))
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#define SIGN(x)         (((x) < 0) ? -1 : 1)
#define SQ(x)           ((x) * (x))

/*============================================================================
 * FUNCTION PROTOTYPES  (hw_setup.c)
 *============================================================================*/
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM8_Init(void);
void MX_TIM4_Init(void);
void MX_ADC1_Init(void);
void MX_ADC2_Init(void);
void MX_USART3_Init(void);
void MX_NVIC_Init(void);
void MX_IWDG_Init(void);

/* Timer start helper */
void startTimers(void);

#ifdef __cplusplus
}
#endif

#endif /* HW_CONFIG_H */
