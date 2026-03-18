---
name: VESC ChibiOS to HAL F103 Conversion Orchestration
description: Master guide for converting VESC dual motor from ChibiOS/PAL to STM32F1 HAL and FreeRTOS CMSIS v2
---

# VESC Dual Motor: ChibiOS → HAL F103 + FreeRTOS CMSIS v2 Migration

## ⚠️ CRITICAL: NO ABSTRACTION WRAPPERS APPROACH

**Policy**: Direct API replacement, NO wrapper abstraction layers.

- ❌ **DO NOT CREATE**: Abstract `hal_gpio_t`, `rtos_mutex_t`, `driver_i2c_t` wrappers
- ✅ **DO THIS INSTEAD**: Replace all ChibiOS calls with direct STM32 HAL + FreeRTOS CMSIS v2 API calls
  - `palReadPad(port, pin)` → `hal_gpio_read(&pin_config)` (simple port+pin wrapper OK)
  - `chMtxLock(&mtx)` → `osMutexAcquire(mtx, osWaitForever)` (direct replacement)
  - `i2cMasterTransmit()` → `i2c_bb_tx_rx()` or `HAL_I2C_Master_Transmit()` (HAL direct)
  - `adcStart()` → `HAL_ADC_Init()` + `HAL_ADC_Start_IT()` (HAL direct)

**Rationale**: Explicit control, minimal indirection, easy debugging, fast execution.

---

## Executive Summary

This document orchestrates a **4-agent conversion** of the VESC dual motor controller firmware from ChibiOS/PAL HAL to STM32F103 native HAL + FreeRTOS CMSIS v2 RTOS.

**Scope**: Dual motor control (24 concurrent tasks), gate drivers, encoders, CAN/USB communication, fault protection
**Timeline**: ~5-15 person-days (phases 1-4)  
**Risk**: Low (well-layered architecture); Critical path = motor ISR latency validation
**Outcome**: Deterministic real-time system ready for field deployment
**Approach**: **NO WRAPPER ABSTRACTION** - direct HAL/FreeRTOS API replacement

---

## The 4 Specialized Agents

| Agent | Focus | Owner | Est. Effort | Dependencies |
|-------|-------|-------|-------------|--------------|
| **Agent 1: HAL GPIO** | Abstraction layer replacement | GPIO driver specialist | 2-3 days | None |
| **Agent 2: RTOS Thread** | Kernel + task architecture | Real-time specialist | 3-5 days | Agent 1 |
| **Agent 3: ISR Motor Control** | Time-critical hardware timing | Motor control specialist | 2-3 days | Agent 1 + 2 |
| **Agent 4: Drivers & Peripherals** | I2C/SPI/CAN/USB/encoders | Peripheral specialist | 3-5 days | Agent 1 + 2 |

**Execution order**: Sequential (Agent 1 → 2 → 3 → 4), with parallel sub-tasks where possible.

---

## Phase-by-Phase Conversion Plan

### PHASE 0: Preparation (0.5 days)

**Before any agent starts:**

✅ Create file structure:
```
.github/
├── agents/
│   ├── 01-hal-gpio-migration.agent.md ✓
│   ├── 02-rtos-thread-architecture.agent.md ✓
│   ├── 03-isr-motor-control.agent.md ✓
│   └── 04-drivers-peripherals.agent.md ✓
└── AGENTS.md (this file)

include/
├── CMSIS/              <- Reference HAL headers
├── RTOS/               <- Reference RTOS headers
└── STM32F1xx_HAL_Driver/  <- Reference drivers

Src/
├── FreeRTOSConfig.h (NEW) ← Agent 2
├── cmsis_rtos_compat.h (NEW) ← Agent 2
├── hwconf/
│   ├── hal_gpio.h (NEW) ← Agent 1
│   ├── hal_gpio.c (NEW) ← Agent 1
│   └── pins.h (NEW) ← Agent 1
├── motor/
│   ├── mcpwm_hal_pwm.c (NEW) ← Agent 3
│   ├── mcpwm_hal_adc.c (NEW) ← Agent 3
│   └── mcpwm_hal_fault.c (NEW) ← Agent 3
├── driver/
│   └── spi_hw.c (NEW) ← Agent 4
├── comm/
│   └── (modified by Agent 4)
└── imu/
    └── (modified by Agent 4)
```

**Baseline**: Commit current ChibiOS version as `refs/baseline/v1-chibi`

**Tools needed**:
- STM32 HAL driver library (v1.3+)
- FreeRTOS kernel (v10.4+)
- CMSIS-RTOS v2 wrapper (`cmsis_os2.h`)
- Oscilloscope (for motor ISR timing validation)
- CAN analyzer (optional, for comm validation)

