---
name: RTOS Thread Architecture Agent
description: |
  Specialized agent for converting ChibiOS thread model to FreeRTOS CMSIS v2.
  Use when: Converting chThdCreateStatic(), chMtxLock(), chEvtWaitAny() to FreeRTOS equivalents.
  Handles: Task creation, thread synchronization, mutexes, event signaling, task priorities.
  Scope: All task spawning in main.c, synchronization in comm/, motor/, applications/.
---

# FreeRTOS CMSIS v2 Thread Architecture Conversion

## Mission
Convert ChibiOS thread creation, synchronization primitives (mutexes, semaphores, events), and scheduling to FreeRTOS CMSIS v2 API while maintaining:
- Original task priority hierarchy
- Deterministic task startup order
- Lock-free communication where possible
- ISR-safe primitives (~25 concurrent tasks)

## Architecture Overview

### ChibiOS → FreeRTOS CMSIS v2 Mapping

| ChibiOS | FreeRTOS CMSIS v2 | Notes |
|---------|-------------------|-------|
| `chThdCreateStatic()` | `osThreadNew()` | Creates task from static pool |
| `chThdWait()` | `osThreadJoin()` | Wait for task termination |
| `chMtxLock()` | `osMutexAcquire()` | Binary mutex with timeout |
| `chSemWait()` | `osSemaphoreAcquire()` | Counting semaphore control |
| `chEvtWaitAny()` | `osEventFlagsWait()` | Bitmask event signaling |
| `CH_CFG_NUM_THREADS` | `configMAX_TASKS` | in FreeRTOSConfig.h |
| Thread priority (0-255) | Task priority (0-56) | CMSIS-RTOS2 with 7 levels by default |

---

## Phase 1: FreeRTOS Kernel Configuration

### 1.1 Create FreeRTOSConfig.h

**Location**: `Src/FreeRTOSConfig.h`

**Core settings** (optimized for VESC dual motor):
```c
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
#define configSUPPORT_DYNAMIC_ALLOCATION        0  // Disable dynamic malloc (safety)
#define configTOTAL_HEAP_SIZE                   (16 * 1024)  // 16KB - only for OS structs

//====== Timer Tick Configuration ======
#define configTICK_RATE_HZ                      1000  // 1ms tick (1000 Hz)
#define configUSE_TICK_HOOK                     0    // No tick hook needed
#define configUSE_IDLE_HOOK                     1    // Watchdog/power management in idle

//====== Synchronization Primitives ======
#define configUSE_MUTEXES                       1  // Priority inheritance mutexes
#define configUSE_RECURSIVE_MUTEXES             1  // Recursive mutex support
#define configUSE_COUNTING_SEMAPHORES           1  // For resource pools
#define configUSE_TRACE_FACILITY                0  // Disable for production
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

//====== ISR Context Checking ======
#define configASSERT( x ) do { if( !( x ) ) { \
    taskDISABLE_INTERRUPTS(); \
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

//====== Hook Functions ======
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2  // Method 2: high water mark

#endif
```

### 1.2 Timing Constraints Analysis

**ChibiOS vs FreeRTOS Overhead** (STM32F1 @ 72 MHz):

| Operation | ChibiOS | FreeRTOS | Impact |
|-----------|---------|----------|--------|
| Context switch | ~0.5 µs | ~1.0 µs | Motor ISR ±0.5 µs jitter (acceptable) |
| Mutex lock (uncontended) | ~0.1 µs | ~0.2 µs | Negligible for non-ISR code |
| Event signal from ISR | ~0.3 µs | ~0.4 µs | Acceptable for <6 kHz ISR |
| Task wake latency | ~0.8 µs | ~1.2 µs | Synchronization ISRs unaffected |

**Critical motor ISR preserved**:
- ADC1_2_3_IRQHandler: Raw handler (not task-based) - **untouched**
- TIM2_IRQHandler: Raw handler - **untouched**
- Both execute in ISR context, no FreeRTOS overhead

---

## Phase 2: FreeRTOS CMSIS v2 Setup (NO WRAPPERS - DIRECT REPLACEMENT)

**PENTING**: Tidak ada wrapper atau abstraksi. Semua API ChibiOS akan diganti langsung dengan FreeRTOS API dalam setiap file.

---

## Phase 3: Task Creation & Initialization

### 3.1 Main Task Startup (main.c)

**New pattern** - Replace ChibiOS chThdCreateStatic calls:

