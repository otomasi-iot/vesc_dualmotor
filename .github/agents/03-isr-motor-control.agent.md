---
name: ISR & Motor Control Migration Agent
description: |
  Specialized agent for converting time-critical hardware interrupt handlers to HAL F103.
  Use when: Converting STM32 ISRs (ADC, timers, PWM) from ChibiOS HAL to STM32 HAL.
  Handles: Motor control ISR context, dual-motor synchronization, timer PWM setup, ADC triggering.
  Critical: Preserves microsecond-level latency in current control loops.
  Scope: Src/motor/mcpwm_*.c, Src/irq_handlers.c, timer/PWM configuration in hwconf/
---

# ISR & Motor Control HAL Conversion

## Mission
Convert motor control ISR handlers from ChibiOS HAL to STM32F103 HAL while maintaining:
- **Sub-microsecond timing**: ADC ISR latency <2 µs (duty update synchronous with sampling)
- **Dual-motor hardware sync**: TIM1 (M1) master ↔ TIM8 (M2) slave phase locking
- **PWM dead-time**: Gate driver timing unchanged (hardware-managed)
- **Current feedback**: Injected ADC conversion triggers from timer events
- **Fault protection**: Comparator shutdown or GPIO override of PWM
- **Raw ISR context**: No FreeRTOS mutex/semaphore calls in critical path (use taskENTER_CRITICAL() instead)

---

## Architecture: Motor Control ISRs

The VESC dual motor uses two hardware interrupt sources:

```
    ADC1_2_3_IRQHandler (6 kHz = 166 µs period)
    ├─ Samples three-phase motor current (IU, IV, IW)
    ├─ Updates FOC observer state (observr.phase, observr.speed)
    └─ Writes TIM1->CCR1-3, TIM8->CCR1-3 (PWM duty) [SYNCHRONOUS]
    
    TIM2_IRQHandler (20 kHz timer comparison)
    └─ Updates motor speed/torque estimates
```

**Why raw ISRs?**
- FreeRTOS task wakeup latency: ~1.2 µs (unacceptable for <2 µs latency requirement)
- Mutex/semaphore operations forbidden in ISR high-priority context
- Motor PWM duty must update same cycle as ADC sample (0-cycle latency requirement)

---

## FASE 1: KONVERSI TIMER & ISR - NO WRAPPER, DIRECT REPLACEMENT

**PENTING**: Tidak ada wrapper. Semua timer/ADC configuration langsung ke HAL calls. ISR handler langsung HAL callback.

### LANGKAH 1: Cari file mcpwm_foc.c atau motor control files dengan timer/ADC setup

**CARI CODE INI di Src/motor/mcpwm_foc.c atau hwconf file**:
```c
// Old ChibiOS ADC driver init
adcStart(&ADCD1, &adc_config);
// Old timer setup
pwmStart(&PWMD1, &pwm_config);
```

**GANTI DENGAN - COPY EXACTLY THIS**:

**Tambah di atas include di Src/motor/mcpwm_foc.c**:
```c
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_tim.h"

// Timer handles (global)
TIM_HandleTypeDef htim1, htim8;
ADC_HandleTypeDef hadc1;
```

**Tempat di mana adcStart/pwmStart dipanggil, ganti dengan**:
```c
void mcpwm_init_hardware(void) {
    // ===== TIM1 MOTOR 1 PWM =====
    __HAL_RCC_TIM1_CLK_ENABLE();
    
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 4500 - 1;  // 16 kHz
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);
    
    // PWM channels 1, 2, 3
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 2250;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);
    
    // Break & Dead Time
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIdleMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 72;  // 1 us
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_LOW;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
    
    // Start PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    
    // ===== TIM8 MOTOR 2 PWM (SLAVE) =====
    __HAL_RCC_TIM8_CLK_ENABLE();
    
    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 0;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 4500 - 1;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim8);
    
    // Slave mode trigger from TIM1
    LL_TIM_SetSlaveMode(TIM8, LL_TIM_SLAVEMODE_TRIGGER);
    LL_TIM_SetTriggerInput(TIM8, LL_TIM_TS_ITR0);
    
    // Phase offset 180 degrees
    TIM8->CNT = 2250;
    
    // Same PWM config
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig);
    
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);
    
    // ===== ADC1 SETUP =====
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.ContinuousConvMode = ADC_CONTINUOUS_DISABLE;
    hadc1.Init.DiscontinuousConvMode = ADC_DISCONTINUOUS_DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC4;
    hadc1.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc1);
    
    // Injected channels
    ADC_InjectionConfTypeDef sConfigInj = {0};
    sConfigInj.InjectedChannel = ADC_CHANNEL_0;
    sConfigInj.InjectedRank = ADC_INJECTED_RANK_1;
    sConfigInj.InjectedNbrOfConversion = 3;
    sConfigInj.InjectedSamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    sConfigInj.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJECCONV_T1_TRGO;
    HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInj);
    
    sConfigInj.InjectedChannel = ADC_CHANNEL_1;
    sConfigInj.InjectedRank = ADC_INJECTED_RANK_2;
    HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInj);
    
    sConfigInj.InjectedChannel = ADC_CHANNEL_2;
    sConfigInj.InjectedRank = ADC_INJECTED_RANK_3;
    HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInj);
    
    HAL_ADC_Start_IT(&hadc1);
    HAL_ADCEx_Calibration_Start(&hadc1);
}
```

