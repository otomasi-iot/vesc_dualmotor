// Comprehensive ChibiOS API compatibility shim for STM32 HAL + FreeRTOS/CMSIS-RTOS2.
// Allows incremental migration away from ChibiOS. Core performance-critical
// files have been natively converted; remaining files use these shims.
#ifndef CH_H_
#define CH_H_

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f1xx_hal.h"
#include "chtypes.h"
#include "chsystypes.h"

// ============================================
// STM32 TYPE COMPATIBILITY
// ============================================
typedef GPIO_TypeDef stm32_gpio_t;
typedef uint32_t stkalign_t;
typedef uint32_t eventmask_t;

#ifndef STM32_UUID_8
#define STM32_UUID_8 ((const uint8_t *)0x1FFFF7E8)
#endif

// ============================================
// CH TICK / TIME HELPERS
// ============================================
#ifndef CH_CFG_ST_FREQUENCY
#define CH_CFG_ST_FREQUENCY ((uint32_t)configTICK_RATE_HZ)
#endif

#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) ((TickType_t)(((uint64_t)(ms) * configTICK_RATE_HZ) / 1000ULL))
#endif

#define MS2ST(ms)        ((systime_t)pdMS_TO_TICKS(ms))
#define ST2MS(ticks)     ((uint32_t)((uint64_t)(ticks) * 1000ULL / configTICK_RATE_HZ))
#define US2ST(usec)      ((systime_t)(((uint64_t)(usec) * configTICK_RATE_HZ) / 1000000ULL))
#define ST2S(ticks)      ((uint32_t)((uint64_t)(ticks) / configTICK_RATE_HZ))

#ifndef TIME_IMMEDIATE
#define TIME_IMMEDIATE   0
#endif

#ifndef MSG_TIMEOUT
#define MSG_TIMEOUT      (-1)
#endif

#ifndef ALL_EVENTS
#define ALL_EVENTS       ((eventmask_t)0xFFFFFFFF)
#endif

// ============================================
// PRIORITY MAPPING
// ============================================
#ifndef NORMALPRIO
#define NORMALPRIO osPriorityNormal
#endif
#ifndef HIGHPRIO
#define HIGHPRIO osPriorityHigh
#endif
#ifndef LOWPRIO
#define LOWPRIO osPriorityLow
#endif

// ============================================
// THREAD_T COMPATIBILITY
// ============================================
typedef osThreadId_t thread_t;

// ============================================
// THREAD MACROS
// ============================================
#define THD_WORKING_AREA(name, size)       uint8_t name[(size)]
#define THD_WORKING_AREA_SIZE(size)        ((size) + sizeof(StaticTask_t))
#define THD_FUNCTION(name, arg)            void *name(void *arg)

// ============================================
// THREAD CREATION: chThdCreateStatic → xTaskCreateStatic
// ============================================
// The wa (working area) is repurposed: first part becomes the StaticTask_t TCB,
// remainder becomes the stack. The priority is already mapped via NORMALPRIO etc.
static inline thread_t chThdCreateStatic(void *wa, size_t wa_size,
                                          osPriority_t prio,
                                          void *(*func)(void *), void *arg) {
    StaticTask_t *tcb = (StaticTask_t *)wa;
    StackType_t  *stack = (StackType_t *)((uint8_t *)wa + sizeof(StaticTask_t));
    uint32_t stack_words = (wa_size - sizeof(StaticTask_t)) / sizeof(StackType_t);

    // Map CMSIS priority roughly to FreeRTOS priority
    UBaseType_t fprio;
    if (prio >= osPriorityHigh) {
        fprio = configMAX_PRIORITIES - 2;
    } else if (prio >= osPriorityAboveNormal) {
        fprio = configMAX_PRIORITIES - 3;
    } else if (prio <= osPriorityBelowNormal) {
        fprio = 1;
    } else if (prio <= osPriorityLow) {
        fprio = 0;
    } else {
        fprio = (configMAX_PRIORITIES / 2);
    }

    TaskHandle_t th = xTaskCreateStatic(
        (TaskFunction_t)func, "ch_thd",
        stack_words, arg, fprio, stack, tcb);
    return (thread_t)th;
}

