# Quick Conversion Checklist - Remaining Threads

This file provides exact patterns for each remaining thread. Copy & modify as needed.

---

## comm_can.c - Remaining 3 Status Threads

### Pattern for cancom_status_thread (line ~1495)
```c
// Step 1: Already added at file top:
// static StaticTask_t cancom_status_thread_buffer;
// static StackType_t cancom_status_thread_stack[512];

// Step 2: Already added in comm_can_init():
// osThreadNew((osThreadFunc_t)cancom_status_thread, NULL, &(...));

// Step 3: Convert function signature
// FROM:
static THD_FUNCTION(cancom_status_thread, arg) {
    (void)arg;
    chRegSetThreadName("CAN status");
    for(;;) {
        // ... existing code ...
        osDelay(pdMS_TO_TICKS(100));
    }
}

// TO:
static void *cancom_status_thread(void *arg) {
    (void)arg;
    while(1) {
        // ... exact same code ...
        osDelay(pdMS_TO_TICKS(100));  // Already FreeRTOS
    }
    return NULL;
}
```

**Key changes for status threads:**
- Remove `chRegSetThreadName` line
- Replace `for(;;)` with `while(1)`
- Add `return NULL;` at end
- ALL existing code inside the loop stays the same
- `osDelay(pdMS_TO_TICKS(...))` already works - NO CHANGE needed

### cancom_status_thread_2 
Same pattern as cancom_status_thread

### cancom_status_internal_thread (HW_HAS_DUAL_MOTORS)
Same pattern - these are relatively simple status broadcast threads

---

## comm_usb.c - USB Serial Threads

### serial_read_thread Pattern

Located: `Src/comm/comm_usb.c` line ~46

```c
// Step 1: Add at file top with other statics:
static StaticTask_t serial_read_thread_buffer;
static StackType_t serial_read_thread_stack[256];

// Step 2: Add in comm_usb_start() function (find where chThdCreateStatic is):
osThreadNew((osThreadFunc_t)serial_read_thread, NULL,
    &(const osThreadAttr_t){
        .name = "usb_read",
        .priority = osPriorityAboveNormal,    // Was NORMALPRIO
        .stack_mem = serial_read_thread_stack,
        .stack_size = sizeof(serial_read_thread_stack),
        .cb_mem = &serial_read_thread_buffer,
        .cb_size = sizeof(serial_read_thread_buffer)
    });

// Step 3: Convert function (typically simpler than CAN):
// FROM:
static THD_FUNCTION(serial_read_thread, arg) {
    (void)arg;
    chRegSetThreadName("Serial RX");
    for(;;) {
        uint32_t data = comm_usb_read();
        if (data != -1) {
            // ... process ...
        }
        osDelay(pdMS_TO_TICKS(10));
    }
}

// TO:
static void *serial_read_thread(void *arg) {
    (void)arg;
    while(1) {
        uint32_t data = comm_usb_read();
        if (data != -1) {
            // ... process ...
        }
        osDelay(pdMS_TO_TICKS(10));
    }
    return NULL;
}
```

### serial_process_thread Pattern

Same as serial_read_thread - just rename function and add appropriate stack size (usually 2048)

**USB Events (if used):**
```c
// If threads use event signaling:
static StaticEventGroup_t usb_event_buffer;
static osEventFlagsId_t usb_event = NULL;
#define USB_DATA_AVAILABLE  0x01
#define USB_DISCONNECTED    0x02

// In init:
if (!usb_event) {
    const osEventFlagsAttr_t attr = {.name = "usb_event"};
    usb_event = osEventFlagsNew(&attr);
}

// In threads:
osEventFlagsWait(usb_event, USB_DATA_AVAILABLE, osFlagsWaitAny, osWaitForever);
osEventFlagsSet(usb_event, USB_DATA_AVAILABLE);
```

---

## app_adc.c - ADC Input Thread - **CRITICAL**

**Location**: Lines ~52-53 (buffers), ~102 (creation), ~164 (function)

### Step-by-step conversion:

```c
// Step 1: Replace working area declarations (line ~52-53)
// FROM:
__attribute__((section(".ram4"))) static THD_WORKING_AREA(adc_thread_wa, 512);

// TO:
static StaticTask_t adc_thread_buffer;
static StackType_t adc_thread_stack[512];

// Step 2: Remove old function declaration, keep forward declare:
// Keep: static THD_FUNCTION(adc_thread, arg);
// Or change to: static void *adc_thread(void *arg);

// Step 3: Replace thread creation (line ~102)
// FROM:
chThdCreateStatic(adc_thread_wa, sizeof(adc_thread_wa), NORMALPRIO, adc_thread, NULL);

// TO (in app_adc_start function):
osThreadNew((osThreadFunc_t)adc_thread, NULL,
    &(const osThreadAttr_t){
        .name = "app_adc",
        .priority = osPriorityNormal,
        .stack_mem = adc_thread_stack,
        .stack_size = sizeof(adc_thread_stack),
        .cb_mem = &adc_thread_buffer,
        .cb_size = sizeof(adc_thread_buffer)
    });

// Step 4: Convert thread function (line ~164)
// FROM:
static THD_FUNCTION(adc_thread, arg) {
    (void)arg;
    chRegSetThreadName("APP_ADC");
    is_running = true;
    
    for(;;) {
        systime_t sleep_time = CH_CFG_ST_FREQUENCY / config.update_rate_hz;
        if (sleep_time == 0) sleep_time = 1;
        chThdSleep(sleep_time);
        
        if (stop_now) {
            is_running = false;
            return;
        }
        
        // ... rest of existing code ...
    }
}

// TO:
static void *adc_thread(void *arg) {
    (void)arg;
    is_running = true;
    
    while(1) {
        // Calculate sleep time in milliseconds
        uint32_t sleep_ms = config.update_rate_hz > 0 ? (1000 / config.update_rate_hz) : 1;
        osDelay(sleep_ms);
        
        if (stop_now) {
            is_running = false;
            break;  // Instead of return
        }
        
        // ... rest of existing code (UNCHANGED) ...
    }
    
    is_running = false;
    return NULL;
}
```

**CRITICAL NOTE**: ADC thread timing is essential for throttle response! 
- Original: `CH_CFG_ST_FREQUENCY / config.update_rate_hz` ticks
- FreeRTOS: `1000 / config.update_rate_hz` milliseconds (since 1 kHz tick = 1 ms)
- These are equivalent!

---

## commands.c - blocking_thread

**Location**: Lines ~70-71 (buffers), ~96 (creation), ~2020 (function)

### Large function - follow this pattern:

```c
// Step 1: Add buffers
static StaticTask_t blocking_thread_buffer;
static StackType_t blocking_thread_stack[3000];  // Large stack for this thread

// Step 2: Replace creation in commands_init()
// FROM:
chThdCreateStatic(blocking_thread_wa, sizeof(blocking_thread_wa), NORMALPRIO, blocking_thread, NULL);

// TO:
osThreadNew((osThreadFunc_t)blocking_thread, NULL,
    &(const osThreadAttr_t){
        .name = "blocking_cmds",
        .priority = osPriorityNormal,
        .stack_mem = blocking_thread_stack,
        .stack_size = sizeof(blocking_thread_stack),
        .cb_mem = &blocking_thread_buffer,
        .cb_size = sizeof(blocking_thread_buffer)
    });

// Step 3: Find mutex/event patterns in the function
// common patterns:
// chMtxLock(&lock)     → osMutexAcquire(lock, osWaitForever)
// chMtxUnlock(&lock)   → osMutexRelease(lock)
// chBSemWait(&sem)     → osSemaphoreAcquire(sem, osWaitForever)
// chBSemSignal(&sem)   → osSemaphoreRelease(sem)
// chThdSleepMilliseconds → osDelay

// Step 4: Convert function signature
// FROM:
static THD_FUNCTION(blocking_thread, arg) {
    (void)arg;
    chRegSetThreadName("Cmds");
    for(;;) {
        // Large existing code block...
        chThdSleepMilliseconds(500);
    }
}

// TO:
static void *blocking_thread(void *arg) {
    (void)arg;
    while(1) {
        // SAME existing code (with mutex/event/sleep replacements)...
        osDelay(500);
    }
    return NULL;
}
```

---

## app_dpv.c / app_skypuff.c / app_custom_template.c

These are application threads. Use identical pattern:

```c
// Step 1: Add buffers to file
static StaticTask_t my_thread_buffer;
static StackType_t my_thread_stack[2048];  // Size from THD_WORKING_AREA

// Step 2: Create in app start function
osThreadNew((osThreadFunc_t)my_thread, NULL,
    &(const osThreadAttr_t){
        .name = "app_name",
        .priority = osPriorityNormal,
        .stack_mem = my_thread_stack,
        .stack_size = sizeof(my_thread_stack),
        .cb_mem = &my_thread_buffer,
        .cb_size = sizeof(my_thread_buffer)
    });

// Step 3: Convert function
// Remove: chRegSetThreadName
// Change: for(;;) → while(1)
// Change: return statements to break; (or just reach end of while)
// Add: return NULL; at end
// Change: chThdSleep* → osDelay
```

---

## util/worker.c - Worker Thread

Simple worker pool:

```c
// Step 1: Add buffers
static StaticTask_t work_thread_buffer;
static StackType_t work_thread_stack[768];

// Step 2: Create in worker_init()  
osThreadNew((osThreadFunc_t)work_thread, NULL,
    &(const osThreadAttr_t){
        .name = "worker",
        .priority = osPriorityNormal,
        .stack_mem = work_thread_stack,
        .stack_size = sizeof(work_thread_stack),
        .cb_mem = &work_thread_buffer,
        .cb_size = sizeof(work_thread_buffer)
    });

// Step 3: Convert function (straightforward)
static void *work_thread(void *arg) {
    (void)arg;
    work_data_t *data = (work_data_t *)arg;  // If data passed
    
    while(1) {
        // Wait for work item...
        // Process...
        osDelay(1);
    }
    
    return NULL;
}
```

