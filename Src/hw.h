// Hoverboard dual-motor hardware shim for building with PlatformIO (STM32F103).
// This replaces the original VESC hwconf selection mechanism for this port.
#ifndef HW_H_
#define HW_H_

#include "hw_config.h"

// Basic hardware identity (used in some comm / configuration paths)
#ifndef HW_NAME
#define HW_NAME "HOVERBOARD_DUAL_F103RC"
#endif

// Feature flags
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

#endif