// ============================================
// EVENT SIGNALING (via FreeRTOS Task Notifications)
// ============================================
static inline void chEvtSignal(thread_t tp, eventmask_t mask) {
    (void)mask;
    if (tp) xTaskNotifyGive((TaskHandle_t)tp);
}

static inline void chEvtSignalI(thread_t tp, eventmask_t mask) {
    (void)mask;
    if (tp) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR((TaskHandle_t)tp, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static inline eventmask_t chEvtWaitAny(eventmask_t mask) {
    (void)mask;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return mask;
}

static inline eventmask_t chEvtWaitAnyTimeout(eventmask_t mask, systime_t timeout) {
    (void)mask;
    uint32_t ms = ST2MS(timeout);
    if (ms == 0) ms = 1;
    uint32_t got = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(ms));
    return got ? mask : 0;
}

// ============================================
// THREAD TERMINATION COMPAT
// ============================================
// ChibiOS has per-thread terminate flags. FreeRTOS doesn't have this natively.
// We use task notification bit 31 as a "terminate request" flag.
#define CH_TERMINATE_BIT (1UL << 31)

static inline void chThdTerminate(thread_t tp) {
    if (tp) {
        xTaskNotify((TaskHandle_t)tp, CH_TERMINATE_BIT,
                     eSetBits);
    }
}

static inline bool chThdShouldTerminateX(void) {
    uint32_t val = 0;
    xTaskNotifyWait(0, CH_TERMINATE_BIT, &val, 0);
    return (val & CH_TERMINATE_BIT) != 0;
}

static inline msg_t chThdWait(thread_t tp) {
    (void)tp;
    // FreeRTOS does not support join on statically created tasks.
    // Best-effort: short delay to let thread exit.
    osDelay(10);
    return 0;
}

// ============================================
// THREAD HELPERS
// ============================================
static inline thread_t chThdGetSelfX(void) {
    return osThreadGetId();
}

static inline void chThdSleepMilliseconds(uint32_t ms) {
    osDelay(ms);
}

static inline void chThdSleep(systime_t ticks) {
    uint32_t ms = ST2MS(ticks);
    if (ms == 0) ms = 1;
    osDelay(ms);
}

static inline void chThdSleepMicroseconds(uint32_t us) {
    uint32_t ms = us / 1000;
    if (ms == 0) ms = 1;
    osDelay(ms);
}

// ============================================
// MUTEX COMPATIBILITY
// ============================================
typedef osMutexId_t mutex_t;

static inline void chMtxObjectInit(mutex_t *mtx) {
    if (mtx && !*mtx) {
        const osMutexAttr_t attr = {.attr_bits = osMutexPrioInherit};
        *mtx = osMutexNew(&attr);
    }
}

static inline void chMtxLock(mutex_t *mtx) {
    (void)osMutexAcquire(mtx ? *mtx : NULL, osWaitForever);
}

static inline void chMtxUnlock(mutex_t *mtx) {
    (void)osMutexRelease(mtx ? *mtx : NULL);
}

// ============================================
// VIRTUAL TIMER COMPATIBILITY
// ============================================
typedef void (*vtfunc_t)(void *p);

typedef struct {
    osTimerId_t timer;
    vtfunc_t cb;
    void *arg;
} virtual_timer_t;

static inline void _ch_vt_trampoline(void *argument) {
    virtual_timer_t *vt = (virtual_timer_t *)argument;
    if (vt && vt->cb) {
        vt->cb(vt->arg);
    }
}

static inline void chVTObjectInit(virtual_timer_t *vt) {
    if (!vt) return;
    if (!vt->timer) {
        vt->cb = NULL;
        vt->arg = NULL;
        vt->timer = osTimerNew(_ch_vt_trampoline, osTimerOnce, vt, NULL);
    }
}

static inline void chVTReset(virtual_timer_t *vt) {
    if (vt && vt->timer) {
        (void)osTimerStop(vt->timer);
    }
}

static inline void chVTSet(virtual_timer_t *vt, systime_t ticks, vtfunc_t cb, void *p) {
    if (!vt) return;
    if (!vt->timer) chVTObjectInit(vt);
    if (vt->timer) {
        vt->cb = cb;
        vt->arg = p;
        uint32_t ms = ST2MS(ticks);
        if (ms == 0) ms = 1;
        (void)osTimerStart(vt->timer, ms);
    }
}

// ============================================
// SYSTEM TIME
// ============================================
static inline systime_t chVTGetSystemTimeX(void) {
    return (systime_t)xTaskGetTickCount();
}

static inline systime_t chVTTimeElapsedSinceX(systime_t t) {
    return (systime_t)(xTaskGetTickCount() - t);
}

// ============================================
// CRITICAL SECTION
// ============================================
static inline void chSysLock(void) {
    taskENTER_CRITICAL();
}

static inline void chSysUnlock(void) {
    taskEXIT_CRITICAL();
}

static inline void chSysLockFromISR(void) {
    /* ISR context on Cortex-M — no additional lock needed */
}

static inline void chSysUnlockFromISR(void) {
    /* ISR context on Cortex-M — no additional unlock needed */
}

// ============================================
// GPIO AF COMPATIBILITY (F1 has no AF numbers)
// ============================================
#ifndef GPIO_AF_TIM1
#define GPIO_AF_TIM1 0
#endif
#ifndef GPIO_AF_TIM8
#define GPIO_AF_TIM8 0
#endif
#ifndef GPIO_AF_CAN1
#define GPIO_AF_CAN1 0
#endif
#ifndef GPIO_AF_USART1
#define GPIO_AF_USART1 0
#endif
#ifndef GPIO_AF_USART3
#define GPIO_AF_USART3 0
#endif
#ifndef GPIO_AF_SPI1
#define GPIO_AF_SPI1 0
#endif
#ifndef GPIO_AF_SPI2
#define GPIO_AF_SPI2 0
#endif
#ifndef GPIO_AF_I2C1
#define GPIO_AF_I2C1 0
#endif

// ============================================
// NVIC HELPERS
// ============================================
#define nvicEnableVector(irq, prio) do { HAL_NVIC_SetPriority((irq), (prio), 0); HAL_NVIC_EnableIRQ((irq)); } while(0)
#define nvicDisableVector(irq) HAL_NVIC_DisableIRQ(irq)

// ============================================
// DMA STREAM HELPERS (no-op stubs for F103)
// ============================================
#define dmaStreamAllocate(...) 0
#define dmaStreamRelease(...)
#define STM32_DMA_STREAM(id) (id)
#define STM32_DMA_STREAM_ID(controller, stream) ((controller)*8 + (stream))
typedef void (*stm32_dmaisr_t)(void *, uint32_t);

// ============================================
// SERIAL DRIVER COMPAT (minimal stubs)
// ============================================
typedef struct {
    UART_HandleTypeDef *huart;
} SerialDriver;

typedef struct {
    uint32_t speed;
    uint32_t cr1;
    uint32_t cr2;
    uint32_t cr3;
} SerialConfig;

// Serial driver stubs - these need actual implementations for boards that use them
#define sdStart(sd, cfg)     ((void)0)
#define sdStop(sd)           ((void)0)

static inline size_t sdWriteTimeout(SerialDriver *sd, const uint8_t *bp, size_t n, systime_t timeout) {
    (void)sd; (void)bp; (void)timeout;
    return n; // stub
}

static inline size_t sdReadTimeout(SerialDriver *sd, uint8_t *bp, size_t n, systime_t timeout) {
    (void)sd; (void)bp; (void)timeout;
    return 0; // stub
}

static inline int sdGetTimeout(SerialDriver *sd, systime_t timeout) {
    (void)sd; (void)timeout;
    return MSG_TIMEOUT;
}

#endif /* CH_H_ */


