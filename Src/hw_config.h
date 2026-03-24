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
 * HARDWARE FEATURES - Dual Motor Hoverboard
 *============================================================================*/
#ifndef HW_HAS_DUAL_MOTORS
#define HW_HAS_DUAL_MOTORS              // Enable dual motor support (TIM1 + TIM8)
#endif
#ifndef HW_NAME
#define HW_NAME                         "VESC_F103_HOVERBOARD_DUAL"
#endif

#ifndef HW_DEFAULT_ID
#define HW_DEFAULT_ID                   0
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

/*============================================================================
 * ADC CHANNEL CONFIGURATION
 *============================================================================*/

// ADC channels for regular conversions (voltage monitoring)
#define HW_ADC_CHANNELS         6
#define HW_ADC_CHANNELS_EXTRA   0
#define HW_ADC_NBR_CONV         6

// ADC channels for injected conversions (current sensing)
#define HW_ADC_INJ_CHANNELS     2

// ADC channel indices (regular conversions)
#define ADC_IND_SENS1           0
#define ADC_IND_SENS2           1
#define ADC_IND_SENS3           2
#define ADC_IND_VIN_SENS        3
#define ADC_IND_EXT             4
#define ADC_IND_TEMP_MOS        5
#define ADC_IND_TEMP_MOS_M2     5  // Motor 2 MOS temp (same channel for F103)
#define ADC_IND_TEMP_MOTOR      5  // Same as TEMP_MOS for F103
#define ADC_IND_TEMP_MOTOR_2    5  // Motor 2 temp (same channel for F103)

// Current sensing ADC channels (injected) - MUST be defined early for macros
#define ADC_IND_CURR1           0
#define ADC_IND_CURR2           1
#define ADC_IND_CURR3           2

// Motor 2 current sensing indices
#define ADC_IND_CURR1_M2        3
#define ADC_IND_CURR2_M2        4
#define ADC_IND_CURR3_M2        5

// Temperature filter constant
#define MOTOR_TEMP_LPF          0.1

// Voltage macros (mapped to ADC_Value array)
#define ADC_V_L1                ADC_Value[ADC_IND_SENS1]
#define ADC_V_L2                ADC_Value[ADC_IND_SENS2]
#define ADC_V_L3                ADC_Value[ADC_IND_SENS3]
#define ADC_V_L4                ADC_Value[ADC_IND_VIN_SENS]
#define ADC_V_L5                ADC_Value[ADC_IND_EXT]
#define ADC_V_L6                ADC_Value[ADC_IND_TEMP_MOS]
#define ADC_V_ZERO              2048  // Virtual ground for current sensing

// ADC voltage conversion constants (defined early to avoid macro expansion issues)
#define HW_ADC_VOLTS            0.0008056640625  // 3.3 / 4096.0
#define HW_ADC_VOLTS_PH_FACTOR  0.0244  // Phase voltage scaling
#define HW_ADC_VOLTS_INPUT_FACTOR  0.0732  // Input voltage scaling

// Backward compatibility aliases
#define ADC_VOLTS               HW_ADC_VOLTS
#define ADC_VOLTS_PH_FACTOR     HW_ADC_VOLTS_PH_FACTOR
#define ADC_VOLTS_INPUT_FACTOR  HW_ADC_VOLTS_INPUT_FACTOR