```c
// Src/main.c - Updated for FreeRTOS

#include "cmsis_os2.h"
#include "cmsis_rtos_compat.h"

/* ===== Static Task Buffers ===== */
// Motor control tasks
static StaticTask_t motor_ctrl_task_buffer;
static StackType_t motor_ctrl_stack[512];

static StaticTask_t foc_observer_task_buffer;
static StackType_t foc_observer_stack[256];

// CAN communication tasks
static StaticTask_t can_rx_task_buffer[5];
static StackType_t can_rx_stack[5][512];

static StaticTask_t can_tx_task_buffer;
static StackType_t can_tx_stack[512];

// USB communication tasks
static StaticTask_t usb_rx_task_buffer;
static StackType_t usb_rx_stack[512];

// Encoder tasks
static StaticTask_t encoder_read_task_buffer[2];  // For dual encoders
static StackType_t encoder_read_stack[2][256];

// IMU task
static StaticTask_t imu_read_task_buffer;
static StackType_t imu_read_stack[512];

// Application tasks
static StaticTask_t app_adc_task_buffer;
static StackType_t app_adc_stack[256];

// Utility tasks
static StaticTask_t led_task_buffer;
static StackType_t led_stack[128];

static StaticTask_t watchdog_task_buffer;
static StackType_t watchdog_stack[256];

static StaticTask_t telemetry_task_buffer;
static StackType_t telemetry_stack[512];

/* ===== Task Priority Map (FreeRTOS 0-7, where 7=highest) ===== */
#define TASK_PRIO_CRITICAL      6  // Clock duty/sync (was 245 in ChibiOS)
#define TASK_PRIO_MOTOR_OBS     6  // FOC observer
#define TASK_PRIO_CAN_RX        5  // Real-time CAN incoming
#define TASK_PRIO_CAN_TX        4  // CAN batch transmit
#define TASK_PRIO_USB_RX        5  // USB serial input
#define TASK_PRIO_ENCODER       5  // Encoder position update
#define TASK_PRIO_IMU           4  // IMU data reading
#define TASK_PRIO_APP           3  // App input (throttle, PAS)
#define TASK_PRIO_TELEMETRY     2  // Low priority telemetry
#define TASK_PRIO_LED           1  // Lowest priority indicator

/* ===== Forward Declarations ===== */
// Task entry points (defined elsewhere in codebase)
void *motor_control_thread(void *arg);
void *foc_observer_thread(void *arg);
void *can_rx_thread(void *arg);
void *can_tx_thread(void *arg);
void *usb_rx_thread(void *arg);
void *encoder_thread(void *arg);
void *imu_thread(void *arg);
void *app_adc_thread(void *arg);
void *led_task_thread(void *arg);
void *watchdog_thread(void *arg);
void *telemetry_thread(void *arg);

void main(void) {
    // 1. Hardware initialization (HAL, GPIO, timers - before RTOS starts)
    hw_init();           // STM32 HAL setup
    hw_init_gpio();      // GPIO abstraction layer (Agent 1)
    // Timer setup happens here - Agent 3
    mc_interface_init(); // Motor control ISRs (not tasks)
    app_init();          // App layer (non-existent in ChibiOS initially)
    
    // 2. Start FreeRTOS kernel
    osKernelInitialize();  // Initialize kernel
    
    // 3. Create all static tasks (in priority order)
    // CRITICAL priority tasks
    osThreadNew((osThreadFunc_t)motor_control_thread, NULL, 
        &(const osThreadAttr_t){
            .name = "motor_ctrl",
            .priority = osPriorityHigh2,  // Maps to FreeRTOS priority 6
            .stack_mem = motor_ctrl_stack,
            .stack_size = sizeof(motor_ctrl_stack),
            .cb_mem = &motor_ctrl_task_buffer,
            .cb_size = sizeof(motor_ctrl_task_buffer)
        });
    
    osThreadNew((osThreadFunc_t)foc_observer_thread, NULL,
        &(const osThreadAttr_t){
            .name = "foc_obs",
            .priority = osPriorityHigh2,
            .stack_mem = foc_observer_stack,
            .stack_size = sizeof(foc_observer_stack),
            .cb_mem = &foc_observer_task_buffer,
            .cb_size = sizeof(foc_observer_task_buffer)
        });
    
    // High priority communication
    for (int i = 0; i < 5; i++) {
        osThreadNew((osThreadFunc_t)can_rx_thread, (void *)(intptr_t)i,
            &(const osThreadAttr_t){
                .name = "can_rx",
                .priority = osPriorityHigh1,  // Priority 5
                .stack_mem = can_rx_stack[i],
                .stack_size = sizeof(can_rx_stack[i]),
                .cb_mem = &can_rx_task_buffer[i],
                .cb_size = sizeof(can_rx_task_buffer[i])
            });
    }
    
    osThreadNew((osThreadFunc_t)can_tx_thread, NULL,
        &(const osThreadAttr_t){
            .name = "can_tx",
            .priority = osPriorityAboveNormal,  // Priority 4
            .stack_mem = can_tx_stack,
            .stack_size = sizeof(can_tx_stack),
            .cb_mem = &can_tx_task_buffer,
            .cb_size = sizeof(can_tx_task_buffer)
        });
    
    osThreadNew((osThreadFunc_t)usb_rx_thread, NULL,
        &(const osThreadAttr_t){
            .name = "usb_rx",
            .priority = osPriorityHigh1,
            .stack_mem = usb_rx_stack,
            .stack_size = sizeof(usb_rx_stack),
            .cb_mem = &usb_rx_task_buffer,
            .cb_size = sizeof(usb_rx_task_buffer)
        });
    
    // Medium priority tasks
    osThreadNew((osThreadFunc_t)encoder_thread, NULL,
        &(const osThreadAttr_t){
            .name = "encoder",
            .priority = osPriorityAboveNormal,
            .stack_mem = encoder_read_stack[0],
            .stack_size = sizeof(encoder_read_stack[0]),
            .cb_mem = &encoder_read_task_buffer[0],
            .cb_size = sizeof(encoder_read_task_buffer[0])
        });
    
    osThreadNew((osThreadFunc_t)imu_thread, NULL,
        &(const osThreadAttr_t){
            .name = "imu",
            .priority = osPriorityNormal,  // Priority 3
            .stack_mem = imu_read_stack,
            .stack_size = sizeof(imu_read_stack),
            .cb_mem = &imu_read_task_buffer,
            .cb_size = sizeof(imu_read_task_buffer)
        });
    
    osThreadNew((osThreadFunc_t)app_adc_thread, NULL,
        &(const osThreadAttr_t){
            .name = "app_adc",
            .priority = osPriorityNormal,
            .stack_mem = app_adc_stack,
            .stack_size = sizeof(app_adc_stack),
            .cb_mem = &app_adc_task_buffer,
            .cb_size = sizeof(app_adc_task_buffer)
        });
    
    // Low priority utility tasks
    osThreadNew((osThreadFunc_t)telemetry_thread, NULL,
        &(const osThreadAttr_t){
            .name = "telemetry",
            .priority = osPriorityBelowNormal,  // Priority 2
            .stack_mem = telemetry_stack,
            .stack_size = sizeof(telemetry_stack),
            .cb_mem = &telemetry_task_buffer,
            .cb_size = sizeof(telemetry_task_buffer)
        });
    
    osThreadNew((osThreadFunc_t)led_task_thread, NULL,
        &(const osThreadAttr_t){
            .name = "led",
            .priority = osPriorityLow,  // Priority 1
            .stack_mem = led_stack,
            .stack_size = sizeof(led_stack),
            .cb_mem = &led_task_buffer,
            .cb_size = sizeof(led_task_buffer)
        });
    
    osThreadNew((osThreadFunc_t)watchdog_thread, NULL,
        &(const osThreadAttr_t){
            .name = "watchdog",
            .priority = osPriorityHigh,  // Priority 5 - can preempt normal tasks
            .stack_mem = watchdog_stack,
            .stack_size = sizeof(watchdog_stack),
            .cb_mem = &watchdog_task_buffer,
            .cb_size = sizeof(watchdog_task_buffer)
        });
    
    // 4. Start kernel scheduler
    osKernelStart();
    
    // Loop never returns (unless kernel stops)
    while (1) {
        osDelay(1000);
    }
}
```

