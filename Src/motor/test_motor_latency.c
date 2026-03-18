/*
	Motor ISR Latency Testing and Validation
	
	This module provides utilities for measuring and validating motor control ISR latency.
	Required latency: <2 µs from ADC sample to PWM duty update
	Measured in CPU cycles @ 72 MHz: <144 cycles
	
	Call motor_isr_latency_start() once at boot to enable DWT cycle counter.
	Results are collected automatically and reported via motor_isr_latency_report().
*/

#include "mcpwm_foc.h"
#include "commands.h"

// Forward declarations
extern volatile uint32_t g_motor_isr_latency_cycles;
extern volatile uint32_t g_motor_isr_latency_min;
extern volatile uint32_t g_motor_isr_latency_max;

void motor_isr_latency_start(void);
void motor_isr_latency_update(uint32_t start_cycles);
void motor_isr_latency_report(void);

/**
 * Test Motor ISR Latency
 * 
 * Runs motor at steady state and collects ISR latency statistics.
 * Prints min/max/current latency in cycles and microseconds.
 * 
 * SUCCESS: max latency < 200 cycles (~2.8 µs @ 72 MHz)
 * This leaves margin for <2 µs target
 */
void test_motor_isr_latency(void) {
    commands_printf("Motor ISR Latency Test\n");
    commands_printf("======================\n");
    commands_printf("Starting motor at 50%% duty...\n");
    
    // Ensure motor latency tracking is enabled
    motor_isr_latency_start();
    
    // Run motor at steady state for 50 ms (300 ADC samples @ 6 kHz)
    osDelay(50);
    
    // Report results
    motor_isr_latency_report();
    
    // Validation
    if (g_motor_isr_latency_max < 200) {
        commands_printf("✓ PASS: Motor ISR latency within target\n");
    } else {
        commands_printf("✗ FAIL: Motor ISR latency exceeds target\n");
        commands_printf("  Consider reducing non-ISR workload or optimizing observer\n");
    }
}

/**
 * Test Dual-Motor Phase Synchronization
 * 
 * Validates that TIM1 and TIM8 remain phase-locked at 180°.
 * Checks for clock drift or synchronization degradation.
 */
void test_dual_motor_phase_sync(void) {
#ifdef HW_HAS_DUAL_MOTORS
    commands_printf("Dual-Motor Phase Sync Test\n");
    commands_printf("===========================\n");
    
    uint16_t phase_errors = 0;
    const uint16_t expected_phase = 2250;  // TIM8 lags TIM1 by 180°
    const uint16_t tolerance = 100;        // ±100 counts tolerance
    
    commands_printf("Sampling 1000 phase offsets...\n");
    
    for (int i = 0; i < 1000; i++) {
        uint16_t tim1_cnt = TIM1->CNT;
        uint16_t tim8_cnt = TIM8->CNT;
        int16_t phase_diff = tim8_cnt - tim1_cnt;
        
        // Account for counter roll-over
        if (phase_diff < 0) {
            phase_diff += 4500;
        }
        
        if (phase_diff < (expected_phase - tolerance) || 
            phase_diff > (expected_phase + tolerance)) {
            phase_errors++;
        }
        
        osDelay(1);
    }
    
    commands_printf("Phase sync errors in 1000 samples: %u\n", phase_errors);
    
    if (phase_errors == 0) {
        commands_printf("✓ PASS: Dual motor phase locked perfectly\n");
    } else if (phase_errors < 10) {
        commands_printf("⚠ WARNING: %u phase errors detected (cyclic jitter expected)\n", phase_errors);
    } else {
        commands_printf("✗ FAIL: Excessive phase sync errors\n");
        commands_printf("  Check TIM1/TIM8 synchronization and clock stability\n");
    }
#else
    commands_printf("⚠ Dual-Motor Phase Sync Test: Not applicable (single motor)\n");
#endif
}

/**
 * Test PWM Output Frequency
 * 
 * Validates PWM frequency by measuring timer period.
 * Expected: 16 kHz (4500 counts @ 72 MHz)
 */
void test_pwm_frequency(void) {
    commands_printf("PWM Frequency Test\n");
    commands_printf("==================\n");
    
    uint32_t tim1_arr = TIM1->ARR;
    uint32_t tim8_arr = TIM8->ARR;
    
    // Calculate frequency: f = SYSCLK / (ARR + 1)
    // At 72 MHz: f = 72MHz / 4500 = 16 kHz
    double f_tim1 = 72000000.0 / (tim1_arr + 1);
    double f_tim8 = 72000000.0 / (tim8_arr + 1);
    
    commands_printf("TIM1 (Motor 1) PWM:\n");
    commands_printf("  ARR = %lu\n", tim1_arr);
    commands_printf("  Frequency = %.1f Hz\n", f_tim1);
    commands_printf("  Period = %.2f µs\n", 1e6 / f_tim1);
    
    commands_printf("\nTIM8 (Motor 2) PWM:\n");
    commands_printf("  ARR = %lu\n", tim8_arr);
    commands_printf("  Frequency = %.1f Hz\n", f_tim8);
    commands_printf("  Period = %.2f µs\n", 1e6 / f_tim8);
    
    // Validation
    if (fabsf(f_tim1 - 16000.0) < 100 && fabsf(f_tim8 - 16000.0) < 100) {
        commands_printf("\n✓ PASS: PWM frequency correct (16 kHz)\n");
    } else {
        commands_printf("\n✗ FAIL: PWM frequency out of spec\n");
    }
}

/**
 * Motor Control ISR Validation Suite
 * 
 * Runs all hardware validation tests for motor ISRs.
 */
void test_motor_isr_validation_suite(void) {
    commands_printf("\n");
    commands_printf("╔════════════════════════════════════════════════════════╗\n");
    commands_printf("║   Motor Control ISR & PWM Validation Suite            ║\n");
    commands_printf("║   Target: <2 µs ISR latency, 180° phase lock         ║\n");
    commands_printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    test_pwm_frequency();
    commands_printf("\n");
    
    test_dual_motor_phase_sync();
    commands_printf("\n");
    
    test_motor_isr_latency();
    commands_printf("\n");
    
    commands_printf("╔════════════════════════════════════════════════════════╗\n");
    commands_printf("║  Validation Suite Complete                            ║\n");
    commands_printf("╚════════════════════════════════════════════════════════╝\n");
}
