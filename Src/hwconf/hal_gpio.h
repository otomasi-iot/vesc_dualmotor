#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================
// COMPATIBILITY LAYER: PAL → HAL GPIO MACROS
// ============================================
// Direct register macros untuk ISR motor (NO FUNCTION CALL OVERHEAD)
#define GPIO_PIN_SET_DIRECT(port, pin)    do { (port)->BSRR = (1 << (pin)); } while(0)
#define GPIO_PIN_RESET_DIRECT(port, pin)  do { (port)->BRR = (1 << (pin)); } while(0)
#define GPIO_PIN_READ_DIRECT(port, pin)   (((port)->IDR >> (pin)) & 1)

// ============================================
// HAL GPIO FUNCTION DECLARATIONS
// ============================================
void hal_gpio_init_output(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_output_od(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pullup(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pulldown(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_analog(GPIO_TypeDef *port, uint16_t pin);

void hal_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool state);
void hal_gpio_set(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_clear(GPIO_TypeDef *port, uint16_t pin);
bool hal_gpio_read(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);

// ============================================
// PAL → HAL COMPATIBILITY MACROS
// ============================================
// These macros allow ChibiOS PAL code to work with STM32 HAL GPIO
// Replace old PAL calls gradually - these ensure no compilation errors

// palSetPad(port, pin) → set GPIO high
#define palSetPad(port, pin)                hal_gpio_set(port, (1 << (pin)))

// palClearPad(port, pin) → set GPIO low
#define palClearPad(port, pin)              hal_gpio_clear(port, (1 << (pin)))

// palWritePad(port, pin, value) → write GPIO
#define palWritePad(port, pin, value)       hal_gpio_write(port, (1 << (pin)), (value) ? 1 : 0)

// palReadPad(port, pin) → read GPIO
#define palReadPad(port, pin)               hal_gpio_read(port, (1 << (pin)))

// palTogglePad(port, pin) → toggle GPIO
#define palTogglePad(port, pin)             hal_gpio_toggle(port, (1 << (pin)))

// ============================================
// PAL GPIO MODE COMPATIBILITY MAPPINGS
// ============================================
// For palSetPadMode compatibility, we provide a conversion function
// that detects the mode and configures appropriately

typedef enum {
	PAL_MODE_RESET = 0,
	PAL_MODE_UNCONNECTED = 1,
	PAL_MODE_INPUT = 2,
	PAL_MODE_INPUT_PULLUP = 3,
	PAL_MODE_INPUT_PULLDOWN = 4,
	PAL_MODE_INPUT_ANALOG = 5,
	PAL_MODE_OUTPUT_PUSHPULL = 6,
	PAL_MODE_OUTPUT_OPENDRAIN = 7,
	PAL_MODE_ALTERNATE = 8
} pal_mode_t;

// Forward declare - implementation detects mode from flags
void hal_gpio_init_pad(GPIO_TypeDef *port, uint16_t pin, uint32_t mode);

// Wrapper for palSetPadMode - detects mode from configuration value
// This is the critical compatibility function
#define palSetPadMode(port, pin, mode)  \
	do { \
		if ((mode) & PAL_MODE_ALTERNATE) { \
			hal_gpio_init_output(port, (1 << (pin))); \
		} else if ((mode) & PAL_MODE_INPUT_ANALOG) { \
			hal_gpio_init_input_analog(port, (1 << (pin))); \
		} else if ((mode) & PAL_MODE_INPUT_PULLUP) { \
			hal_gpio_init_input_pullup(port, (1 << (pin))); \
		} else if ((mode) & PAL_MODE_INPUT_PULLDOWN) { \
			hal_gpio_init_input_pulldown(port, (1 << (pin))); \
		} else if ((mode) & PAL_MODE_INPUT) { \
			hal_gpio_init_input(port, (1 << (pin))); \
		} else if ((mode) & PAL_MODE_OUTPUT_OPENDRAIN) { \
			hal_gpio_init_output_od(port, (1 << (pin))); \
		} else { \
			hal_gpio_init_output(port, (1 << (pin))); \
		} \
	} while(0)

// PAL mode constant definitions for compatibility
#define PAL_STM32_OSPEED_HIGHEST   0
#define PAL_STM32_OSPEED_MID1      1
#define PAL_STM32_OSPEED_MID2      2
#define PAL_STM32_OSPEED_LOWEST    3
#define PAL_STM32_PUDR_FLOATING    0
#define PAL_STM32_PUDR_PULLUP      1
#define PAL_STM32_PUDR_PULLDOWN    2
#define PAL_STM32_OTYPE_PUSHPULL   0
#define PAL_STM32_OTYPE_OPENDRAIN  1
#define PAL_STM32_OTYPE_RESERVED   2

// Mode combination helpers
#define PAL_MODE_ALTERNATE(af)     (PAL_MODE_ALTERNATE | (af))

#endif