// Voltage conversion macros (using direct calculation to avoid macro expansion issues)
#define ADC_V_L1_VOLTS          ((float)ADC_Value[ADC_IND_SENS1] * HW_ADC_VOLTS * HW_ADC_VOLTS_PH_FACTOR)
#define ADC_V_L2_VOLTS          ((float)ADC_Value[ADC_IND_SENS2] * HW_ADC_VOLTS * HW_ADC_VOLTS_PH_FACTOR)
#define ADC_V_L3_VOLTS          ((float)ADC_Value[ADC_IND_SENS3] * HW_ADC_VOLTS * HW_ADC_VOLTS_PH_FACTOR)
#define ADC_V_L4_VOLTS          ((float)ADC_Value[ADC_IND_VIN_SENS] * HW_ADC_VOLTS * HW_ADC_VOLTS_INPUT_FACTOR)
#define ADC_V_L5_VOLTS          ((float)ADC_Value[ADC_IND_EXT] * HW_ADC_VOLTS)
#define ADC_V_L6_VOLTS          ((float)ADC_Value[ADC_IND_TEMP_MOS] * HW_ADC_VOLTS)

// External ADC_Value array (defined in motor control)
extern volatile uint16_t ADC_Value[HW_ADC_CHANNELS];

// Hardware parameters
#define HW_DEAD_TIME_NSEC           360     // Dead time in nanoseconds
#define HW_MAX_CURRENT_OFFSET       620     // Maximum current offset
#define SYSTEM_CORE_CLOCK           72000000 // 72 MHz system clock

/*============================================================================
 * TEMPERATURE SENSING MACROS
 *============================================================================*/
// Helper macro for NTC resistance calculation
#define NTC_RES(adc_val) ((4095.0f * 10000.0f) / (float)(adc_val) - 10000.0f)

// Helper macro for basic NTC temperature
#define NTC_TEMP(adc_val) (1.0f / ((logf(NTC_RES(adc_val) / 10000.0f) / 3380.0f) + (1.0f / 298.15f)) - 273.15f)

// NTC with custom beta value
#define NTC_TEMP_MOTOR(beta) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR]) / 10000.0f) / (beta)) + (1.0f / 298.15f)) - 273.15f)
#define NTC_TEMP_MOTOR_2(beta) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR_2]) / 10000.0f) / (beta)) + (1.0f / 298.15f)) - 273.15f)

// 100K NTC thermistor
#define NTC100K_TEMP_MOTOR(beta) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR]) / 100000.0f) / (beta)) + (1.0f / 298.15f)) - 273.15f)
#define NTC100K_TEMP_MOTOR_2(beta) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR_2]) / 100000.0f) / (beta)) + (1.0f / 298.15f)) - 273.15f)

// PTC temperature calculation
#define PTC_TEMP_MOTOR(res, con, temp_base) ((float)ADC_Value[ADC_IND_TEMP_MOTOR] * (res) * (con) - (temp_base))
#define PTC_TEMP_MOTOR_2(res, con, temp_base) ((float)ADC_Value[ADC_IND_TEMP_MOTOR_2] * (res) * (con) - (temp_base))

// Custom NTC/PTC with base temperature
#define NTCX_TEMP_MOTOR(res, beta, t_base) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR]) / (res)) / (beta)) + (1.0f / (273.15f + (t_base)))) - 273.15f)
#define NTCX_TEMP_MOTOR_2(res, beta, t_base) (1.0f / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOTOR_2]) / (res)) / (beta)) + (1.0f / (273.15f + (t_base)))) - 273.15f)

// NTC resistance for motor temperature sensor
#define NTC_RES_MOTOR(adc_val) NTC_RES(adc_val)

// MOS temperature (using same NTC formula)
#define NTC_TEMP_MOS1() NTC_TEMP(ADC_Value[ADC_IND_TEMP_MOS])
#define NTC_TEMP_MOS2() NTC_TEMP(ADC_Value[ADC_IND_TEMP_MOS])
#define NTC_TEMP_MOS1_M2() NTC_TEMP(ADC_Value[ADC_IND_TEMP_MOS_M2])
#define NTC_TEMP_MOS2_M2() NTC_TEMP(ADC_Value[ADC_IND_TEMP_MOS_M2])

/*============================================================================
 * HALL SENSOR READING MACROS
 *============================================================================*/