---

### PHASE 1: HAL GPIO Abstraction Layer (Days 1-3)

**Agent 1 deliverables**:

1. **Create `Src/hwconf/hal_gpio.h/c`**
   - Core functions: `hal_gpio_init()`, `hal_gpio_read()`, `hal_gpio_write()`, `hal_gpio_toggle()`
   - ISR setup: `hal_gpio_irq_enable()`, callback dispatcher
   - Inlines for hot-path (ISR gate outputs via direct register access)

2. **Create `Src/hwconf/pins.h`**
   - Centralized pin definitions: `gate_m1_uh`, `gate_m2_uh`, `fault_m1`, `fault_m2`, `led`, `hall_*`, etc.
   - Board-specific mapping from existing hwconf files

3. **Update `Src/hwconf/board.c`**
   - Replace all `palSetPadMode()` → `hal_gpio_init()`
   - Implement `hw_init_gpio()` using `hal_gpio_init()` for all pins
   - Setup ISR callbacks for fault detection

4. **Verify no breaking changes**:
   - Search entire codebase for `palReadPad`, `palWritePad`, `palSetPadMode`
   - Replace all occurrences (except motor ISR gate drives - use direct register macros)

**Validation**:
- LED toggles on/off
- Button press detected
- Fault input triggers ISR
- Gate output timing unchanged (oscilloscope test)
- Dual motor GPIO behavior synchronized

**Acceptance**: All GPIO I/O functional; compile with `-Wall -Werror`

---

### PHASE 2: FreeRTOS RTOS Architecture (Days 4-8)

**Agent 2 deliverables**:

1. **Create `Src/FreeRTOSConfig.h`**
   - Scheduler: 1 kHz tick, 7 priority levels, max 30 tasks
   - Memory: static allocation only, 16 KB heap
   - Synchronization: mutexes with priority inheritance, event flags, semaphores

2. **Create `Src/cmsis_rtos_compat.h`** (macro abstraction)
   - Backward-compatible wrappers for existing code:
     - `rtos_mutex_lock()`, `rtos_mutex_unlock()`
     - `rtos_sem_wait()`, `rtos_sem_signal()`
     - `rtos_event_wait()`, `rtos_event_set()`
     - `rtos_delay_ms()`, `rtos_delay_ticks()`

3. **Rewrite `Src/main.c`**
   - Initialize FreeRTOS kernel
   - Create 25 static tasks in priority order:
     - Motor control (prio 6) ×1
     - FOC observer (prio 6) ×1
     - CAN RX threads (prio 5) ×5
     - CAN TX (prio 4) ×1
     - USB RX (prio 5) ×1
     - Encoder (prio 4) ×1
     - IMU (prio 3) ×1
     - App ADC (prio 3) ×1
     - Watchdog (prio 5) ×1
     - Telemetry (prio 2) ×1
     - LED blink (prio 1) ×1
     - **Total: 24 tasks + idle = 25**
   - Call `osKernelStart()` to begin scheduling

4. **Convert synchronization in all modules**:
   - Replace `chMtxLock()` → `osMutexAcquire()`
   - Replace `chEvtWaitAny()` → `osEventFlagsWait()`
   - Replace `chEvtSignal()` → `osEventFlagsSet()`
   - Update all thread functions to `void *thread_name(void *arg)`

5. **Update task entry points**:
   - `motor_control_thread()` → FOC feedback loop
   - `can_rx_thread()` → CAN message dispatcher (×5 instances)
   - `usb_rx_thread()` → UART/USB command line
   - `encoder_thread()` → Position polling
   - `imu_thread()` → Sensor reading
   - `app_adc_thread()` → Input processing (throttle, PAS, etc.)
   - Utility threads: LED, watchdog, telemetry

**Validation**:
- All tasks created and scheduled
- Mutex priority inheritance prevents inversion
- Event flag signaling works (monitor task states)
- Thread priorities enforced (higher prio preempts lower)
- No deadlocks over 60-second runtime
- Task stack usage < 70% for all tasks

**Acceptance**: 25 tasks running, kernel tick stable at 1 kHz, no priority inversions observed

---

### PHASE 3: Motor ISR & Hardware Timing (Days 9-11)

**Agent 3 deliverables**:

1. **Create hardware timer/ADC configuration**:
   - `Src/motor/mcpwm_hal_pwm.c`: TIM1/TIM8 PWM 16 kHz, complementary outputs, dead-time 1 µs
   - `Src/motor/mcpwm_hal_adc.c`: ADC injected conversions triggered by TIM1 CC4 event
   - `Src/motor/mcpwm_hal_fault.c`: Comparator OCP shutdown or GPIO fault detection

