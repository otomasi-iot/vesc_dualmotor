// Hoverboard dual-motor hardware shim for building with PlatformIO (STM32F103).
// This replaces the original VESC hwconf selection mechanism for this port.
#ifndef HW_H_
#define HW_H_

#include "conf_general.h"
#include "ch.h"
#include "hw_config.h"

// Basic hardware identity (used in some comm / configuration paths)
#ifndef HW_NAME
#define HW_NAME "HOVERBOARD_DUAL_F103RC"
#endif

// Feature flags
#undef HW_HAS_DUAL_MOTORS
#define HW_HAS_DUAL_MOTORS 1

// LED control macros expected by some VESC modules
#define LED_RED_ON()      HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET)
#define LED_RED_OFF()     HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET)
#define LED_GREEN_ON()    LED_RED_ON()
#define LED_GREEN_OFF()   LED_RED_OFF()

// Hooks used in main.c (keep as no-ops for now)
#ifndef HW_TRIM_HSI
#define HW_TRIM_HSI() do {} while (0)
#endif

#ifndef HW_EARLY_INIT
#define HW_EARLY_INIT() do {} while (0)
#endif

#ifndef HW_VERY_EARLY_INIT
#define HW_VERY_EARLY_INIT() do {} while (0)
#endif

// Firmware identity
#ifndef FW_NAME
#define FW_NAME "VESC_HOVERBOARD_F103"
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

// Flash result codes
#ifndef FLASH_COMPLETE
#define FLASH_COMPLETE HAL_OK
#endif

// UART device (SerialDriver) — used by app_uartcomm
extern SerialDriver HW_UART_DEV_inst;
#define HW_UART_DEV    HW_UART_DEV_inst
#define HW_UART_GPIO_AF 0

// USB — STM32F103RCT6 hoverboard boards have no USB connector; disable
#ifndef COMM_USE_USB
#define COMM_USE_USB 0
#endif

// NTC temperature sensor stubs (no external NTC on hoverboard)
#ifndef NTC_TEMP_MOS1
#define NTC_TEMP_MOS1()      25.0f
#endif
#ifndef NTC_TEMP_MOS2
#define NTC_TEMP_MOS2()      25.0f
#endif
#ifndef NTC_TEMP_MOS3
#define NTC_TEMP_MOS3()      25.0f
#endif
#ifndef NTC_TEMP_MOS1_M2
#define NTC_TEMP_MOS1_M2()   25.0f
#endif
#ifndef NTC_TEMP_MOS2_M2
#define NTC_TEMP_MOS2_M2()   25.0f
#endif
#ifndef NTC_TEMP_MOS3_M2
#define NTC_TEMP_MOS3_M2()   25.0f
#endif

// HW limits — commands.c uses hw_lim_upper(HW_LIM_FOC_CTRL_LOOP_FREQ)
// which expects two args (low, high). Provide as a comma-separated pair.
#ifndef HW_LIM_FOC_CTRL_LOOP_FREQ
#define HW_LIM_FOC_CTRL_LOOP_FREQ  5000.0, 50000.0
#endif

// Current calibration defaults
#ifndef CURRENT_SHUNT_RES
#define CURRENT_SHUNT_RES   RSHUNT
#endif
#ifndef CURRENT_AMP_GAIN
#define CURRENT_AMP_GAIN    AMPLIFICATION_GAIN
#endif
#ifndef V_REG
#define V_REG               ADC_REFERENCE_VOLTAGE
#endif
#ifndef VIN_R1
#define VIN_R1              33000.0
#endif
#ifndef VIN_R2
#define VIN_R2              2200.0
#endif
#ifndef CURRENT_CAL1
#define CURRENT_CAL1        1.0
#endif
#ifndef CURRENT_CAL2
#define CURRENT_CAL2        1.0
#endif
#ifndef CURRENT_CAL3
#define CURRENT_CAL3        1.0
#endif
#ifndef CURRENT_CAL1_M2
#define CURRENT_CAL1_M2     1.0
#endif
#ifndef CURRENT_CAL2_M2
#define CURRENT_CAL2_M2     1.0
#endif
#ifndef CURRENT_CAL3_M2
#define CURRENT_CAL3_M2     1.0
#endif

// DRV8313 stubs
#ifndef HW_HAS_DRV8313
#define HW_HAS_DRV8313  0
#endif
#undef INIT_BR
#define INIT_BR()    do {} while(0)
#undef DISABLE_BR
#define DISABLE_BR() do {} while(0)
#undef ENABLE_BR_2
#define ENABLE_BR_2() do {} while(0)

// LED PWM indices
#ifndef LED_GREEN
#define LED_GREEN 0
#endif
#ifndef LED_RED
#define LED_RED   1
#endif

// gsvesc stub (used by app_dpv)
static inline float gsvesc_get_angle(void) { return 0.0f; }

#endif