#define READ_HALL1() HAL_GPIO_ReadPin(LEFT_HALL_U_PORT, LEFT_HALL_U_PIN)
#define READ_HALL2() HAL_GPIO_ReadPin(LEFT_HALL_V_PORT, LEFT_HALL_V_PIN)
#define READ_HALL3() HAL_GPIO_ReadPin(LEFT_HALL_W_PORT, LEFT_HALL_W_PIN)

#define READ_HALL1_2() HAL_GPIO_ReadPin(RIGHT_HALL_U_PORT, RIGHT_HALL_U_PIN)
#define READ_HALL2_2() HAL_GPIO_ReadPin(RIGHT_HALL_V_PORT, RIGHT_HALL_V_PIN)
#define READ_HALL3_2() HAL_GPIO_ReadPin(RIGHT_HALL_W_PORT, RIGHT_HALL_W_PIN)

/*============================================================================
 * BRAKE RESISTOR CONTROL MACROS (for BLDC commutation)
 *============================================================================*/
#define ENABLE_BR()     do { /* Brake resistor enable - not used on hoverboard */ } while(0)
#define DISABLE_BR()    do { /* Brake resistor disable - not used on hoverboard */ } while(0)
#define ENABLE_BR1()    ENABLE_BR()
#define ENABLE_BR2()    ENABLE_BR()
#define ENABLE_BR3()    ENABLE_BR()
#define DISABLE_BR1()   DISABLE_BR()
#define DISABLE_BR2()   DISABLE_BR()
#define DISABLE_BR3()   DISABLE_BR()
#define ENABLE_BR_2()   ENABLE_BR()

/*============================================================================
 * CURRENT AND PHASE FILTER MACROS
 *============================================================================*/
#define CURRENT_FILTER_OFF()    do { /* Current filter off */ } while(0)
#define CURRENT_FILTER_OFF_M2() do { /* Current filter off motor 2 */ } while(0)
#define CURRENT_FILTER_ON()     do { /* Current filter on */ } while(0)
#define CURRENT_FILTER_ON_M2()  do { /* Current filter on motor 2 */ } while(0)
#define PHASE_FILTER_OFF()      do { /* Phase filter off */ } while(0)
#define PHASE_FILTER_OFF_M2()   do { /* Phase filter off motor 2 */ } while(0)
#define PHASE_FILTER_ON()       do { /* Phase filter on */ } while(0)
#define PHASE_FILTER_ON_M2()    do { /* Phase filter on motor 2 */ } while(0)

/*============================================================================
 * INPUT VOLTAGE READING MACRO
 *============================================================================*/
#define GET_INPUT_VOLTAGE() ((float)ADC_Value[ADC_IND_VIN_SENS] * HW_ADC_VOLTS * HW_ADC_VOLTS_INPUT_FACTOR)

/*============================================================================
 * AUXILIARY OUTPUT CONTROL
 *============================================================================*/
#define AUX_ON()  do { /* Auxiliary output on */ } while(0)
#define AUX_OFF() do { /* Auxiliary output off */ } while(0)

/*============================================================================
 * DRIVER FAULT RESET AND CONTROL
 *============================================================================*/
#define HW_RESET_DRV_FAULTS() do { /* Reset gate driver faults */ } while(0)
#define IS_DRV_FAULT()        (0)  // No DRV fault pin on hoverboard
#define IS_DRV_FAULT_2()      (0)  // No DRV fault pin motor 2

/*============================================================================
 * DC CALIBRATION AND GATE CONTROL
 *============================================================================*/
#define DCCAL_ON()            do { /* DC calibration on */ } while(0)
#define DCCAL_OFF()           do { /* DC calibration off */ } while(0)
#define ENABLE_GATE()         do { /* Enable gate drivers */ } while(0)
#define DISABLE_GATE()        do { /* Disable gate drivers */ } while(0)

/*============================================================================
 * CURRENT SENSING MACROS (ADC injection)
 *============================================================================*/
