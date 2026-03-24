// Stub implementations for disabled application modules
// These are needed to satisfy linker when modules are not built

#include "datatypes.h"
#include "conf_general.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Application stubs - Commands, MCPWM, ADC app, UART app are in the build.
// PPM, Nunchuk, PAS are excluded to save RAM on F103.

// Encoder stubs
void encoder_init(mc_configuration *conf) { (void)conf; }
void encoder_deinit(void) {}
void encoder_update_config(mc_configuration *conf) { (void)conf; }
bool encoder_check_faults(mc_configuration *conf, bool is_second_motor) { 
    (void)conf; (void)is_second_motor; return false; 
}

// Utils_sys stubs removed - utils_sys.c is now in the build

// PPM application stubs (disabled to save RAM)
void app_ppm_stop(void) {}
void app_ppm_start(bool is_primary_output) { (void)is_primary_output; }
void app_ppm_configure(app_configuration *conf) { (void)conf; }
float app_ppm_get_decoded_level(void) { return 0.0f; }

// Nunchuk application stubs (disabled to save RAM)
void app_nunchuk_stop(void) {}
void app_nunchuk_start(void) {}
void app_nunchuk_configure(chuk_config *conf) { (void)conf; }
float app_nunchuk_get_decoded_chuk(void) { return 0.0f; }
float app_nunchuk_get_decoded_y(void) { return 0.0f; }
void app_nunchuk_update_output(chuk_config *config) { (void)config; }

// PAS application stubs (PAS not enabled, but needed by other apps)
void app_pas_stop(void) {}
void app_pas_start(bool is_primary_output) { (void)is_primary_output; }
void app_pas_configure(pas_config *conf) { (void)conf; }
bool app_pas_is_running(void) { return false; }
float app_pas_get_current_target_rel(void) { return 0.0f; }

// NRF driver stubs
void nrf_driver_stop(void) {}
void nrf_driver_start(void) {}
void nrf_driver_init(void) {}
void nrf_driver_pause(int ms) { (void)ms; }
bool nrf_driver_ext_nrf_running(void) { return false; }
void rfhelp_restart(void) {}
void rfhelp_update_conf(nrf_config *conf) { (void)conf; }

// CAN stubs removed - all provided by comm_can.h inline functions when CAN_ENABLE=0

// IMU stubs
void imu_init(imu_config *conf) { (void)conf; }
void imu_get_calibration(float *accel, float *gyro, float *mag) {
    (void)accel; (void)gyro; (void)mag;
}

// Servo decoder stubs
void servodec_stop(void) {}
void servodec_init(void (*d_func)(void)) { (void)d_func; }
void servodec_set_pulse_options(float min, float max, bool median_filter) {
    (void)min; (void)max; (void)median_filter;
}
float servodec_get_servo(int servo_num) { (void)servo_num; return 0.0f; }
uint32_t servodec_get_time_since_update(void) { return 0; }
float servodec_get_last_pulse_len(uint32_t channel) { (void)channel; return 0.0f; }

// PWM servo stubs
void pwm_servo_init_servo(void) {}
void pwm_servo_stop(void) {}
void pwm_servo_set_servo_out(float value) { (void)value; }

// Encoder stubs
bool encoder_is_configured(void) { return false; }

// Terminal stubs (disabled to save RAM for commands.c)
void terminal_process_string(char *str) { (void)str; }
void terminal_register_command_callback(
    const char* command,
    const char *help,
    const char *arg_names,
    void(*cbf)(int argc, const char **argv)) {
    (void)command; (void)help; (void)arg_names; (void)cbf;
}

// confgenerator stubs removed - confgenerator.c is now in the build

// Utility stubs
void shutdown_reset_timer(void) {}

// HAL GPIO stubs
void hal_gpio_init_af(void *port, uint32_t pin, uint32_t af) {
    (void)port; (void)pin; (void)af;
}

// confgenerator_set_defaults stubs removed - confgenerator.c is now in the build

