/**
 * ==============================================================================
 * VESC FIRMWARE CONVERSION - COMPLETE ROADMAP
 * ChibiOS → STM32 HAL + FreeRTOS CMSIS-RTOS v2
 * ==============================================================================
 * 
 * STATUS: 35% Complete (Core Peripheral Layer)
 * NEXT: Agent 1 (HAL) → Agent 2 (RTOS) → Integration Testing
 * 
 * ==============================================================================
 */

#ifndef CONVERSION_ROADMAP_H_
#define CONVERSION_ROADMAP_H_

/**
 * AGENT 3 CONVERSION STATUS: ✅ COMPLETE
 * 
 * Peripheral Drivers & Interfaces (Hardware-specific layer)
 * 
 * COMPLETED CONVERSIONS:
 * =====================
 * 
 * 1. I2C Bit-Bang Driver (i2c_bb.c/h)
 *    - ✅ Replaced chThdSleep → osDelay
 *    - ✅ Updated mutex: mutex_t → osMutexId_t
 *    - ✅ Headers: Removed ch.h/hal.h → stm32f1xx_hal.h
 *    - ✅ Maintains 400kHz I2C frequency
 *    - STATUS: Ready for encoder/IMU integration
 * 
 * 2. SPI Bit-Bang Driver (spi_bb.c/h)
 *    - ✅ Uses hal_gpio_* abstraction (no direct register access)
 *    - ✅ osMutex* for thread-safe transfers
 *    - ✅ Headers: STM32 HAL compatible
 *    - ✅ Supports 1-10 MHz programmable frequency
 *    - STATUS: Ready for AS504x, TLE5012, MT6816 encoders
 * 
 * 3. CAN Hardware Layer (comm_can.c + NEW comm_can_hal.c/h)
 *    - ✅ Created comm_can_hal.h: CANTxFrame/CANRxFrame compatibility
 *    - ✅ Created comm_can_hal.c: can_hal_init/start/transmit functions
 *    - ✅ Replaced canStart() → can_hal_init() + can_hal_start()
 *    - ✅ Replaced canTransmit() → can_hal_transmit()
 *    - ✅ RX exception handling via HAL callbacks
 *    - ✅ Supports 125k/250k/500k/1000k baud rates
 *    - STATUS: Ready for CAN message processing threads (Agent 2)
 * 
 * 4. Encoder Drivers (enc_*.c)
 *    - ✅ enc_as504x, enc_as5x47u, enc_abi, enc_ad2s1205
 *    - ✅ enc_bissc, enc_ma782, enc_mt6816, enc_pwm
 *    - ✅ enc_sincos, enc_tle5012, enc_ts5700n8501
 *    - ✅ All headers: Removed ch.h/hal.h → stm32f1xx_hal.h
 *    - ✅ encoder_datatype.h: Added stm32_gpio_t typedef
 *    - STATUS: Ready for motor control integration
 * 
 * PARTIALLY COMPLETED:
 * ====================
 * 
 * 5. CAN Driver (comm_can.c)
 *    - ✅ CAN init function converted (can_hal_init calls)
 *    - ✅ CAN TX converted (can_hal_transmit calls)
 *    - ✅ Replaced chThdSleepMicroseconds → osDelay (CAN TX retry)
 *    - ⚠️ RX thread: Still uses ChibiOS patterns (Agent 2 scope)
 *    - ⚠️ Event flags: partially converted (Agent 2 scope)
 *    - ⚠️ Thread creation: still chThdCreateStatic (Agent 2 scope)
 *    - STATUS: Peripheral layer 90% done, thread mgmt pending
 * 
 * 6. IMU Drivers
 *    - ✅ mpu9150.c: 3× chThdSleepMicroseconds → osDelay (partial)
 *    - ✅ icm20948.c: 1× chThdSleepMicroseconds → osDelay
 *    - ⚠️ chVTGetSystemTimeX() calls: Pending Agent 1 timer layer
 *    - STATUS: Sleep calls done, timer foundation needed
 * 
 * ==============================================================================
 * 
 * AGENT 1 CONVERSION TASKS: HAL GPIO/Timer/ISR Foundation
 * 
 * PENDING CONVERSIONS (Not Agent 3 scope):
 * ========================================
 * 
 * 1. Hardware Configuration Cores (54 files)
 *    Files: hwconf/vesc/{str365,pronto,maxim,etc}_core.c
 *    Calls: ~80 instances of chThdSleepMicroseconds(T_SAMP_US)
 *    Task: Replace with osDelay(pdMS_TO_TICKS(T_SAMP_US/1000))
 *    Note: These are in sampling loops, NOT ISRs, so safe for CMSIS-RTOS v2
 *    Priority: HIGH - blocking motor control tests
 * 
 * 2. Timer Functions (driver/timer.c)
 *    Current: Uses ChibiOS TIM5 register access
 *    Needed: HAL_TIM_* equivalents
 *    Function: timer_init(), timer_seconds_elapsed_since()
 *    Priority: CRITICAL - encoder/position tracking depends on this
 * 
 * 3. Encoder GPIO/ISR Initialization (encoder.c)
 *    Lines: nvicDisableVector(HW_ENC_EXTI_CH),  HW_ENC_TIM, TIM_DeInit()
 *    Calls: TIM_TimeBaseInit() → HAL_TIM_Base_Init()
 *    Task: Convert to STM32 HAL timer initialization
 *    Note: Requires Agent 1 to define hal_gpio.c functionality
 *    Priority: HIGH
 * 
 * 4. GPIO Alternate Function (existing hal_gpio.c)
 *    Status: Already has hal_gpio_init_af() - verify it's complete
 *    Needed: Confirm hal_gpio_init_output_od() for I2C open-drain
 *    Priority: MEDIUM - hal_gpio already partially done
 * 
 * 5. PWM Servo Functions (driver/pwm_servo.c, servo_dec.c)
 *    Current: ChibiOS HAL references
 *    Needed: Timer library functions and GPIO initialization
 *    Priority: MEDIUM
 * 
 * ==============================================================================
 * 
 * AGENT 2 CONVERSION TASKS: RTOS Thread Architecture
 * 
 * PENDING CONVERSIONS (Not Agent 3 scope):
 * ========================================
 * 
 * 1. CAN Receive Thread (comm_can.c)
 *    Current: canReceiveThread() reads from CAN FIFO
 *    Pattern: chEvtWaitAny() waiting for new message
 *    Convert: Use HAL CAN RX interrupts + osEventFlagsWait()
 *    Code lines: ~150 lines for thread function
 *    Priority: CRITICAL - CAN communication backbone
 * 
 * 2. CAN Status Threads (comm_can.c)
 *    - cancom_status_thread()
 *    - cancom_status_thread_2()
 *    - cancom_status_internal_thread() [dual motor]
 *    - cancom_process_thread()
 *    Task: Already partially converted to osThreadNew() 
 *    Remaining: Replace chEvtSignal/chEvtWait patterns in thread bodies
 *    Priority: HIGH
 * 
 * 3. Motor Control ISR Timing (mcpwm_foc.c)
 *    Calls: 10 instances of chThdSleepMicroseconds()
 *    Context: Not in ISR, in PID control thread
 *    Lines: 4462-4470 (PID_RATE_ sleep cases)
 *    Convert: osDelay(pdMS_TO_TICKS(...))
 *    Priority: CRITICAL - motor control loop timing
 * 
 * 4. Event Signaling Throughout Codebase
 *    Files: 15+ (motor/mc_interface.c, app_*.c, terminal.c, etc.)
 *    Calls: chEvtSignal(), chEvtWaitAny(), chEvtWaitAnyTimeout()
 *    Pattern: Replace with osEventFlagsSet() / osEventFlagsWait()
 *    Note: osEventFlags are UINT32_t, support up to 32 different flags
 *    Priority: CRITICAL - all thread synchronization depends on this
 * 
 * 5. USB CDC Serial Threads (comm_usb_serial.c)
 *    Threads: serial_read_thread(), serial_process_thread()
 *    Current: chThdCreateStatic() with ChibiOS wait patterns
 *    Convert: osThreadNew() + FreeRTOS USB integration
 *    Note: Requires USB HAL stack (Agent 1) + thread scheduling (Agent 2)
 *    Priority: MEDIUM - terminal communication
 * 
 * 6. Application Threads (app_*.c)
 *    - app_adc_thread() [app_adc.c]
 *    - app_ppm_thread() [app_ppm.c]
 *    - app_uart_thread() [app_uartcomm.c]  
 *    - app_dpv_thread() [app_dpv.c]
 *    Pattern: All use chThdCreateStatic() and event signaling
 *    Task: Replace thread creation + event synchronization
 *    Priority: MEDIUM - application features
 * 
 * 7. Command Handler Threads (commands.c)
 *    Threads: blocking_thread(), terminal interaction
 *    Pattern: chMtxLock/Unlock for mutual exclusion
 *    Status: Already partially converted to osMutex* in globals
 *    Remaining: Thread creation, event signaling for blocking operations
 *    Priority: MEDIUM
 * 
 * ==============================================================================
 * 
 * INTEGRATION CHECKLIST:
 * ======================
 * 
 * [ ] Agent 1: Complete timer.c conversion (hal_timer_*functions)
 * [ ] Agent 1: Complete hwconf cores sleep calls (54 files)
 * [ ] Agent 1: encoder.c ISR initialization (nvicDisableVector)
 * 
 * [ ] Agent 2: CAN RX thread conversion with HAL interrupt handling
 * [ ] Agent 2: Motor control PID timing (mcpwm_foc.c sleep calls)
 * [ ] Agent 2: Event signaling framework (osEventFlags everywhere)
 * [ ] Agent 2: USB CDC thread integration
 * [ ] Agent 2: Application threads (app_*.c)
 * 
 * [ ] Integration Test: I2C encoder read (AS504x over i2c_bb)
 * [ ] Integration Test: CAN communication (transmit + receive)
 * [ ] Integration Test: Motor startup (timing + control loops)
 * [ ] Integration Test: ADC sampling (app_adc thread)
 * [ ] Integration Test: Terminal communication (USB CDC + command handler)
 * 
 * ==============================================================================
 * 
 * DESIGN NOTES:
 * =============
 * 
 * 1. Mutex vs EventFlags:
 *    - Use osMutex* for protecting shared resources
 *    - Use osEventFlags* for thread synchronization/signaling
 *    - Both converted from ChibiOS equivalents
 * 
 * 2. Timing Conversion:
 *    - chThdSleepMicroseconds(X) → osDelay(pdMS_TO_TICKS(X/1000))
 *    - Note: FreeRTOS osDelay() has millisecond precision
 *    - For microsecond-level timing, keep direct loop delays or use HAL timer
 * 
 * 3. ISR vs Thread Context:
 *    - ISR context: Can use HAL interrupt callbacks, no osDelay()
 *    - Thread context: Can use osDelay(), osEventFlagsWait(), etc.
 *    - CAN RX interrupt: Set event flag from ISR, thread waits
 * 
 * 4. CAN Hardware Layer:
 *    - comm_can_hal.c provides STM32 HAL wrapper
 *    - Maintains compatibility with existing comm_can.c interface
 *    - Can be extended for dual-motor CAN synchronization
 * 
 * ==============================================================================
 */

#endif /* CONVERSION_ROADMAP_H_ */