---

## Phase 4: Detailed File-by-File Synchronization Conversion

**TIDAK ADA WRAPPER.** Setiap file dikonversi langsung dari ChibiOS ke FreeRTOS.

### 4.1 FIND & REPLACE PATTERNS - MUTEX

**Pattern 1: Deklarasi mutex statis**

```
ChibiOS:  static mutex_t NAMA;
FreeRTOS: static StaticSemaphore_t NAMA##_buffer;
          static osMutexId_t NAMA = NULL;
```

**Pattern 2: Inisialisasi mutex di chMtxObjectInit()**

```
Cari di file:   chMtxObjectInit(&NAMA);
Ganti dengan:   do { \
                    if (!NAMA) { \
                        const osMutexAttr_t attr = {.name = #NAMA}; \
                        NAMA = osMutexNew(&attr); \
                    } \
                } while(0)
```

**Pattern 3: Lock/Unlock**

```
ChibiOS:  chMtxLock(&lock);      → FreeRTOS: osMutexAcquire(lock, osWaitForever);
ChibiOS:  chMtxUnlock(&lock);    → FreeRTOS: osMutexRelease(lock);
ChibiOS:  chMtxTryLock(&lock)    → FreeRTOS: osMutexAcquire(lock, 0) == osOK
```

**File-by-file (MUTEX)**:

1. **Src/comm/comm_can.c**
   - Line ~50: `static mutex_t can_queue_lock;`
   - Replace dengan:
     ```c
     static StaticSemaphore_t can_queue_lock_buffer;
     static osMutexId_t can_queue_lock = NULL;
     
     // Di function can_init():
     if (!can_queue_lock) {
         const osMutexAttr_t attr = {.name = "can_queue_lock"};
         can_queue_lock = osMutexNew(&attr);
     }
     ```
   - Semua `chMtxLock(&can_queue_lock)` → `osMutexAcquire(can_queue_lock, osWaitForever);`
   - Semua `chMtxUnlock(&can_queue_lock)` → `osMutexRelease(can_queue_lock);`

2. **Src/motor/mcpwm_foc.c**
   - Cari: `static mutex_t pwm_lock;` atau semua mutex yang ada
   - Lakukan replacement (3 baris di atas, 3 di bawah untuk konteks)

3. **Src/terminal.c**
   - Cari: `static mutex_t terminal_lock;`
   - Replace dengan struct buffer + osMutexId_t

4. **Src/comm/comm_usb_serial.c**
   - Cari: semua `chMtxLock` calls
   - Replace dengan `osMutexAcquire`

5. **Src/driver/i2c_bb.c**
   - Cari: `static mutex_t i2c_mutex;`
   - Replace dengan FreeRTOS mutex

---

### 4.2 FIND & REPLACE PATTERNS - EVENT FLAGS

**Pattern 1: Deklarasi event**

```
ChibiOS:  static event_listener_t listener;
FreeRTOS: static StaticEventGroup_t event_buffer;
          static osEventFlagsId_t event_flags = NULL;
```

**Pattern 2: Enumerasi flag**

```
ChibiOS:  #define EVENT_MASK_1  (1<<0)
          #define EVENT_MASK_2  (1<<1)
          
FreeRTOS: #define EVENT_FLAG_1  0x01
          #define EVENT_FLAG_2  0x02
```

**Pattern 3: Wait/Signal**

```
ChibiOS:  chEvtWaitAny(EVENT_MASK_1 | EVENT_MASK_2);
FreeRTOS: osEventFlagsWait(event_flags, 0x01 | 0x02, osFlagsWaitAny, osWaitForever);

ChibiOS:  chEvtSignal(thread_ptr, EVENT_MASK_1);
FreeRTOS: osEventFlagsSet(event_flags, 0x01);
```

**File-by-file (EVENT FLAGS)**:

1. **Src/comm/comm_usb.c**
   - Deklarasi: `static event_listener_t ... listener;` 
   - Replace dengan:
     ```c
     #define USB_RX_AVAILABLE   0x01
     #define USB_DISCONNECT     0x02
     
     static StaticEventGroup_t usb_events_buffer;
     static osEventFlagsId_t usb_events = NULL;
     
     // Di usb_init():
     if (!usb_events) {
         const osEventFlagsAttr_t attr = {.name = "usb_events"};
         usb_events = osEventFlagsNew(&attr);
     }
     ```
   - `chEvtWaitAny(...)` → `osEventFlagsWait(usb_events, ...flags..., osFlagsWaitAny, osWaitForever)`
   - `chEvtSignal(usb_thread_ptr, ...)` → `osEventFlagsSet(usb_events, ...flags...)`

2. **Src/comm/commands.c**
   - Cari `chEvtWaitAny`
   - Replace dengan osEventFlagsWait

3. **Src/applications/app_*.c** (semua app files)
   - Cari event signals
   - Replace dengan osEventFlagsSet