2. **Preserve ISR latency** (CRITICAL):
   - ADC ISR: `ADC1_2_3_IRQHandler()` → ADC callback → FOC observer update in <2 µs
   - Direct register access for PWM duty (TIM1->CCR1-3, TIM8->CCR1-3)
   - Signal FOC observer task via FreeRTOS task notification (not mutex)
   - No FreeRTOS locks in ISR critical path (only `taskENTER_CRITICAL()` if needed)

3. **Update motor control loop**:
   - Keep FOC observer computation in ISR context (fast path)
   - Defer slower filtering to `foc_observer_thread` task
   - Motor speed/torque estimation runs in separate TIM2 ISR
   - Dual motor PWM updates synchronized (TIM1 master, TIM8 slave via hardware trigger)

4. **Integrate with hwconf/board.c**:
   - Call `mcpwm_hal_tim1_init()`, `mcpwm_hal_tim8_init()`, `mcpwm_hal_adc_init()` in `hw_init()`
   - Verify TIM1 ↔ TIM8 hardware synchronization before starting ISRs

5. **Update ISR dispatcher** (`Src/stm32f1xx_it.c`):
   - Route hardware interrupts to HAL handlers:
     - `TIM1_UP_IRQHandler()` → `HAL_TIM_IRQHandler(&htim1)`
     - `ADC1_2_3_IRQHandler()` → `HAL_ADC_IRQHandler(&hadc1)`
     - `TIM8_UP_IRQHandler()` → `HAL_TIM_IRQHandler(&htim8)`
   - HAL calls registered callbacks: `HAL_ADC_ConvCpltCallback()`, etc.

**Validation** (MOST CRITICAL):
- PWM frequency = 16 kHz (oscilloscope, measure period = 62.5 µs)
- ADC ISR fires at exactly 6 kHz (1/6 of PWM frequency trigger)
- PWM duty update latency from ADC sample: <2 µs (cycle counter measurement)
- Dual motor gate outputs phase-locked at 180° (oscilloscope, simultaneous dual capture)
- Motor commutation smooth (no cogging, FOC running)
- Fault input triggers PWM shutdown within 1 ISR cycle

**Acceptance**: Motor runs at full power without ISR latency regression; dual motor phases synchronized

---

### PHASE 4: Peripheral Drivers (Days 12-15)

**Agent 4 deliverables**:

1. **I2C Bit-Bang** (`Src/driver/i2c_bb.c`):
   - Pure GPIO-based I2C (400 kHz)
   - Mutex protects against concurrent encoder/IMU reads
   - Clock stretching support
   - No ChibiOS dependency

2. **Hardware SPI** (`Src/driver/spi_hw.c`):
   - HAL SPI interface for gate driver chips (DRV8301, etc.)
   - Parameterized baud rate, polarity, phase
   - Chip select via GPIO

3. **CAN Bus** (`Src/comm/comm_can.c`):
   - STM32 HAL CAN driver (1 Mbps, configurable)
   - RX FIFO0 interrupt signals CAN RX thread
   - libcanard integration (optional, for UAVCAN)

4. **USB CDC Serial** (`Src/comm/comm_usb_serial.c`):
   - STM32CubeMX USB stack (or HAL native)
   - USB enumeration and CDC class
   - RX callback signals USB RX thread

5. **Encoders** (`Src/encoder/*.c`):
   - `encoder.c`: Common interface with thread-safe position/velocity
   - `enc_as504x.c`: SPI magnetic encoder
   - `enc_abi.c`: Hall sensor incremental encoder
   - Other encode types as needed (TS5700, BiSSC, etc.)

6. **IMU** (`Src/imu/imu.c`):
   - I2C sensor driver (MPU6050 or similar)
   - Accelerometer + gyroscope polling in `imu_thread`
   - Thread-safe access via mutex

7. **Fault Handling**:
   - GPIO ISR for fault inputs (OCP, overtemp, driver error)
   - PWM shutdown on fault (direct register write)
   - Graceful recovery with manual reset

**Validation**:
- I2C reads encoder position correctly (matches mechanical angle)
- CAN messages transmit/receive without errors
- USB serial terminal responsive (>9600 baud, no dropped chars)
- IMU accelerometer reads ~1g on Z axis (gravity check)
- Fault input triggers PWM shutdown
- All peripherals work together without resource conflicts

**Acceptance**: All peripherals functional; no collisions between concurrent drivers

---

## Validation Strategy