**Panggil function ini di main() sebelum task dimulai**:
```c
void main(void) {
    hw_init();
    mcpwm_init_hardware();  // ADD THIS
    // ... rest of init
}
```

---

## Phase 2: ADC Interrupt Handler Conversion

### 2.1 ADC ISR (6 kHz - Time-Critical)

**File**: `Src/motor/mcpwm_foc.c` → Update ADC1_2_3_IRQHandler

**Before (ChibiOS HAL)**:
```c
// BEFORE: ChibiOS ADC callback
void adcEndCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
    // Sample from buffer
    int32_t iu = buffer[0] - 2048;
    int32_t iv = buffer[1] - 2048;
    int32_t iw = buffer[2] - 2048;
    
    // FOC observer update
    mcpwm_foc_observer_update(iu, iv, iw);
    
    // Output PWM duty
    TIM1->CCR1 = m1_duty_u;
    TIM1->CCR2 = m1_duty_v;
    TIM1->CCR3 = m1_duty_w;
    
    if (dual_motor_enabled) {
        TIM8->CCR1 = m2_duty_u;
        TIM8->CCR2 = m2_duty_v;
        TIM8->CCR3 = m2_duty_w;
    }
    
    // Signal FOC observer thread
    chEvtSignal(foc_observer_thread_ptr, FOC_READY);
}
```

**After (STM32 HAL + FreeRTOS)**:
```c
// Global volatile variables (updated by ISR, read by thread)
volatile int32_t g_adc_iu, g_adc_iv, g_adc_iw;
volatile uint32_t g_adc_sample_count = 0;

// ISR handler (6 kHz)
void ADC1_2_3_IRQHandler(void) {
    // HAL dispatcher (automatically handles JEOC interrupt)
    HAL_ADC_IRQHandler(&hadc1);
}

// ISR end-of-conversion callback (called by HAL)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        // Read ADC samples from injected DR registers
        g_adc_iu = (int16_t)ADC1->JDR1 - 2048;
        g_adc_iv = (int16_t)ADC1->JDR2 - 2048;
        g_adc_iw = (int16_t)ADC1->JDR3 - 2048;
        g_adc_sample_count++;
        
        // === CRITICAL SECTION (<2 µs) ===
        // FOC observer update (in-ISR computation)
        mcpwm_foc_observer_update(g_adc_iu, g_adc_iv, g_adc_iw);
        
        // Immediately write PWM duty (synchronous with ADC sample)
        mcpwm_pwm_set_m1(m1_duty_u, m1_duty_v, m1_duty_w);
        
        if (dual_motor_enabled) {
            mcpwm_pwm_set_m2(m2_duty_u, m2_duty_v, m2_duty_w);
        }
        // === END CRITICAL SECTION ===
        
        // Signal observer task (LOW PRIORITY)
        // Use FreeRTOS task notification instead of semaphore
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(foc_observer_task_handle, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}
```

**Key optimization**: Observer update stays in ISR context for `<2 µs` latency; slower filtering is deferred to task

### 2.2 Timer Interrupt Handler (20 kHz Optional)

**File**: `Src/motor/mcpwm_foc.c` → TIM2_IRQHandler

```c
// TIM2 used for speed/torque estimation (lower priority than ADC)
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        // Speed/torque update (lower priority computation)
        mcpwm_foc_speed_estimator_update();
        
        // Signal telemetry thread (optional)
        osEventFlagsSet(telemetry_events, TELEMETRY_READY);
    }
}
```

---

## Phase 3: Fault Detection & Protection

