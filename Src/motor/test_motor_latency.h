/*
	Motor ISR Latency & Synchronization Testing
	
	Header file with test function declarations for motor control ISR validation
*/

#ifndef TEST_MOTOR_LATENCY_H_
#define TEST_MOTOR_LATENCY_H_

// Test function declarations
void motor_isr_latency_start(void);
void motor_isr_latency_update(uint32_t start_cycles);
void motor_isr_latency_report(void);

void test_motor_isr_latency(void);
void test_dual_motor_phase_sync(void);
void test_pwm_frequency(void);
void test_motor_isr_validation_suite(void);

// Hardware synchronization functions
void hw_verify_dual_motor_sync(void);
void mcpwm_hal_adc_dual_sync_verify(void);

#endif  // TEST_MOTOR_LATENCY_H_
