// Minimal stubs for modules not available on F103 hoverboard hardware
// Only encoder, IMU, NRF, shutdown, virtual_motor, mempools remain stubbed.
// All application modules (PPM, ADC, UART, nunchuk, PAS) use real code.

#include "datatypes.h"
#include "conf_general.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// ===== Encoder stubs (hoverboard uses hall sensors, not encoders) =====
void encoder_init(mc_configuration *conf) { (void)conf; }
void encoder_deinit(void) {}
void encoder_update_config(mc_configuration *conf) { (void)conf; }
bool encoder_check_faults(mc_configuration *conf, bool is_second_motor) {
    (void)conf; (void)is_second_motor; return false;
}
bool encoder_is_configured(void) { return false; }
bool encoder_index_found(void) { return false; }
float encoder_read_deg(void) { return 0.0f; }

// ===== NRF driver stubs (no NRF radio on hoverboard) =====
void nrf_driver_stop(void) {}
void nrf_driver_start(void) {}
void nrf_driver_init(void) {}
void nrf_driver_pause(int ms) { (void)ms; }
bool nrf_driver_ext_nrf_running(void) { return false; }
void rfhelp_restart(void) {}
void rfhelp_update_conf(nrf_config *conf) { (void)conf; }
void nrf_driver_start_pairing(int ms) { (void)ms; }
void nrf_driver_init_ext_nrf(void) {}
bool nrf_driver_is_pairing(void) { return false; }
void nrf_driver_process_packet(unsigned char *data, unsigned int len) { (void)data; (void)len; }

// ===== IMU stubs (no IMU chip on hoverboard) =====
void imu_init(imu_config *conf) { (void)conf; }
void imu_get_calibration(float *accel, float *gyro, float *mag) {
    (void)accel; (void)gyro; (void)mag;
}
void imu_get_rpy(float *rpy) { (void)rpy; }
void imu_get_accel(float *accel) { (void)accel; }
void imu_get_gyro(float *gyro) { (void)gyro; }
void imu_get_mag(float *mag) { (void)mag; }
void imu_get_quaternions(float *q) { (void)q; }
void imu_reset_orientation(void) {}

// ===== Shutdown stubs (hoverboard uses OFF_PIN directly) =====
void shutdown_init(void) {}
void shutdown_reset_timer(void) {}
void do_shutdown(bool cut_power) { (void)cut_power; }

// ===== Virtual motor stub (not used on real hardware) =====
void virtual_motor_init(void) {}
void virtual_motor_set_configuration(void *conf) { (void)conf; }

// ===== HAL fault stubs =====
void mcpwm_hal_fault_init(void) {}

// ===== Mempools stubs (static allocation on F103, no dynamic pools) =====
static mc_configuration _mc_conf_buf;
static app_configuration _app_conf_buf;
static uint8_t _packet_buf[512];

void mempools_init(void) {}
void *mempools_alloc(unsigned int size) { (void)size; return 0; }
void mempools_free(void *ptr) { (void)ptr; }
void *mempools_alloc_appconf(void) { return &_app_conf_buf; }
void mempools_free_appconf(void *ptr) { (void)ptr; }
void *mempools_alloc_mcconf(void) { return &_mc_conf_buf; }
void mempools_free_mcconf(void *ptr) { (void)ptr; }
void *mempools_get_packet_buffer(void) { return _packet_buf; }
void mempools_free_packet_buffer(void *ptr) { (void)ptr; }

// ===== Filter stubs (digital_filter module) =====
void *filter_create_fir_lowpass(float f_s, float f_c, int order) {
    (void)f_s; (void)f_c; (void)order; return 0;
}
void filter_run_fir_iteration(void *filter, float *input, float *output, int length) {
    (void)filter; (void)input; (void)output; (void)length;
}
void filter_add_sample(void *filter, float sample) {
    (void)filter; (void)sample;
}

// ===== ADC channel setup (dual motor hoverboard specific) =====
void hw_setup_adc_channels(void) {
    // ADC1 regular sequence: CH11(PC1 R_DC), CH0(PA0 L_U), CH14(PC4 R_U), CH12(PC2 VBUS), CH16(TempSensor)
    ADC1->SQR3 = (11 << 0) | (0 << 5) | (14 << 10) | (12 << 15) | (16 << 20);
    // ADC2 regular sequence: CH10(PC0 L_DC), CH13(PC3 L_V), CH15(PC5 R_V), CH2(PA2 throttle), CH3(PA3 brake)
    ADC2->SQR3 = (10 << 0) | (13 << 5) | (15 << 10) | (2 << 15) | (3 << 20);
}

// ===== HAL GPIO stubs =====
void hal_gpio_init_af(void *port, uint32_t pin, uint32_t af) {
    (void)port; (void)pin; (void)af;
}

// ===== comm_usb stubs (F103 hoverboard uses UART, not USB) =====
void comm_usb_init(void) {}
void comm_usb_send_packet(unsigned char *data, unsigned int len) { (void)data; (void)len; }
unsigned int comm_usb_serial_configured_cnt(void) { return 0; }
unsigned int comm_usb_get_write_timeout_cnt(void) { return 0; }

// ===== mempools diagnostics (not using dynamic pool allocator) =====
int mempools_mcconf_allocated_num(void) { return 0; }
int mempools_mcconf_highest(void) { return 0; }
int mempools_appconf_allocated_num(void) { return 0; }
int mempools_appconf_highest(void) { return 0; }