### 3.1 Hardware Comparator Shutdown (OCP - Over-Current Protection)

**If using STM32F1 internal comparators**:

```c
// Src/motor/mcpwm_hal_fault.c

#include "stm32f1xx_hal.h"

COMP_HandleTypeDef hcomp1, hcomp2;

void mcpwm_hal_fault_init(void) {
    // Comparator 1: Motor 1 OCP (gate shutdown)
    hcomp1.Instance = COMP1;
    hcomp1.Init.InvertingInput = COMP_INVERTINGINPUT_VREFINT;  // Ref: 1.2V
    hcomp1.Init.Output = COMP_OUTPUT_TIM1BKIN;  // Auto-shutdown TIM1
    hcomp1.Init.Mode = COMP_MODE_HIGHSPEED;
    HAL_COMP_Init(&hcomp1);
    
    // Comparator 2: Motor 2 OCP
    hcomp2.Instance = COMP2;
    hcomp2.Init.InvertingInput = COMP_INVERTINGINPUT_VREFINT;
    hcomp2.Init.Output = COMP_OUTPUT_TIM8BKIN;  // Auto-shutdown TIM8
    hcomp2.Init.Mode = COMP_MODE_HIGHSPEED;
    HAL_COMP_Init(&hcomp2);
    
    HAL_COMP_Start(&hcomp1);
    HAL_COMP_Start(&hcomp2);
}

// ISR for external fault input (if GPIO-based)
void EXTI_OCP_IRQHandler(void) {
    // Disable PWM outputs
    __HAL_TIM_MOE_DISABLE(&htim1);  // Master Output Disable
    __HAL_TIM_MOE_DISABLE(&htim8);
    
    // Log fault and trigger recovery sequence
    fault_occurred = 1;
    fault_code = FAULT_OCP;
}
```

### 3.2 Software Fault Flags

```c
// Src/motor/mcpwm_foc.c - Add to observer update

typedef enum {
    FAULT_NONE = 0,
    FAULT_OCP = 0x01,       // Over-current
    FAULT_OVR = 0x02,       // Over-voltage
    FAULT_TMP = 0x04,       // Over-temperature
    FAULT_DRV = 0x08        // Gate driver error
} fault_t;

volatile fault_t g_fault_status = FAULT_NONE;

void mcpwm_foc_observer_update(int32_t iu, int32_t iv, int32_t iw) {
    // Check current magnitude
    int32_t i_mag_sq = iu*iu + iv*iv + iw*iw;
    if (i_mag_sq > (MAX_CURRENT * MAX_CURRENT)) {
        g_fault_status |= FAULT_OCP;
        // Disable PWM (optional)
        __HAL_TIM_MOE_DISABLE(&htim1);
        return;  // Skip observer update
    }
    
    // Normal observer processing...
    observr.phase += observr.phase_vel * dt;
    observr.speed = estimator_speed(iu, iv, iw);
}
```

---

## Phase 4: Dual Motor Synchronization Verification

### 4.1 Hardware Sync Check

**In hwconf/board.c**:

```c
void hw_verify_dual_motor_sync(void) {
    // Measure phase offset between TIM1 and TIM8 outputs
    // Expected: 180° (2250 counts at 4500-count period)
    
    uint16_t tim1_cnt = TIM1->CNT;
    uint16_t tim8_cnt = TIM8->CNT;
    int16_t phase_diff = tim8_cnt - tim1_cnt;
    
    if (phase_diff < (2250 - 50) || phase_diff > (2250 + 50)) {
        // Phase misalignment detected
        printf("ERROR: TIM8 phase out of sync (expected 2250, got %d)\n", tim8_cnt);
        __BKPT();
    } else {
        printf("✓ Dual motor phase locked (TIM1=%u, TIM8=%u)\n", tim1_cnt, tim8_cnt);
    }
}
```

### 4.2 Current Sampling Dual Motor

