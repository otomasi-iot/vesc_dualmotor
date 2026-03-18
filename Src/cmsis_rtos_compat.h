#ifndef CMSIS_RTOS_COMPAT_H
#define CMSIS_RTOS_COMPAT_H
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

// Define commonly used macros mapped from ChibiOS to CMSIS OS2 / FreeRTOS
#define chRegSetThreadName(name) osThreadSetName(osThreadGetId(), name)
#define NORM_PRIO osPriorityNormal
#define LOW_PRIO osPriorityLow
#define HIGH_PRIO osPriorityHigh

#endif