// Motor 1 current sensing
#define HW_GET_INJ_CURR1()    ((float)(ADC_Value[ADC_IND_CURR1] - 2048) * FAC_CURRENT1)
#define HW_GET_INJ_CURR2()    ((float)(ADC_Value[ADC_IND_CURR2] - 2048) * FAC_CURRENT2)
#define HW_GET_INJ_CURR3()    ((float)(ADC_Value[ADC_IND_CURR3] - 2048) * FAC_CURRENT3)
#define HW_GET_INJ_CURR1_S2() ((float)(ADC_Value[ADC_IND_CURR1] - 2048) * FAC_CURRENT1)
#define GET_CURRENT1()        ((float)(ADC_Value[ADC_IND_CURR1] - 2048) * FAC_CURRENT1)
#define GET_CURRENT2()        ((float)(ADC_Value[ADC_IND_CURR2] - 2048) * FAC_CURRENT2)
#define GET_CURRENT3()        ((float)(ADC_Value[ADC_IND_CURR3] - 2048) * FAC_CURRENT3)

// Motor 2 current sensing
#define GET_CURRENT1_M2()     ((float)(ADC_Value[ADC_IND_CURR1_M2] - 2048) * FAC_CURRENT1_M2)
#define GET_CURRENT2_M2()     ((float)(ADC_Value[ADC_IND_CURR2_M2] - 2048) * FAC_CURRENT2_M2)
#define GET_CURRENT3_M2()     ((float)(ADC_Value[ADC_IND_CURR3_M2] - 2048) * FAC_CURRENT3_M2)

// Current sensing parameters
#define RSHUNT                      0.001   // Shunt resistor value (1 mOhm)
#define AMPLIFICATION_GAIN          20.0    // Current amplifier gain
#define ADC_REFERENCE_VOLTAGE       3.3     // ADC reference voltage

// Current sensing factors (calculated from shunt and amplifier)
#define FAC_CURRENT1        (1.0f / (RSHUNT * AMPLIFICATION_GAIN * 4095.0f / ADC_REFERENCE_VOLTAGE))
#define FAC_CURRENT2        FAC_CURRENT1
#define FAC_CURRENT3        FAC_CURRENT1

// Motor 2 current sensing factors
#define FAC_CURRENT1_M2     FAC_CURRENT1
#define FAC_CURRENT2_M2     FAC_CURRENT1
#define FAC_CURRENT3_M2     FAC_CURRENT1

// UART pins for VESC communication
#define VESC_USART_BAUD    115200
#define HW_UART_TX_PORT    GPIOB
#define HW_UART_TX_PIN     GPIO_PIN_10
#define HW_UART_RX_PORT    GPIOB
#define HW_UART_RX_PIN     GPIO_PIN_11

// LED for status indication
#define LED_PORT           GPIOB
#define LED_PIN            GPIO_PIN_2

/* APP Input Macros */
#define HW_ICU_GPIO        PPM_INPUT_PORT
#define HW_ICU_PIN         PPM_INPUT_PIN
#define HW_HALL_TRIGGER_GPIO LEFT_HALL_U_PORT
#define HW_HALL_TRIGGER_PIN  LEFT_HALL_U_PIN
#undef ADC_IND_EXT
#define ADC_IND_EXT        0
#undef ADC_IND_EXT2
#define ADC_IND_EXT2       0
// MSG_OK is defined in ch.h — do not redefine here

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
#undef RSHUNT
#define RSHUNT                 0.003f   /* Ohms — typical hoverboard */
#undef AMPLIFICATION_GAIN
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
#undef ADC_REFERENCE_VOLTAGE
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
#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif
// SIGN macro is defined in utils_math.h — do not redefine here
#define SQ(x)           ((x) * (x))

/*============================================================================
 * ADC HELPER MACROS (VESC compatibility)
 * ADC_Value[] is the DMA buffer filled by the injected ADC conversions.
 * ADC_VOLTS converts a raw ADC index to voltage (3.3V / 4095).
 *============================================================================*/