```c
// If ADC2 and ADC3 are used for Motor 2:
// Modify mcpwm_hal_adc.c to setup all three ADCs

void mcpwm_hal_adc_init_dual(void) {
    // ADC1 for Motor 1 (IU, IV, IW)
    mcpwm_hal_adc_init();
    
    // ADC2 & ADC3 for Motor 2 (parallel conversion)
    // Same injected configuration, triggered by TIM8 instead of TIM1
    hadc2.Instance = ADC2;
    hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T8_CC4;  // TIM8 trigger
    HAL_ADC_Init(&hadc2);
    // ... configure injected channels for Motor 2 ...
    
    // Synchronize start of ADC1 and ADC2
    HAL_ADCEx_MultiModeStart_DMA(&hadc1, &hadc2, buffer_dual, ADC_BUFF_SIZE);
}

void mcpwm_adc_irq_handler_m1m2(void) {
    // Called at 6 kHz for both motors
    // Read Motor 1 from ADC1
    mcpwm_adc_read_m1(&g_adc_m1_iu, &g_adc_m1_iv, &g_adc_m1_iw);
    
    // Read Motor 2 from ADC2
    mcpwm_adc_read_m2(&g_adc_m2_iu, &g_adc_m2_iv, &g_adc_m2_iw);
    
    // Update both motors in ISR
    mcpwm_foc_observer_update_m1(g_adc_m1_iu, g_adc_m1_iv, g_adc_m1_iw);
    mcpwm_foc_observer_update_m2(g_adc_m2_iu, g_adc_m2_iv, g_adc_m2_iw);
    
    // Write PWM for both motors
    mcpwm_pwm_set_m1(m1_u, m1_v, m1_w);
    mcpwm_pwm_set_m2(m2_u, m2_v, m2_w);
}
```

---

## Phase 5: Integration with hwconf/board.c

### 5.1 Motor Initialization Sequence

**File**: `Src/hwconf/board.c`

```c
void hw_init(void) {
    // 1. GPIO initialization (from Agent 1)
    hw_init_gpio();
    
    // 2. RCC clock configuration (if needed)
    SystemClock_Config();
    
    // 3. ADC and timer PWM setup
    mcpwm_hal_adc_init();  // Must happen BEFORE motor ISR setup
    mcpwm_hal_tim1_init();
    mcpwm_hal_tim8_init();  // Slave timer
    mcpwm_hal_fault_init();
    
    // 4. ISR vector table (updated in irq_handlers.c)
    // ISRs will be auto-called by HAL on events
}
```

### 5.2 ISR Dispatcher in stm32f1xx_it.c

**File**: `Src/stm32f1xx_it.c` (already exists, update handlers):

```c
#include "stm32f1xx_hal.h"

// Forward declare HAL timer and ADC handles
extern TIM_HandleTypeDef htim1, htim2, htim8;
extern ADC_HandleTypeDef hadc1;

// ISR dispatcher for TIM1 (Update, Compare)
void TIM1_UP_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void TIM1_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

// ISR dispatcher for TIM2
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

// ISR dispatcher for TIM8
void TIM8_UP_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim8);
}

void TIM8_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim8);
}

// ISR dispatcher for ADC1
void ADC1_2_3_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc1);
}

// Fault/DCDC ISR
void EXTI_OCP_IRQHandler(void) {
    // Handle OCP (over-current protection)
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_X);  // Fault input pin
}
```

---

## Phase 6: Testing & Validation

### 6.1 Motor ISR Latency Testing

```c
// Src/tests/test_motor_latency.c

volatile uint32_t adc_irq_time_start, adc_irq_time_end;
volatile uint32_t pwm_update_latency;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    // Measure ISR entry to PWM write
    SystemCoreClockUpdate();
    adc_irq_time_start = DWT->CYCCNT;
    
    // Perform observer update
    mcpwm_foc_observer_update(g_adc_iu, g_adc_iv, g_adc_iw);
    
    // Write PWM
    mcpwm_pwm_set_m1(m1_duty_u, m1_duty_v, m1_duty_w);
    
    adc_irq_time_end = DWT->CYCCNT;
    pwm_update_latency = adc_irq_time_end - adc_irq_time_start;
}

void test_motor_isr_latency(void) {
    // Run motor at steady state for 100 cycles
    vTaskDelay(pdMS_TO_TICKS(20));  // 120 ADC samples @ 6 kHz
    
    uint32_t max_lat = 0;
    uint32_t min_lat = UINT32_MAX;
    for (int i = 0; i < 100; i++) {
        if (pwm_update_latency > max_lat) max_lat = pwm_update_latency;
        if (pwm_update_latency < min_lat) min_lat = pwm_update_latency;
    }
    
    printf("Motor ISR latency: min=%u cycles, max=%u cycles\n", min_lat, max_lat);
    printf("  Min = %.2f µs, Max = %.2f µs @ 72 MHz\n", 
        min_lat / 72.0, max_lat / 72.0);
    
    // PASS if max latency < 200 cycles (2.8 µs)
    if (max_lat < 200) {
        printf("✓ PASS: ISR latency acceptable\n");
    } else {
        printf("✗ FAIL: ISR latency too high\n");
    }
}
```

