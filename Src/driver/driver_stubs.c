/*
 * Driver Stubs for STM32F103 Hoverboard
 * Stubs for drivers not implemented on F103 hoverboard hardware
 */

#include "datatypes.h"

// IMU stubs
void imu_init(ATTITUDE_INFO *att) {
    (void)att;
}

void imu_stop(void) {
}

// Servo decoder stubs
void servodec_stop(void) {
}

// PWM servo stubs
void pwm_servo_init_servo(void) {
}

void pwm_servo_stop(void) {
}

// NRF SPI stubs
void spi_sw_init(void) {
}

void spi_sw_stop(void) {
}

void spi_sw_begin(void) {
}

void spi_sw_end(void) {
}

unsigned char spi_sw_transfer(unsigned char data) {
    return data;
}

// Terminal stubs
void terminal_register_command_callback(
    const char* command,
    const char *help,
    const char *arg_names,
    void(*cbf)(int argc, const char **argv)) {
    (void)command;
    (void)help;
    (void)arg_names;
    (void)cbf;
}

// Utils stubs
int utils_is_func_valid(void *addr) {
    (void)addr;
    return 1;
}