### Tier 1: Unit Tests (Agent responsibilities)

Each agent performs isolated tests within their domain:
- **Agent 1**: LED toggle, GPIO read/write, ISR callbacks
- **Agent 2**: Task creation, mutex locking, event signals, no starvation
- **Agent 3**: ADC ISR latency, PWM frequency, dual-motor phase sync
- **Agent 4**: I2C communication, CAN frame exchange, USB enumeration, encoder/IMU reading

### Tier 2: Integration Tests (After Agent completion)

Full system validation:
- Motor runs without shutdown
- Dual motor synchronized
- CAN/USB communication doesn't interfere with motor ISR
- All tasks complete startup without exception
- No priority inversions
- Motor ISR latency unchanged from ChibiOS baseline

### Tier 3: Stress Tests (End of Phase 4)

- Motor hold-down load test (10+ minutes at full current)
- CAN high-frequency burst (1000 msgs/sec)
- USB serial streaming (100 KB/sec)
- Fault injection (OCP, temperature shutdown)
- Thermal stability (sustained operation >30 min)

---

## Integration Checkpoints

**After Agent 1 (day 3)**:
- [ ] Compile passes: no `pal*` references except motor ISR gate writes
- [ ] All GPIO functional
- [ ] LED and button respond correctly

**After Agent 2 (day 8)**:
- [ ] FreeRTOS kernel runs for 60 seconds without exception
- [ ] All 25 tasks scheduled correctly
- [ ] Mutex priority inheritance verified
- [ ] Task stack usage <70%

**After Agent 3 (day 11)**:
- [ ] Motor PWM at 16 kHz
- [ ] ADC ISR fires at 6 kHz
- [ ] PWM latency <2 µs from ADC sample
- [ ] Dual motor phases 180° synchronized
- [ ] Motor runs under no load smoothly

**After Agent 4 (day 15)**:
- [ ] Encoder position reads correctly
- [ ] CAN bus transmits/receives
- [ ] USB serial console works
- [ ] All drivers coexist without resource conflicts
- [ ] Motor operates with CAN/USB active
- [ ] Fault detection triggers properly

---

## Risk Mitigation

| Risk | Severity | Mitigation |
|------|----------|-----------|
| Motor ISR latency regression | **CRITICAL** | Measure with cycle counter, preserve direct register writes, skip task-based protection |
| FreeRTOS memory overrun | HIGH | Static allocation only, validate stack sizes, monitor free heap |
| Dual motor desynchronization | HIGH | Hardware timer trigger linking (unchanged), verify phase offset at boot |
| I2C bus hang (clock stretching) | MEDIUM | Implement timeout, GPIO bus recovery sequence |
| CAN frame loss | MEDIUM | RX FIFO interrupt, no task preemption during handler |
| USB cable unplugged mid-session | LOW | Graceful disconnect handler, automatic re-enumeration |
| Encoder noise | LOW | Filter in software, use hardware debouncing (if available) |

---

## File Checklist: Creation & Modification

### New Files (Agent 1)
- [ ] `Src/hwconf/hal_gpio.h` - GPIO abstraction
- [ ] `Src/hwconf/hal_gpio.c` - GPIO implementation
- [ ] `Src/hwconf/pins.h` - Centralized pin definitions

### New Files (Agent 2)
- [ ] `Src/FreeRTOSConfig.h` - Kernel configuration
- [ ] `Src/cmsis_rtos_compat.h` - CMSIS-RTOS v2 wrappers

### New Files (Agent 3)
- [ ] `Src/motor/mcpwm_hal_pwm.c` - TIM1/TIM8 PWM setup
- [ ] `Src/motor/mcpwm_hal_adc.c` - ADC injected config
- [ ] `Src/motor/mcpwm_hal_fault.c` - Fault protection ISR

### New Files (Agent 4)
- [ ] `Src/driver/spi_hw.c` - Hardware SPI interface
- [ ] `Src/libcanard/canard_hal.c` - UAVCAN bridge (optional)

### Modified Files (Agent 1)
- [ ] `Src/hwconf/board.c` - Replace PAL init with HAL
- [ ] All files with `palReadPad`, `palWritePad`, `palSetPadMode`

### Modified Files (Agent 2)
- [ ] `Src/main.c` - Task creation, kernel start
- [ ] All files with `chMtxLock`, `chEvt*`, `chThdCreate*`

### Modified Files (Agent 3)
- [ ] `Src/motor/mcpwm_foc.c` - ISR callbacks, latency
- [ ] `Src/stm32f1xx_it.c` - ISR routing to HAL

