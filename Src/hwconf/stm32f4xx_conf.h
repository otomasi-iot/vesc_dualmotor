#ifndef __STM32F4xx_CONF_H
#define __STM32F4xx_CONF_H

/*
 * Compatibility header for legacy VESC sources that still include
 * "hwconf/stm32f4xx_conf.h".
 *
 * This PlatformIO port targets STM32F103 (HAL). Do NOT include STM32F4 StdPeriph.
 * The real migration work should replace these includes in each module, but
 * keeping this header as a harmless stub unblocks incremental conversion.
 */

#include "stm32f1xx_hal.h"

#ifndef USE_RTOS
#define USE_RTOS 0
#endif

#endif