---

### 4.3 FIND & REPLACE PATTERNS - SEMAPHORE

**Pattern 1: Binary semaphore init**

```
ChibiOS:  static binary_semaphore_t can_sem;
          chBSemObjectInit(&can_sem, FALSE);
          
FreeRTOS: static StaticSemaphore_t can_sem_buffer;
          static osSemaphoreId_t can_sem = NULL;
          
          if (!can_sem) {
              const osSemaphoreAttr_t attr = {.name = "can_sem"};
              can_sem = osSemaphoreNew(1, 0, &attr);  // max=1, init=0
          }
```

**Pattern 2: Wait/Signal **

```
ChibiOS:  chBSemWait(&sem);        → FreeRTOS: osSemaphoreAcquire(sem, osWaitForever);
ChibiOS:  chBSemSignal(&sem);      → FreeRTOS: osSemaphoreRelease(sem);
ChibiOS:  chSemWait(&sem)          → FreeRTOS: osSemaphoreAcquire(sem, osWaitForever);
ChibiOS:  chSemSignal(&sem)        → FreeRTOS: osSemaphoreRelease(sem);
```

**File-by-file (SEMAPHORE)**:

1. **Src/comm/comm_can.c**
   - Cari: `chBSemObjectInit` atau `chSemObjectInit`
   - Replace dengan osSemaphoreNew initialization
   - `chBSemWait` → `osSemaphoreAcquire(sem, osWaitForever)`
   - `chBSemSignal` → `osSemaphoreRelease(sem)`

---

### 4.4 THREAD CREATION & MANAGEMENT

NO WRAPPER. Semua thread diganti langsung.

**Pattern: Thread function signature**

```
ChibiOS:  
static THD_FUNCTION(thread_name, arg) {
    chRegSetThreadName("name");
    while (chThdShouldTerminateNow() == FALSE) {
        // code
    }
}

FreeRTOS: 
void *thread_name(void *arg) {
    osThreadSetName(arg, "name");
    while (1) {
        // code
    }
    return NULL;
}
```

**Pattern: Thread creation dalam main()**

```
ChibiOS:  
chThdCreateStatic(wa_thread, sizeof(wa_thread), 
    NORM_PRIO, thread_func, NULL);

FreeRTOS: 
osThreadNew((osThreadFunc_t)thread_func, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityNormal,
        .stack_mem = thread_stack,
        .stack_size = sizeof(thread_stack),
        .cb_mem = &thread_cb_buffer,
        .cb_size = sizeof(thread_cb_buffer)
    });
```

**File-by-file (THREAD CREATION)**:

(Lihat Phase 3 di atas untuk detail lengkap setiap thread)

---

## Phase 5: Detailed Thread Function Conversions (SETIAP FILE)

**Tidak ada wrapper. Direct import cmsis_os2.h dan ubah semua function signature.**

### 5.1 Motor Control Loop (Src/motor/mcpwm_foc.c)

**FIND THIS CODE**:
```c
static const evhandler_t evhndl[] = {
    mcpwm_foc_event_handler
};

static THD_FUNCTION(mcpwm_foc_process_thread, arg) {
    chRegSetThreadName("mcpwm_foc");
    
    while (chThdShouldTerminateNow() == FALSE) {
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }
}
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

static osEventFlagsId_t foc_events = NULL;

void *mcpwm_foc_process_thread(void *arg) {
    if (!foc_events) {
        const osEventFlagsAttr_t attr = {.name = "foc_events"};
        foc_events = osEventFlagsNew(&attr);
    }
    
    while (1) {
        uint32_t flags = osEventFlagsWait(foc_events, 0xFFFFFFFF, 
            osFlagsWaitAny, osWaitForever);
        
        if (flags & FOC_UPDATE_READY) {
            mcpwm_foc_event_handler();
        }
    }
    
    return NULL;
}
```

---

### 5.2 CAN RX Threads (Src/comm/comm_can.c) - 5 INSTANCES