// Additional NRF stubs
void nrf_driver_start_pairing(int ms) { (void)ms; }
void nrf_driver_init_ext_nrf(void) {}
bool nrf_driver_is_pairing(void) { return false; }
void nrf_driver_process_packet(unsigned char *data, unsigned int len) { (void)data; (void)len; }

// CAN stubs removed - all provided by comm_can.h inline functions when CAN_ENABLE=0

// Additional IMU stubs
void imu_get_rpy(float *rpy) { (void)rpy; }
void imu_get_accel(float *accel) { (void)accel; }
void imu_get_gyro(float *gyro) { (void)gyro; }
void imu_get_mag(float *mag) { (void)mag; }
void imu_get_quaternions(float *q) { (void)q; }

// BMS stubs
void bms_process_cmd(unsigned char *data, unsigned int len) { (void)data; (void)len; }

// Shutdown stub
void do_shutdown(bool cut_power) { (void)cut_power; }

// utils_is_func_valid now in utils_sys.c

// LED PWM stubs
void ledpwm_set_intensity(unsigned int led, float intensity) { (void)led; (void)intensity; }

// BMS additional stubs
void bms_init(void) {}
void bms_update_limits(float *min_voltage, float *max_voltage, float *max_charge_current, float *max_discharge_current) {
    (void)min_voltage; (void)max_voltage; (void)max_charge_current; (void)max_discharge_current;
}

// UTILS_AGE_S is now a macro in utils_sys.h

// LED PWM init stub
void ledpwm_init(void) {}

// CAN init provided by comm_can.h inline when CAN_ENABLE=0

// Terminal fault data stub
void terminal_add_fault_data(void *data) { (void)data; }

// ADC setup - configure ADC channel sequence registers for dual motor
void hw_setup_adc_channels(void) {
    // ADC1 regular sequence: CH11(PC1), CH0(PA0), CH14(PC4), CH12(PC2), CH16(TempSensor)
    ADC1->SQR3 = (11 << 0) | (0 << 5) | (14 << 10) | (12 << 15) | (16 << 20);
    // ADC2 regular sequence: CH10(PC0), CH13(PC3), CH15(PC5), CH2(PA2), CH3(PA3)
    ADC2->SQR3 = (10 << 0) | (13 << 5) | (15 << 10) | (2 << 15) | (3 << 20);
}

// Encoder index detection stub
bool encoder_index_found(void) { return false; }

// utils_read_hall now in utils_sys.c

// Encoder stubs (encoder module excluded)
float encoder_read_deg(void) { return 0.0f; }

// Shutdown init stub (shutdown module excluded)
void shutdown_init(void) {}

// IMU stubs (IMU module excluded)
void imu_reset_orientation(void) {}

// HAL fault stubs (mcpwm_hal_fault excluded)
void mcpwm_hal_fault_init(void) {}

// Virtual motor stub (virtual_motor.c excluded)
void virtual_motor_init(void) {}
void virtual_motor_set_configuration(void *conf) { (void)conf; }

// Mempools stubs (excluded to save RAM on RCT6)
void mempools_init(void) {}
void *mempools_alloc(unsigned int size) { (void)size; return 0; }
void mempools_free(void *ptr) { (void)ptr; }
void *mempools_alloc_appconf(void) { return 0; }
void mempools_free_appconf(void *ptr) { (void)ptr; }
void *mempools_alloc_mcconf(void) { return 0; }
void mempools_free_mcconf(void *ptr) { (void)ptr; }
void *mempools_get_packet_buffer(void) { return 0; }
void mempools_free_packet_buffer(void *ptr) { (void)ptr; }

// Filter stubs (digital_filter module)
void *filter_create_fir_lowpass(float f_s, float f_c, int order) {
    (void)f_s; (void)f_c; (void)order;
    return 0;
}
void filter_run_fir_iteration(void *filter, float *input, float *output, int length) {
    (void)filter; (void)input; (void)output; (void)length;
}
void filter_add_sample(void *filter, float sample) {
    (void)filter; (void)sample;
}

// CAN status stubs removed - provided by comm_can.h inline functions when CAN_ENABLE=0
