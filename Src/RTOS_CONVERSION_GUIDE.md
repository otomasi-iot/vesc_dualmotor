# FreeRTOS CMSIS v2 Conversion Reference Guide

## Overview
This guide covers the systematic conversion of remaining ChibiOS thread functions to FreeRTOS CMSIS v2.
All replacements are DIRECT - no wrapper functions.

---

## Conversion Rules - Quick Reference

### Rule 1: Thread Function Signature
```c
// ChibiOS
static THD_FUNCTION(thread_name, arg) {
    chRegSetThreadName("name");
    for(;;) {
        // code
    }
}

// FreeRTOS CMSIS v2
static void *thread_name(void *arg) {
    (void)arg;
    while(1) {
        // code
    }
    return NULL;
}
```

### Rule 2: Thread Creation

```c
// ChibiOS
static THD_WORKING_AREA(thread_wa, 512);  // Stack declaration
chThdCreateStatic(thread_wa, sizeof(thread_wa), NORMALPRIO, thread_func, NULL);

// FreeRTOS CMSIS v2
// (1) Add at beginning of file, with other static buffers:
static StaticTask_t thread_buffer;
static StackType_t thread_stack[512];

// (2) Add osThreadNew in init function:
osThreadNew((osThreadFunc_t)thread_func, NULL,
    &(const osThreadAttr_t){
        .name = "thread_name",
        .priority = osPriorityNormal,    // See priority mapping below
        .stack_mem = thread_stack,
        .stack_size = sizeof(thread_stack),
        .cb_mem = &thread_buffer,
        .cb_size = sizeof(thread_buffer)
    });
```

### Rule 3: Priority Mapping

| ChibiOS        | FreeRTOS CMSIS v2        | Numeric |
|----------------|--------------------------|---------|
| LOWPRIO        | osPriorityLow            | 1       |
| NORMALPRIO     | osPriorityNormal         | 3       |
| HIGHPRIO       | osPriorityAboveNormal    | 4       |
| NORMALPRIO+1   | osPriorityHigh           | 5       |
| NORMALPRIO+2   | osPriorityHigh1          | 6       |
| NORMALPRIO+3   | osPriorityHigh2          | 7       |
| BelowNormal    | osPriorityBelowNormal    | 2       |

### Rule 4: Delay/Sleep Conversions

```c
// ChibiOS
chThdSleep(MS2ST(100));              // sleep 100ms
chThdSleepMilliseconds(100);         // sleep 100ms
chThdSleepMicroseconds(500);         // NOT SUPPORTED
osDelay(pdMS_TO_TICKS(100));         // Already FreeRTOS

// FreeRTOS CMSIS v2
osDelay(100);                        // sleep 100ms (1 kHz tick = 1ms)
osDelay(pdMS_TO_TICKS(100));         // Explicit conversion
```

### Rule 5: Mutex Conversions

```c
// ChibiOS
static mutex_t lock;
void init(void) {
    chMtxObjectInit(&lock);
}
void func(void) {
    chMtxLock(&lock);
    // critical section
    chMtxUnlock(&lock);
}

// FreeRTOS CMSIS v2
static StaticSemaphore_t lock_buffer;
static osMutexId_t lock = NULL;

void init(void) {
    if (!lock) {
        const osMutexAttr_t attr = {.name = "lock"};
        lock = osMutexNew(&attr);
    }
}

void func(void) {
    osMutexAcquire(lock, osWaitForever);
    // critical section
    osMutexRelease(lock);
}

void func_with_timeout(void) {
    osStatus_t status = osMutexAcquire(lock, pdMS_TO_TICKS(100));  // 100ms timeout
    if (status == osOK) {
        // critical section
        osMutexRelease(lock);
    } else {
        // timeout occurred
    }
}
```

### Rule 6: Event Flags Conversions