extern volatile uint16_t ADC_Value[];
#undef ADC_VOLTS
#define ADC_VOLTS(ch)      ((float)ADC_Value[(ch)] / 4095.0f * 3.3f)

#ifndef ADC_IND_EXT
#define ADC_IND_EXT        0
#endif
#ifndef ADC_IND_EXT2
#define ADC_IND_EXT2       1
#endif

/*============================================================================
 * I2C STUBS (no I2C peripheral used on hoverboard, stubs for VESC compat)
 *============================================================================*/
typedef uint8_t i2caddr_t;

/* Dummy I2C device handle */
#include "stm32f1xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
#define HW_I2C_DEV         hi2c1

static inline void hw_start_i2c(void) { /* no-op */ }
static inline void hw_stop_i2c(void) { /* no-op */ }
static inline void hw_try_restore_i2c(void) { /* no-op */ }

static inline void i2cAcquireBus(void *dev) { (void)dev; }
static inline void i2cReleaseBus(void *dev) { (void)dev; }
static inline int32_t i2cMasterTransmitTimeout(void *dev, i2caddr_t addr,
    const uint8_t *txbuf, size_t txlen, uint8_t *rxbuf, size_t rxlen, uint32_t tmo) {
    (void)dev; (void)addr; (void)txbuf; (void)txlen; (void)rxbuf; (void)rxlen; (void)tmo;
    return 0; /* MSG_OK */
}
static inline int32_t i2cMasterReceiveTimeout(void *dev, i2caddr_t addr,
    uint8_t *rxbuf, size_t rxlen, uint32_t tmo) {
    (void)dev; (void)addr; (void)rxbuf; (void)rxlen; (void)tmo;
    return 0; /* MSG_OK */
}

/*============================================================================
 * CAN CONFIGURATION (stubs for F103 hoverboard - CAN not used)
 *============================================================================*/
#ifndef HW_CAN_DEV
#define HW_CAN_DEV         hcan1
#endif

#ifndef HW_CANRX_PORT
#define HW_CANRX_PORT      GPIOA
#endif

#ifndef HW_CANRX_PIN
#define HW_CANRX_PIN       GPIO_PIN_11
#endif

#ifndef HW_CANTX_PORT
#define HW_CANTX_PORT      GPIOA
#endif

#ifndef HW_CANTX_PIN
#define HW_CANTX_PIN       GPIO_PIN_12
#endif

#ifndef HW_CAN_GPIO_AF
#define HW_CAN_GPIO_AF     GPIO_AF_CAN1
#endif

#ifndef ADC_IND_EXT3
#define ADC_IND_EXT3       ADC_IND_EXT
#endif

/*============================================================================
 * EXTERNAL PERIPHERAL HANDLES (defined in hw_setup.c)
 *============================================================================*/
extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart3;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

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

/*============================================================================
 * F1 HAL NAME ALIASES for VESC motor control code
 * These are direct register-value name mappings (not a compat layer).
 * VESC code uses STM32F4 StdPeriph naming; F1 HAL uses different names
 * for the same register bit values.
 *============================================================================*/

/* Timer channel constants (VESC uses TIM_Channel_x) */
#define TIM_Channel_1               TIM_CHANNEL_1
#define TIM_Channel_2               TIM_CHANNEL_2
#define TIM_Channel_3               TIM_CHANNEL_3
#define TIM_Channel_4               TIM_CHANNEL_4

/* Timer OC mode */
#define TIM_OCMode_PWM1             TIM_OCMODE_PWM1
#define TIM_OCMode_PWM2             TIM_OCMODE_PWM2
#define TIM_OCMode_Inactive         TIM_OCMODE_INACTIVE
#define TIM_ForcedAction_InActive   TIM_OCMODE_FORCED_INACTIVE
#define TIM_ForcedAction_Active     TIM_OCMODE_FORCED_ACTIVE