**FIND THIS CODE**:
```c
static THD_FUNCTION(can_rx_thread, arg) {
    chRegSetThreadName("can_rx");
    CANRxFrame rxframe;
    
    while (!chThdShouldTerminateNow()) {
        msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxframe, MS2ST(100));
        if (result == MSG_OK) {
            // process frame
        }
    }
}

// In main():
chThdCreateStatic(wa_can_rx_1, sizeof(wa_can_rx_1), NORM_PRIO, can_rx_thread, NULL);
chThdCreateStatic(wa_can_rx_2, sizeof(wa_can_rx_2), NORM_PRIO, can_rx_thread, NULL);
// ... etc 5x total
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

static osEventFlagsId_t can_rx_events[5] = {0};

void *can_rx_thread(void *arg) {
    int bus_id = (intptr_t)arg;  // 0, 1, 2, 3, 4
    
    if (!can_rx_events[bus_id]) {
        const osEventFlagsAttr_t attr = {.name = "can_rx"};
        can_rx_events[bus_id] = osEventFlagsNew(&attr);
    }
    
    while (1) {
        uint32_t flags = osEventFlagsWait(can_rx_events[bus_id], 0x01,
            osFlagsWaitAny, pdMS_TO_TICKS(100));
        
        if (flags & 0x01) {
            // Get received CAN message
            CAN_RxHeaderTypeDef rxhdr;
            uint8_t rxdata[8];
            HAL_CAN_GetRxMessage(&hcan[bus_id], CAN_RX_FIFO0, &rxhdr, rxdata);
            comm_can_rx_handler(&rxhdr, rxdata);
        }
    }
    
    return NULL;
}

// In main() - create 5 threads:
for (int i = 0; i < 5; i++) {
    osThreadNew((osThreadFunc_t)can_rx_thread, (void *)(intptr_t)i, 
        &(const osThreadAttr_t){
            .priority = osPriorityHigh1,
            .stack_mem = can_rx_stacks[i],
            .stack_size = 512,
            .cb_mem = &can_rx_buffers[i],
            .cb_size = sizeof(StaticTask_t)
        });
}
```

---

### 5.3 USB RX Thread (Src/comm/comm_usb.c)

**FIND**:
```c
static THD_FUNCTION(usb_rx_thread, arg) {
    chRegSetThreadName("usb_rx");
    
    while (!chThdShouldTerminateNow()) {
        chEvtWaitAny(USB_RX_AVAILABLE);
        // process USB data
    }
}

// In main():
chThdCreateStatic(wa_usb_rx, sizeof(wa_usb_rx), HIGH_PRIO, usb_rx_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

static osEventFlagsId_t usb_rx_events = NULL;
#define USB_DATA_AVAILABLE  0x01
#define USB_DISCONNECT      0x02

void *usb_rx_thread(void *arg) {
    if (!usb_rx_events) {
        const osEventFlagsAttr_t attr = {.name = "usb_rx"};
        usb_rx_events = osEventFlagsNew(&attr);
    }
    
    while (1) {
        uint32_t flags = osEventFlagsWait(usb_rx_events,
            USB_DATA_AVAILABLE | USB_DISCONNECT,
            osFlagsWaitAny, osWaitForever);
        
        if (flags & USB_DATA_AVAILABLE) {
            // Process USB data
            uint8_t buffer[256];
            int len = USB_read(buffer, sizeof(buffer));
            if (len > 0) {
                terminal_rx(buffer, len);
            }
        }
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)usb_rx_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityHigh1,
        .stack_mem = usb_rx_stack,
        .stack_size = 512,
        .cb_mem = &usb_rx_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.4 Encoder Thread (Src/encoder/encoder.c)

**FIND**:
```c
static THD_FUNCTION(encoder_thread, arg) {
    chRegSetThreadName("encoder");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(10);
        // update encoder position
    }
}