```c
// ChibiOS - Using event listeners
event_listener_t listener;
chEvtRegister(&event_source, &listener, 0);
uint32_t events = chEvtWaitAny(EVENT_MASK);
uint32_t events = chEvtWaitAnyTimeout(EVENT_MASK, MS2ST(100));
chEvtSignal(thread_ptr, EVENT_MASK);
chEvtUnregister(&event_source, &listener);

// FreeRTOS CMSIS v2
static StaticEventGroup_t event_buffer;
static osEventFlagsId_t event_flags = NULL;

// In init:
if (!event_flags) {
    const osEventFlagsAttr_t attr = {.name = "events"};
    event_flags = osEventFlagsNew(&attr);
}

// Wait indefinitely:
uint32_t flags = osEventFlagsWait(event_flags, 0x01, osFlagsWaitAny, osWaitForever);

// Wait with timeout:
uint32_t flags = osEventFlagsWait(event_flags, 0x01, osFlagsWaitAny, pdMS_TO_TICKS(100));

// Check if timeout:
if (flags & osFlagsError) {
    // Error (including timeout)
} else {
    // Flags set successfully
}

// Signal from ISR or thread:
osEventFlagsSet(event_flags, 0x01);
```

### Rule 7: Semaphore Conversions

```c
// ChibiOS
binary_semaphore_t sem;
chBSemObjectInit(&sem, FALSE);  // FALSE = not signaled initially
chBSemWait(&sem);
chBSemSignal(&sem);

// FreeRTOS CMSIS v2
static StaticSemaphore_t sem_buffer;
static osSemaphoreId_t sem = NULL;

// In init:
if (!sem) {
    const osSemaphoreAttr_t attr = {.name = "sem"};
    sem = osSemaphoreNew(1, 0, &attr);  // max=1, initial=0
}

osSemaphoreAcquire(sem, osWaitForever);
osSemaphoreRelease(sem);

// Counting semaphore (multiple resources):
sem = osSemaphoreNew(5, 5, &attr);  // max=5, initial=5
```

### Rule 8: Time Measurement Conversions

```c
// ChibiOS
systime_t last_time = chVTGetSystemTimeX();
systime_t elapsed_ticks = chVTTimeElapsedSinceX(last_time);
float elapsed_ms = ST2MS(elapsed_ticks);

// FreeRTOS CMSIS v2
uint32_t last_time = osKernelGetTickCount();  // Time in milliseconds (1 kHz tick)
uint32_t elapsed_ms = osKernelGetTickCount() - last_time;

// With explicit tick rate:
uint32_t elapsed_ms = (osKernelGetTickCount() - last_time) * 1000 / osKernelGetTickFreq();
```

### Rule 9: Termination Check Conversions

```c
// ChibiOS
while (!chThdShouldTerminateNow()) { ... }
if (chThdShouldTerminateX()) { break; }

// FreeRTOS CMSIS v2
while (1) { ... }  // Infinite loop - FreeRTOS doesn't delete tasks
// To support early exit on termination (if needed):
static volatile bool should_exit = false;
while (!should_exit) { ... }
```

---

## File-by-File Conversion Templates

### comm_usb.c - USB Serial Threads

**Location**: `Src/comm/comm_usb.c` lines ~34, ~46, ~74, ~126-127

**Elements to add at file top**:
```c
// FreeRTOS CMSIS v2 thread buffers
static StaticTask_t serial_read_thread_buffer;
static StackType_t serial_read_thread_stack[256];
static StaticTask_t serial_process_thread_buffer;
static StackType_t serial_process_thread_stack[2048];

// Synchronization
static StaticSemaphore_t serial_rx_sem_buffer;
static osSemaphoreId_t serial_rx_sem = NULL;

static StaticEventGroup_t usb_event_buffer;
static osEventFlagsId_t usb_event = NULL;
#define USB_DATA_RECEIVED    0x01
#define USB_DISCONNECTED     0x02
```