### 6.2 Dual Motor Synchronization Test

```c
void test_dual_motor_phase(void) {
    uint16_t phase_errors = 0;
    
    for (int i = 0; i < 1000; i++) {
        uint16_t tim1 = TIM1->CNT;
        uint16_t tim8 = TIM8->CNT;
        int16_t phase = tim8 - tim1;
        
        // Expect phase ≈ 2250 ± 100
        if (phase < 2150 || phase > 2350) {
            phase_errors++;
        }
        
        vTaskDelay(1);
    }
    
    printf("Phase sync errors in 1000 samples: %u\n", phase_errors);
    if (phase_errors == 0) {
        printf("✓ PASS: Dual motor phase locked\n");
    }
}
```

### 6.3 PWM Frequency Verification

```c
void test_pwm_frequency(void) {
    // Oscilloscope on TIM1_CH1 (PWM output)
    // Expected frequency: 16 kHz
    // Period: 62.5 µs
    
    printf("Enable PWM at 50%% duty...\n");
    mcpwm_pwm_set_m1(2250, 2250, 2250);
    
    // Manually measure with oscilloscope or high-speed GPIO counter
    // Or use timer input capture
}
```

### 6.4 Completion Checklist

- [ ] STM32 HAL timer and ADC initialized in hw_init()
- [ ] TIM1/TIM8 PWM frequency confirmed at 16 kHz
- [ ] ADC injected conversions trigger from TIM1 CC4
- [ ] ADC ISR handler updates PWM duty (<2 µs latency)
- [ ] Dual-motor phase offset = 2250 counts (±100)
- [ ] Fault interrupt (OCP) triggers PWM shutdown
- [ ] No ChibiOS ADC/timer references remain
- [ ] Motor runs without ISR latency regression on oscilloscope
- [ ] All ISR handlers use HAL callbacks (no direct register access except PWM duty update)
- [ ] stm32f1xx_it.c dispatches ISRs to HAL handlers
- [ ] Observer thread wakes on ADC ISR (task notification from ISR)

---

## Key Files to Create/Modify

| File | Action | Purpose | Priority |
|------|--------|---------|----------|
| `Src/motor/mcpwm_hal_pwm.c` | **CREATE** | TIM1/TIM8 PWM initialization | **CRITICAL** |
| `Src/motor/mcpwm_hal_adc.c` | **CREATE** | ADC injected configuration | **CRITICAL** |
| `Src/motor/mcpwm_hal_fault.c` | **CREATE** | OCP/fault protection setup | **HIGH** |
| `Src/motor/mcpwm_foc.c` | **MODIFY** | ADC/TIM ISR callbacks | **CRITICAL** |
| `Src/hwconf/board.c` | **MODIFY** | Call HAL init functions | **CRITICAL** |
| `Src/stm32f1xx_it.c` | **MODIFY** | Dispatch ISRs to HAL handlers | **CRITICAL** |
| `include/isr_vector_table.h` | **VERIFY** | Correct ISR vector mapping | **HIGH** |

---

## Success Criteria

✅ ADC ISR fires at 6 kHz ±1%
✅ PWM update latency <2 µs (measurable on oscilloscope)
✅ Motor duty cycle matches command (50% duty = 2250 counts)
✅ Dual-motor PWM phases offset by 180°
✅ No jitter in ADC sample timing
✅ Fault detection interrupts PWM within 1 ISR cycle
✅ Motor commutates smoothly (no cogging)
✅ All 3-phase currents sampled synchronously
✅ All register accesses atomic (no race conditions)
✅ ISR callbacks defined and called by HAL automatically

---

## Critical Notes

⚠️ **ISR Latency Requirement**: <2 µs from ADC sample to PWM update
  - Direct register writes (TIM1->CCR1) preserved
  - No FreeRTOS locks in ISR critical path
  - Observer computation must stay in ISR context

⚠️ **Dual Motor Sync**: Hardware timer linking (TIM1 TRGO → TIM8 TRGI)
  - Not destroyed by conversion; HAL setup preserves hardware coupling
  - Phase offset set at init (TIM8->CNT = TIM1->ARR/2)

⚠️ **ADC Trigger**: Must fire from TIM1 event (TIM1_CC4_IRQ)
  - Ensures synchronous sampling with PWM commutation
  - Configurable in ADC_EXTTRIG register