// In main():
chThdCreateStatic(wa_encoder, sizeof(wa_encoder), NORM_PRIO, encoder_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *encoder_thread(void *arg) {
    while (1) {
        // Update encoder position
        float new_pos = encoder_read_raw();
        
        // Thread-safe update via mutex
        osMutexAcquire(encoder_lock, osWaitForever);
        encoder_position = new_pos;
        osMutexRelease(encoder_lock);
        
        osDelay(pdMS_TO_TICKS(10));  // 100 Hz update
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)encoder_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityAboveNormal,
        .stack_mem = encoder_stack,
        .stack_size = 256,
        .cb_mem = &encoder_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.5 IMU Thread (Src/imu/imu.c)

**FIND**:
```c
static THD_FUNCTION(imu_thread, arg) {
    chRegSetThreadName("imu");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(10);
        // read IMU sensors
    }
}

// In main():
chThdCreateStatic(wa_imu, sizeof(wa_imu), NORM_PRIO, imu_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *imu_thread(void *arg) {
    while (1) {
        uint8_t imu_data[14];
        if (imu_read_registers(0x3B, imu_data, 14) == 0) {
            // Parse and store sensor data
            osMutexAcquire(imu_lock, osWaitForever);
            // ... process IMU data ...
            osMutexRelease(imu_lock);
        }
        
        osDelay(pdMS_TO_TICKS(10));  // 100 Hz
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)imu_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityNormal,
        .stack_mem = imu_stack,
        .stack_size = 512,
        .cb_mem = &imu_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.6 APP ADC Thread (Src/applications/app.c)

**FIND**:
```c
static THD_FUNCTION(app_adc_thread, arg) {
    chRegSetThreadName("app_adc");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(5);
        app_sample_adc();
    }
}

// In main():
chThdCreateStatic(wa_app_adc, sizeof(wa_app_adc), NORM_PRIO, app_adc_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *app_adc_thread(void *arg) {
    while (1) {
        app_sample_adc();
        osDelay(pdMS_TO_TICKS(5));  // 200 Hz
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)app_adc_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityNormal,
        .stack_mem = app_adc_stack,
        .stack_size = 256,
        .cb_mem = &app_adc_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.7 Watchdog Thread (Src/timeout.c)

**FIND**:
```c
static THD_FUNCTION(watchdog_thread, arg) {
    chRegSetThreadName("watchdog");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(100);
        timeout_check();
    }
}

// In main():
chThdCreateStatic(wa_watchdog, sizeof(wa_watchdog), HIGH_PRIO, watchdog_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *watchdog_thread(void *arg) {
    while (1) {
        timeout_check();
        osDelay(pdMS_TO_TICKS(100));
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)watchdog_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityHigh,
        .stack_mem = watchdog_stack,
        .stack_size = 256,
        .cb_mem = &watchdog_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.8 LED Task (Src/ledpwm.c)

**FIND**:
```c
static THD_FUNCTION(ledpwm_thread, arg) {
    chRegSetThreadName("LED");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(500);
        led_toggle();
    }
}

// In main():
chThdCreateStatic(wa_led, sizeof(wa_led), LOW_PRIO, ledpwm_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *led_thread(void *arg) {
    while (1) {
        led_toggle();
        osDelay(pdMS_TO_TICKS(500));
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)led_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityLow,
        .stack_mem = led_stack,
        .stack_size = 128,
        .cb_mem = &led_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

### 5.9 Telemetry Thread (Src/commands.c atau Src/comm/log.c)

**FIND**:
```c
static THD_FUNCTION(telemetry_thread, arg) {
    chRegSetThreadName("telemetry");
    
    while (!chThdShouldTerminateNow()) {
        chThdSleepMilliseconds(100);
        log_send_telemetry();
    }
}

// In main():
chThdCreateStatic(wa_telemetry, sizeof(wa_telemetry), LOW_PRIO, telemetry_thread, NULL);
```

**REPLACE WITH**:
```c
#include "cmsis_os2.h"

void *telemetry_thread(void *arg) {
    while (1) {
        log_send_telemetry();
        osDelay(pdMS_TO_TICKS(100));
    }
    
    return NULL;
}

// In main():
osThreadNew((osThreadFunc_t)telemetry_thread, NULL,
    &(const osThreadAttr_t){
        .priority = osPriorityBelowNormal,
        .stack_mem = telemetry_stack,
        .stack_size = 512,
        .cb_mem = &telemetry_buffer,
        .cb_size = sizeof(StaticTask_t)
    });
```

---

## Phase 6: ISR Signal Callbacks

### 6.1 Motor ISR Event Signals

In `Src/motor/mcpwm_foc.c`, ADC/timer ISRs signal observer tasks:

```c
// ISR context - signal from ADC interrupt
static osEventFlagsId_t foc_update_event;

void ADC1_2_3_IRQHandler(void) {
    // ... existing ISR code (unchanged) ...
    
    // Signal FOC observer thread (ISR-safe version)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    osEventFlagsSet(foc_update_event, FOC_READY);  // or use task notification
    
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
```

### 6.2 Initialize Events in main.c

```c
// In main() before osKernelStart()
void init_isr_events(void) {
    const osEventFlagsAttr_t attr = {0};
    
    foc_update_event = osEventFlagsNew(&attr);
    can_rx_events[0] = osEventFlagsNew(&attr);
    can_rx_events[1] = osEventFlagsNew(&attr);
    // ... etc for all ISR-signaled events
}
```

---

## Phase 7: Testing & Validation

### 7.1 Task Scheduling Test

```c
// Src/tests/test_rtos.c
void test_task_priorities(void) {
    // Launch tasks with different priorities
    // Verify higher priority task preempts lower priority
    // Verify no hard real-time guarantees broken
}

void test_mutex_priority_inheritance(void) {
    // High priority task blocks on low priority task's mutex
    // Verify low priority task inherits high priority (no inversion)
}

void test_isr_signaling(void) {
    // ADC ISR fires → signals FOC task
    // Verify FOC task wakes within <2 ticks
}

void test_concurrent_tasks(void) {
    // Launch all 25 tasks
    // Run for 10 seconds
    // Check stack usage, task states, no deadlocks
}
```

### 7.2 Stack Sizing Validation

```c
// Tools: FreeRTOS stack monitoring
void check_stack_usage(void) {
    osTaskId_t task_list[30];
    uint32_t task_count = osThreadEnumerate(task_list, 30);
    
    for (int i = 0; i < task_count; i++) {
        // Get high water mark for each task
        uint32_t free_stack = osThreadGetStackSpace(task_list[i]);
        printf("Task %s: %lu bytes free\n", osThreadGetName(task_list[i]), free_stack);
    }
}
```

### 7.3 Checklist for Completion

- [ ] FreeRTOSConfig.h created with all settings
- [ ] cmsis_rtos_compat.h abstraction complete
- [ ] All task creation in main.c using osThreadNew()
- [ ] All `chMtxLock` replaced with `osMutexAcquire`
- [ ] All `chEvtWaitAny` replaced with `osEventFlagsWait`
- [ ] All CAN RX tasks created (×5)
- [ ] All USB RX tasks created
- [ ] Encoder tasks created (×2 for dual motor)
- [ ] ISR callbacks signal events correctly
- [ ] No `chThdCreateStatic` references remain
- [ ] No `chEvt*` references remain except in comments
- [ ] All tasks compile without errors
- [ ] Tasks launch in correct priority order
- [ ] No deadlocks or task starvation observed
- [ ] Stack usage <70% for all tasks

---

## Key Files to Create/Modify

| File | Action | Purpose | Priority |
|------|--------|---------|----------|
| `Src/FreeRTOSConfig.h` | **CREATE** | FreeRTOS kernel configuration | **CRITICAL** |
| `Src/cmsis_rtos_compat.h` | **CREATE** | Abstraction layer for easy migration | **CRITICAL** |
| `Src/main.c` | **MODIFY** | Task creation and kernel start | **CRITICAL** |
| `Src/motor/mcpwm_foc.c` | **MODIFY** | Motor observer thread (FOC update) | **HIGH** |
| `Src/comm/comm_can.c` | **MODIFY** | CAN RX/TX threads (×6) | **HIGH** |
| `Src/comm/comm_usb.c` | **MODIFY** | USB RX thread | **HIGH** |
| `Src/encoder/encoder.c` | **MODIFY** | Encoder position polling | **HIGH** |
| `Src/imu/imu.c` | **MODIFY** | IMU data reading thread | **MEDIUM** |
| `Src/applications/app_adc.c` | **MODIFY** | Input ADC sampling | **MEDIUM** |
| `Src/ledpwm.c` | **MODIFY** | LED blink thread | **LOW** |
| `Src/timeout.c` | **MODIFY** | Watchdog/timeout thread | **MEDIUM** |

---

## Success Criteria

✅ All 25 tasks created successfully
✅ No priority inversions observed
✅ Tasks wake correctly from mutex and event waits
✅ ISR callbacks signal events without timing drift
✅ All tasks run in steady state (no exceptions)
✅ Motor control latency unchanged (<1% regression)
✅ CAN/USB communication maintains throughput
✅ No stack overflow detected
✅ Code compiles without `ch*` (ChibiOS) references

---

## Migration Notes

- **FreeRTOS tick rate**: 1 kHz (1 ms) is sufficient for VESC applications
- **Task priorities**: Inverse map from ChibiOS prio-255 to FreeRTOS prio 0-7
- **Memory model**: All static allocation (no dynamic malloc) → deterministic behavior
- **ISR safety**: Use `osEventFlagsSet()` with FreeRTOS ISR macros for context switch awareness
- **Motor ISR**: Running as raw interrupt handler (not FreeRTOS task) → no scheduler overhead