**USB Initialization**:
```c
// In comm_usb_init():
if (!serial_rx_sem) {
    const osSemaphoreAttr_t attr = {.name = "serial_rx"};
    serial_rx_sem = osSemaphoreNew(1, 0, &attr);
}

if (!usb_event) {
    const osEventFlagsAttr_t attr = {.name = "usb_event"};
    usb_event = osEventFlagsNew(&attr);
}

osThreadNew((osThreadFunc_t)serial_read_thread, NULL, &...);
osThreadNew((osThreadFunc_t)serial_process_thread, NULL, &...);
```

**Thread signature changes**:
```c
// OLD: static THD_FUNCTION(serial_read_thread, arg)
// NEW:
static void *serial_read_thread(void *arg) {
    (void)arg;
    // ... existing code ...
    return NULL;
}

static void *serial_process_thread(void *arg) {
    (void)arg;
    // ... existing code, replacing for(;;) with while(1) ...
    return NULL;
}
```

---

### app_adc.c - ADC Input Thread - **CRITICAL**

**Location**: `Src/applications/app_adc.c` lines ~52-53, ~102, ~164

**Key patterns in this file**:
- Thread function: `adc_thread` - uses `chThdSleepMilliseconds` and internal delay logic
- No mutexes or events visible - straightforward conversion
- Critical for motor control - ensure timing is preserved

**Conversion template**:
```c
// At file top:
static StaticTask_t adc_thread_buffer;
static StackType_t adc_thread_stack[512];

// In app_adc_start():
osThreadNew((osThreadFunc_t)adc_thread, NULL,
    &(const osThreadAttr_t){
        .name = "app_adc",
        .priority = osPriorityNormal,
        .stack_mem = adc_thread_stack,
        .stack_size = sizeof(adc_thread_stack),
        .cb_mem = &adc_thread_buffer,
        .cb_size = sizeof(adc_thread_buffer)
    });

// Thread function:
static void *adc_thread(void *arg) {
    (void)arg;
    
    while (1) {
        // Original loop body
        systime_t sleep_time = CH_CFG_ST_FREQUENCY / config.update_rate_hz;
        // Becomes:
        uint32_t sleep_ms = 1000 / config.update_rate_hz;  // Convert to ms
        osDelay(sleep_ms);
        
        // ... rest of code ...
    }
    
    return NULL;
}
```

---

### commands.c - Blocking Command Thread

**Location**: `Src/comm/commands.c` lines ~70-71, ~96, ~2020

**Pattern**:
- Uses `chMtxLock` / `chMtxUnlock` - needs conversion to osMutexAcquire/Release
- Likely uses events for signaling - convert to osEventFlagsWait

**Conversion template**:
```c
// At file top:
static StaticTask_t blocking_thread_buffer;
static StackType_t blocking_thread_stack[3000];

// Add mutex conversions:
// Wherever: static mutex_t cmd_mtx;
// Change to:
static StaticSemaphore_t cmd_mtx_buffer;
static osMutexId_t cmd_mtx = NULL;

// In initialization:
if (!cmd_mtx) {
    const osMutexAttr_t attr = {.name = "cmd_mtx"};
    cmd_mtx = osMutexNew(&attr);
}

// Thread function:
static void *blocking_thread(void *arg) {
    (void)arg;
    
    while (1) {
        // Original code; replace chMtxLock/Unlock
    }
    
    return NULL;
}
```

---

### (Other App Threads: app_dpv.c, app_skypuff.c, app_custom_template.c)

These follow the same pattern as app_adc.c:
1. Convert THD_FUNCTION to void *
2. Add static task buffers and stack
3. Replace chThdSleep* with osDelay()
4. Convert for(;;) to while(1)
5. Add return NULL at end

---

### util/worker.c - Worker Thread Pool

**Pattern**: 
- Generic worker thread
- Likely uses queue or semaphore for work items
- Simple periodic execution

**Conversion**: Standard thread function conversion

---

### nrf_driver.c - NRF RX/TX Threads

**Pattern**: 
- Two threads: rx_thread and tx_thread
- Radio communication
- Use same principles as USB/CAN threads

