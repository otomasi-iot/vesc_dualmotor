// Minimal ChibiOS "hal.h" compatibility shim.
// Avoid adding new dependencies on this header; migrate code to STM32 HAL directly.
#ifndef HAL_H_
#define HAL_H_

#include "stm32f1xx_hal.h"

static inline void halInit(void) {
	// ChibiOS halInit() equivalent in STM32 HAL world.
	HAL_Init();
}

#endif

