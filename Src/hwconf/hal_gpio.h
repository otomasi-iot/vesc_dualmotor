#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Direct register macros untuk ISR motor (NO FUNCTION CALL OVERHEAD)
#define GPIO_PIN_SET_DIRECT(port, pin)    do { (port)->BSRR = (1 << (pin)); } while(0)
#define GPIO_PIN_RESET_DIRECT(port, pin)  do { (port)->BRR = (1 << (pin)); } while(0)
#define GPIO_PIN_READ_DIRECT(port, pin)   (((port)->IDR >> (pin)) & 1)

// Standard HAL function calls untuk non-ISR
void hal_gpio_init_output(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_output_od(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pullup(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pulldown(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_analog(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af);

void hal_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool state);
bool hal_gpio_read(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_set(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_clear(GPIO_TypeDef *port, uint16_t pin);

#endif