---

## ISR Signal Handling - **CRITICAL FOR MOTOR CONTROL**

### Motor ISR Modifications

When ISRs need to signal tasks, use FreeRTOS ISR-safe APIs:

```c
// In motor ISR (e.g., ADC1_2_3_IRQHandler):
#include "cmsis_os2.h"

extern osEventFlagsId_t foc_observer_event;
#define FOC_READY 0x01

void ADC1_2_3_IRQHandler(void) {
    // ... existing ISR code (motor control) ...
    
    // Signal FOC observer task (ISR-safe)
    osEventFlagsSet(foc_observer_event, FOC_READY);
    
    // NO context switch needed - osEventFlagsSet handles it
}
```

### Initialize ISR Events in main.c

```c
void init_isr_events(void) {
    const osEventFlagsAttr_t attr = {0};
    foc_observer_event = osEventFlagsNew(&attr);
    // ... other ISR events ...
}

// Call in main() before osKernelStart()
osKernelInitialize();
init_isr_events();
// Create all tasks...
osKernelStart();
```

---

## Stack Requirements Summary

Task stacks must accommodate:
- Local variables
- Function call depth
- Worst-case stack growth

Current stack sizes from ChibiOS remain valid for FreeRTOS:
| Task                    | Stack (bytes) |
|-------------------------|---------------|
| LED thread              | 256           |
| Periodic thread         | 256           |
| Flash check             | 256           |
| ADC app thread          | 512           |
| CAN read                | 256           |
| CAN process             | 2048          |
| CAN status              | 512           |
| Serial comm             | 256-2048      |
| Worker thread           | 768           |
| NRF rx/tx               | 2048/256      |

---

## Validation Checklist

After converting all threads:

- [ ] All `THD_FUNCTION` replaced with `void *` returning NULL
- [ ] All `chThdCreateStatic` replaced with `osThreadNew`
- [ ] All `chMtxObjectInit` replaced with `osMutexNew` in init
- [ ] All `chMtxLock` replaced with `osMutexAcquire`
- [ ] All `chMtxUnlock` replaced with `osMutexRelease`
- [ ] All `chEvtWaitAny` replaced with `osEventFlagsWait`
- [ ] All `chEvtSignal` replaced with `osEventFlagsSet`
- [ ] All `chThdSleep*` replaced with `osDelay`
- [ ] All `chRegSetThreadName` removed (moved to osThreadAttr_t.name)
- [ ] All `for(;;)` in threads changed to `while(1)` (no semantic difference, just consistency)
- [ ] Code compiles without ChibiOS references (except includes)
- [ ] Motor control timing verified to be within <1% of original

---

## Common Mistakes to Avoid

1. **Forgetting return NULL** at end of thread functions - FreeRTOS requires it
2. **Using &mutex with osMutexAcquire** - Pass the ID directly: `osMutexAcquire(lock, ...)`
3. **Leaving chRegSetThreadName calls** - Remove these; use osThreadAttr_t.name instead
4. **Not initializing FreeRTOS objects before use** - Initialize in init functions before thread creation
5. **Using ST2MS/MS2ST macros without defining** - Use direct osDelay(ms) with 1 kHz tick
6. **Forgetting to remove THD_WORKING_AREA declarations** - Replace with StaticTask_t and StackType_t array
7. **Not handling osEventFlagsWait return values** - Check for osFlagsError for timeouts
8. **Mixing ChibiOS and FreeRTOS APIs** - Complete conversion per file

---

## Testing Strategy

1. **Per-file testing**: After converting each file, compile and verify no undefined references
2. **Functional testing**: Power on, verify basic operation (LEDs, CAN, USB)
3. **Timing testing**: Measure motor control loop jitter with oscilloscope
4. **Stress testing**: Run at full throttle for 10+ minutes, monitor for crashes
5. **Stack monitoring**: Enable configCHECK_FOR_STACK_OVERFLOW in FreeRTOSConfig.h