/* Timer CC enable/disable */
#define TIM_CCx_Enable              TIM_CCx_ENABLE
#define TIM_CCx_Disable             TIM_CCx_DISABLE
#define TIM_CCxN_Enable             TIM_CCxN_ENABLE
#define TIM_CCxN_Disable            TIM_CCxN_DISABLE

/* Timer event source */
#define TIM_EventSource_COM         TIM_EVENTSOURCE_COM
#define TIM_EventSource_Update      TIM_EVENTSOURCE_UPDATE

/* DMA stream mapping (F4 DMA2_Stream4 → F1 DMA1_Channel1 for ADC) */
#define DMA2_Stream4                DMA1_Channel1

/* ADC IRQ (F103 uses ADC1_2_IRQn) */
#define ADC_IRQn                    ADC1_2_IRQn

/* MCCONF defaults not in conf_general.h */
#ifndef MCCONF_MAX_CURRENT_UNBALANCE
#define MCCONF_MAX_CURRENT_UNBALANCE       130.0
#endif
#ifndef MCCONF_MAX_CURRENT_UNBALANCE_RATE
#define MCCONF_MAX_CURRENT_UNBALANCE_RATE  0.3
#endif

/*--- Inline wrappers for VESC timer control functions (direct register ops) ---*/
static inline void TIM_SelectOCxM(TIM_TypeDef *TIMx, uint16_t ch, uint16_t mode) {
    if (ch == TIM_CHANNEL_1) { TIMx->CCMR1 = (TIMx->CCMR1 & ~TIM_CCMR1_OC1M) | (mode & TIM_CCMR1_OC1M); }
    else if (ch == TIM_CHANNEL_2) { TIMx->CCMR1 = (TIMx->CCMR1 & ~TIM_CCMR1_OC2M) | ((mode & TIM_CCMR1_OC1M) << 8); }
    else if (ch == TIM_CHANNEL_3) { TIMx->CCMR2 = (TIMx->CCMR2 & ~TIM_CCMR2_OC3M) | (mode & TIM_CCMR2_OC3M); }
    else if (ch == TIM_CHANNEL_4) { TIMx->CCMR2 = (TIMx->CCMR2 & ~TIM_CCMR2_OC4M) | ((mode & TIM_CCMR2_OC3M) << 8); }
}
static inline void TIM_CCxCmd(TIM_TypeDef *TIMx, uint16_t ch, uint16_t state) {
    uint32_t tmp = 0;
    if (ch == TIM_CHANNEL_1) tmp = TIM_CCER_CC1E;
    else if (ch == TIM_CHANNEL_2) tmp = TIM_CCER_CC2E;
    else if (ch == TIM_CHANNEL_3) tmp = TIM_CCER_CC3E;
    else if (ch == TIM_CHANNEL_4) tmp = TIM_CCER_CC4E;
    if (state == TIM_CCx_ENABLE) TIMx->CCER |= tmp; else TIMx->CCER &= ~tmp;
}
static inline void TIM_CCxNCmd(TIM_TypeDef *TIMx, uint16_t ch, uint16_t state) {
    uint32_t tmp = 0;
    if (ch == TIM_CHANNEL_1) tmp = TIM_CCER_CC1NE;
    else if (ch == TIM_CHANNEL_2) tmp = TIM_CCER_CC2NE;
    else if (ch == TIM_CHANNEL_3) tmp = TIM_CCER_CC3NE;
    if (state == TIM_CCxN_ENABLE) TIMx->CCER |= tmp; else TIMx->CCER &= ~tmp;
}
static inline void TIM_GenerateEvent(TIM_TypeDef *TIMx, uint16_t src) {
    TIMx->EGR = src;
}

#ifdef __cplusplus
}
#endif

#endif /* HW_CONFIG_H */