### Modified Files (Agent 4)
- [ ] `Src/driver/i2c_bb.c` - GPIO-based I2C
- [ ] `Src/comm/comm_can.c` - HAL CAN driver
- [ ] `Src/comm/comm_usb_serial.c` - HAL USB CDC
- [ ] `Src/encoder/encoder.c` - Thread-safe interface
- [ ] `Src/encoder/enc_*.c` - Specific encoder drivers
- [ ] `Src/imu/imu.c` - I2C IMU reader

---

## Success Criteria (Final)

✅ **Build**: Compiles with `-Wall -Werror`, no warnings
✅ **Runtime**: Kernel scheduler runs for 1+ hour without exception
✅ **Motor**: Runs at full power, latency <2 µs from ADC to PWM update
✅ **Dual Motor**: Gate outputs synchronized (180° phase, measurable on oscilloscope)
✅ **Communication**: 100 CAN frames/sec + USB streaming without errors
✅ **Peripherals**: Encoder, IMU, fault detection all operational
✅ **Tasks**: 25 concurrent tasks, no deadlocks, priority inheritance working
✅ **Memory**: Stack usage <70%, no heap fragmentation
✅ **Timing**: ADC ISR fires at 6 kHz ±1%, PWM at 16 kHz ±0.1%
✅ **Fault Handling**: OCP/overtemp triggers safe shutdown, recovery possible

---

## Rollback Plan

If any agent's work causes regression:

1. Identify failing agent (1, 2, 3, or 4)
2. Revert changes in that agent's file list
3. Re-run validation tests
4. Root-cause analysis and fix
5. Re-run full integration tests

**Git branch strategy**:
```bash
git branch agent-1-gpio     # After Agent 1 complete
git branch agent-2-rtos     # After Agent 2 complete
git branch agent-3-isr      # After Agent 3 complete
git branch agent-4-drivers  # After Agent 4 complete (ready to merge main)
```

---

## Reference Documentation

**ChibiOS to FreeRTOS API Mapping**:
- ChibiOS threads (chThdCreateStatic) → FreeRTOS osThreadNew()
- ChibiOS mutexes (chMtxLock) → FreeRTOS osMutexAcquire()
- ChibiOS events (chEvtWaitAny) → FreeRTOS osEventFlagsWait()
- ChibiOS semaphores (chSemWait) → FreeRTOS osSemaphoreAcquire()

**STM32F1 HAL Key Functions**:
- GPIO: `HAL_GPIO_Init()`, `HAL_GPIO_ReadPin()`, `HAL_GPIO_WritePin()`
- Timer: `HAL_TIM_PWM_Init()`, `HAL_TIM_PWM_Start()`, `HAL_TIM_IRQHandler()`
- ADC: `HAL_ADC_Init()`, `HAL_ADCEx_InjectedConfigChannel()`, `HAL_ADC_Start_IT()`
- CAN: `HAL_CAN_Init()`, `HAL_CAN_AddTxMessage()`, `HAL_CAN_GetRxMessage()`
- I2C: `HAL_I2C_Master_Transmit()`, `HAL_I2C_Master_Receive()`

**CMSIS-RTOS v2 Key Functions**:
- `osKernelInitialize()`, `osKernelStart()`
- `osThreadNew()`, `osThreadResume()`, `osThreadSuspend()`
- `osMutexNew()`, `osMutexAcquire()`, `osMutexRelease()`
- `osEventFlagsNew()`, `osEventFlagsSet()`, `osEventFlagsWait()`
- `osDelay()`, `osDelayUntil()`

---

## Communication & Progress Tracking

**Daily standup**:
- Agent 1: GPIO abstraction layer status
- Agent 2: ROS thread migration, task creation
- Agent 3: Motor ISR latency validation
- Agent 4: Driver integration, peripheral functionality

**Weekly validation**:
- Run all tier 1 + 2 tests
- Document any blockers or risks
- Adjust timeline if needed

**Completion sign-off**:
- All agents report green✓ on their respective checklists
- Integration tests pass
- Code review complete
- Ready for field testing

---

## Next Steps

1. **Assign agents**: One specialist per agent (or single developer responsible for all 4 if solo)
2. **Timeline planning**: Schedule agents to start sequentially (Agent 1 first, Agent 4 last)
3. **Tool setup**: Ensure oscilloscope, CAN analyzer, USB serial terminal available
4. **Baseline commit**: Commit current ChibiOS version before starting Agent 1
5. **Daily testing**: Run validation tests at end of each working day

---

**Document Version**: 1.0  
**Last Updated**: March 2026  
**Status**: Ready for Agent 1 (GPIO) to start

