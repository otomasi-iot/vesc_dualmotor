#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

//====== Scheduler Configuration ======
#define configUSE_PREEMPTION                    1  // Enable preemptive scheduling
#define configUSE_TIME_SLICING                  1  // Time slice equal priority tasks
#define configMAX_PRIORITIES                    (7 + 1)  // CMSIS-RTOS2: 0-7 (0=idle)
#define configMAX_TASK_NAME_LEN                 16
#define configIDLE_SHOULD_YIELD                 1

// Task count = 25 (motor: 5, CAN: 5, USB: 3, encoder: 4, IMU: 2, app: 3, utility: 3)
#define configMAX_TASKS                         30  // Add 5 extra for safety

//====== Memory Management ======
#define configSUPPORT_STATIC_ALLOCATION         1  // Use static pools
#define configSUPPORT_DYNAMIC_ALLOCATION        1  // Required by CMSIS-RTOS2 wrapper
#define configTOTAL_HEAP_SIZE                   (16 * 1024)  // Heap for RTOS objects

//====== Timer Tick Configuration ======
#define configTICK_RATE_HZ                      1000  // 1ms tick (1000 Hz)
#define configUSE_TICK_HOOK                     0    // No tick hook needed
#define configUSE_IDLE_HOOK                     1    // Watchdog/power management in idle

// Minimal stack sizes (words, not bytes)
#define configMINIMAL_STACK_SIZE                128

// Software timers (used by CMSIS-RTOS2 wrapper)
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

//====== Synchronization Primitives ======
#define configUSE_MUTEXES                       1  // Priority inheritance mutexes
#define configUSE_RECURSIVE_MUTEXES             1  // Recursive mutex support
#define configUSE_COUNTING_SEMAPHORES           1  // For resource pools
#define configUSE_TRACE_FACILITY                0  // Disable for production
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

//====== Optional API includes (required by CMSIS-RTOS2 wrapper) ======
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_xTaskAbortDelay                 1

//====== ISR Context Checking ======
#define configASSERT( x ) do { if( !( x ) ) { \
    for( ;; ); \
} } while( 0 )

//====== CMSIS-RTOS v2 Compatibility ======
#define configUSE_OS2_EVENTFLAGS_FROM_ISR       1  // osEventFlagsSet() from ISR
#define configUSE_OS2_TASK_NOTIFY_FROM_ISR      1  // Task notifications from ISR

//====== Optional Features ======
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_QUEUE_SETS                    0
#define configUSE_TASK_NOTIFICATIONS            1  // Lightweight signaling

//====== Port Specific (Cortex-M3 STM32F1) ======
#define configCPU_CLOCK_HZ                      72000000  // STM32F103 @ 72 MHz
#define configSYSTICK_USE_LOW_POWER_MODE        0
#define configUSE_16_BIT_TICKS                  0  // 32-bit tick counter

//====== Interrupt Priority Configuration (Cortex-M3) ======
// STM32F1 uses 4 priority bits by default (16 levels)
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY     5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

//====== Hook Functions ======
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2  // Method 2: high water mark

#endif