---

## nrf_driver.c - NRF Threads (rx_thread, tx_thread)

If NRF driver is used:

```c
// Step 1: Add both buffers
static StaticTask_t rx_thread_buffer;
static StackType_t rx_thread_stack[2048];
static StaticTask_t tx_thread_buffer;
static StackType_t tx_thread_stack[256];

// Step 2: Create in nrf_init()
osThreadNew((osThreadFunc_t)rx_thread, NULL,
    &(const osThreadAttr_t){
        .name = "nrf_rx",
        .priority = osPriorityNormal,
        .stack_mem = rx_thread_stack,
        .stack_size = sizeof(rx_thread_stack),
        .cb_mem = &rx_thread_buffer,
        .cb_size = sizeof(rx_thread_buffer)
    });

osThreadNew((osThreadFunc_t)tx_thread, NULL,
    &(const osThreadAttr_t){
        .name = "nrf_tx",
        .priority = osPriorityNormal,
        .stack_mem = tx_thread_stack,
        .stack_size = sizeof(tx_thread_stack),
        .cb_mem = &tx_thread_buffer,
        .cb_size = sizeof(tx_thread_buffer)
    });

// Step 3: Convert both functions (radio RX/TX loops)
static void *rx_thread(void *arg) {
    (void)arg;
    while(1) {
        // Receive radio packet
        osDelay(1);
    }
    return NULL;
}

static void *tx_thread(void *arg) {
    (void)arg;
    while(1) {
        // Send radio packet
        osDelay(1);
    }
    return NULL;
}
```

---

## Search/Replace Patterns (for quick conversion)

Use your editor's Find & Replace (Ctrl+H in VS Code):

### Pattern 1: Remove chRegSetThreadName
**Find**: `\s+chRegSetThreadName\(".*?"\);`  (regex enabled)
**Replace**: `` (empty)

### Pattern 2: Change for(;;) in threads
**Find**: `for\(\;\;\)`  (DO THIS CAREFULLY - only in thread functions)
**Replace**: `while(1)`

### Pattern 3: Replace chThdSleep
**Find**: `chThdSleep\(MS2ST\((\d+)\)\)`
**Replace**: `osDelay($1)`

**Find**: `chThdSleepMilliseconds\((\d+)\)`
**Replace**: `osDelay($1)`

### Pattern 4: Mutex operations
**Find**: `chMtxLock\(&(\w+)\)`
**Replace**: `osMutexAcquire($1, osWaitForever)`

**Find**: `chMtxUnlock\(&(\w+)\)`
**Replace**: `osMutexRelease($1)`

---

## Testing After Each File Conversion

1. **Compile check**: `platformio run`
2. **No undefined refs**: Search for `chThdCreate`, `chMtxLock`, `chEvt` in error output
3. **Smoke test**: If main.c boots, quickly test affected subsystem
4. **Check syntax**: Ensure all `return NULL` statements are present in threads

---

## Common Pitfalls (Double-Check!)

- [ ] Did you add `return NULL;` at end of each thread function?
- [ ] Did you remove ALL `chRegSetThreadName` calls?
- [ ] Did you change `chMtxLock(&var)` to `osMutexAcquire(var, ...)`? (no & symbol!)
- [ ] Did you remove old `THD_WORKING_AREA` declarations?
- [ ] Did you add `static StaticTask_t` and `StackType_t[]` buffers?
- [ ] Did you add `osThreadNew()` calls in init functions?
- [ ] Did you check for `chEvtWaitAny` and replace with `osEventFlagsWait`?

---

## Validation After Complete Conversion

Run this grep to verify:
```bash
# Should return 0 results (no ChibiOS APIs left):
grep -rn "chThdCreate\|chMtxLock\|chEvt\|THD_FUNCTION\|THD_WORKING_AREA" Src/ --include="*.c"

# Should return many results (FreeRTOS APIs):
grep -rn "osThreadNew\|osMutexAcquire\|osEventFlags" Src/ --include="*.c"
```

---

## Troubleshooting

**ERROR: undefined reference to `chThdCreateStatic`**
- You missed replacing a thread creation call
- Check ALL `_init()` functions and `_start()` functions

**ERROR: `osStatus_t` issues**
-  Return type changed - check `osMutexAcquire`, `osEventFlagsWait` usage
- These return `osStatus_t` / `uint32_t` which need checking

**Compilation works but motor doesn't run**
- Check that `app_adc_thread` was converted (critical for throttle)
- Verify `osKernelStart()` is in main.c after all thread creation
- Check FreeRTOSConfig.h priorities match expectations

