/*
 * Motor Control Interface Stubs for STM32F103 Hoverboard
 * These are minimal stubs to allow compilation without full motor control implementation.
 * Replace with actual motor control code when porting is complete.
 */

#include "mc_interface.h"
#include "conf_general.h"

// Stub implementations for motor control interface functions

mc_fault_code mc_interface_get_fault(void) {
    return FAULT_CODE_NONE;
}

const volatile mc_configuration* mc_interface_get_configuration(void) {
    static mc_configuration conf = {0};
    return &conf;
}

void mc_interface_set_configuration(mc_configuration *configuration) {
    (void)configuration;
}

float mc_interface_get_rpm(void) {
    return 0.0f;
}

void mc_interface_set_duty(float dutyCycle) {
    (void)dutyCycle;
}

void mc_interface_set_duty_noramp(float dutyCycle) {
    (void)dutyCycle;
}

void mc_interface_set_pid_speed(float rpm) {
    (void)rpm;
}

void mc_interface_set_pid_pos(float pos) {
    (void)pos;
}

void mc_interface_set_current(float current) {
    (void)current;
}

void mc_interface_set_brake_current(float current) {
    (void)current;
}

void mc_interface_set_current_rel(float val) {
    (void)val;
}

void mc_interface_set_brake_current_rel(float val) {
    (void)val;
}

void mc_interface_set_handbrake(float current) {
    (void)current;
}

void mc_interface_set_handbrake_rel(float val) {
    (void)val;
}

void mc_interface_brake_now(void) {
}

void mc_interface_release_motor(void) {
}

float mc_interface_get_duty_cycle_now(void) {
    return 0.0f;
}

float mc_interface_get_tot_current_directional_filtered(void) {
    return 0.0f;
}

float mc_interface_get_tot_current_filtered(void) {
    return 0.0f;
}

float mc_interface_get_tot_current_in_filtered(void) {
    return 0.0f;
}

int mc_interface_get_tachometer_value(bool reset) {
    (void)reset;
    return 0;
}

int mc_interface_get_tachometer_abs_value(bool reset) {
    (void)reset;
    return 0;
}

void mc_interface_lock(void) {
}

void mc_interface_unlock(void) {
}

mc_state mc_interface_get_state(void) {
    return MC_STATE_OFF;
}

void mc_interface_ignore_tacho_for_time(int time_ms) {
    (void)time_ms;
}

float mc_interface_read_reset_avg_motor_current(void) {
    return 0.0f;
}

float mc_interface_read_reset_avg_input_current(void) {
    return 0.0f;
}

float mc_interface_read_reset_avg_id(void) {
    return 0.0f;
}

float mc_interface_read_reset_avg_iq(void) {
    return 0.0f;
}

float mc_interface_read_reset_avg_vd(void) {
    return 0.0f;
}

float mc_interface_read_reset_avg_vq(void) {
    return 0.0f;
}

void mc_interface_init(void) {
}

void mc_interface_select_motor_thread(int motor) {
    (void)motor;
}

int mc_interface_motor_now(void) {
    return 0;
}

void mc_interface_set_pwm_callback(void (*p_func)(void)) {
    (void)p_func;
}

float mc_interface_temp_fet_filtered(void) {
    return 25.0f;
}

float mc_interface_temp_motor_filtered(void) {
    return 25.0f;
}

float mc_interface_get_battery_level(float *wh_left) {
    if (wh_left) *wh_left = 0.0f;
    return 0.0f;
}

float mc_interface_get_speed(void) {
    return 0.0f;
}

float mc_interface_get_distance(void) {
    return 0.0f;
}

float mc_interface_get_distance_abs(void) {
    return 0.0f;
}

float mc_interface_get_pid_pos_now(void) {
    return 0.0f;
}

float mc_interface_get_pid_pos_set(void) {
    return 0.0f;
}

void mc_interface_release_motor_override(void) {
}

void mc_interface_ignore_input_both(int time_ms) {
    (void)time_ms;
}

// mcpwm stubs
void mcpwm_init_hardware(void) {
}

float mcpwm_get_detect_pos(void) {
    return 0.0f;
}

// mcpwm_foc stubs
float mcpwm_foc_get_phase_observer(void) {
    return 0.0f;
}

float mcpwm_foc_get_phase_encoder(void) {
    return 0.0f;
}

float mcpwm_foc_get_phase_hall(void) {
    return 0.0f;
}

// mcpwm_hal_fault stubs
void mcpwm_hal_fault_init(void) {
}

// encoder stubs
float encoder_read_deg(void) {
    return 0.0f;
}

// shutdown stubs
void shutdown_init(void) {
}

// imu stubs
void imu_reset_orientation(void) {
}

void mc_interface_release_motor_override_both(void) {
}

int mc_interface_get_motor_thread(void) {
    return 0;
}

setup_values mc_interface_get_setup_values(void) {
    setup_values values = {0};
    return values;
}
