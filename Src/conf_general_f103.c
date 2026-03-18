/*
 * STM32F103 port of conf_general.
 *
 * The upstream VESC conf_general.c contains STM32F4 StdPeriph + legacy EEPROM
 * emulation logic and depends on many ADC index/macros that are not applicable
 * to this hoverboard STM32F103 HAL port.
 *
 * This file provides a minimal, deterministic configuration implementation:
 * - Defaults come from mcconf_default.h / appconf_default.h
 * - Store/read hooks are stubbed for now (TODO: implement flash-page EEPROM emu)
 */

#include "conf_general.h"

#include <string.h>

bool conf_general_permanent_nrf_found = false;
volatile backup_data g_backup;

static void appconf_set_defaults(app_configuration *conf) {
	memset(conf, 0, sizeof(*conf));
	conf->controller_id = APPCONF_CONTROLLER_ID;
	conf->timeout_msec = APPCONF_TIMEOUT_MSEC;
	conf->timeout_brake_current = APPCONF_TIMEOUT_BRAKE_CURRENT;
	conf->kill_sw_mode = APPCONF_KILL_SW_MODE;
	conf->can_mode = APPCONF_CAN_MODE;
	conf->can_baud_rate = APPCONF_CAN_BAUD_RATE;
	conf->permanent_uart_enabled = APPCONF_PERMANENT_UART_ENABLED;
	conf->app_uart_baudrate = APPCONF_UART_BAUDRATE;
	conf->shutdown_mode = APPCONF_SHUTDOWN_MODE;
}

static void mcconf_set_defaults(mc_configuration *conf) {
	memset(conf, 0, sizeof(*conf));
	conf->motor_type = MCCONF_DEFAULT_MOTOR_TYPE;
	conf->pwm_mode = MCCONF_PWM_MODE;
	conf->sensor_mode = MCCONF_SENSOR_MODE;
	conf->comm_mode = MCCONF_COMM_MODE;
	conf->l_current_max = MCCONF_L_CURRENT_MAX;
	conf->l_current_min = MCCONF_L_CURRENT_MIN;
	conf->l_in_current_max = MCCONF_L_IN_CURRENT_MAX;
	conf->l_in_current_min = MCCONF_L_IN_CURRENT_MIN;
	conf->l_min_vin = MCCONF_L_MIN_VOLTAGE;
	conf->l_max_vin = MCCONF_L_MAX_VOLTAGE;
}

void conf_general_init(void) {
	// TODO(F103): load from flash pages (EEPROM emulation) if present.
	(void)memset((void *)&g_backup, 0, sizeof(g_backup));
}

bool conf_general_store_backup_data(void) {
	// TODO(F103): store to flash.
	return false;
}

bool conf_general_read_eeprom_var_hw(eeprom_var *v, int address) {
	(void)v;
	(void)address;
	return false;
}

bool conf_general_read_eeprom_var_custom(eeprom_var *v, int address) {
	(void)v;
	(void)address;
	return false;
}

bool conf_general_store_eeprom_var_hw(eeprom_var *v, int address) {
	(void)v;
	(void)address;
	return false;
}

bool conf_general_store_eeprom_var_custom(eeprom_var *v, int address) {
	(void)v;
	(void)address;
	return false;
}

void conf_general_read_app_configuration(app_configuration *conf) {
	if (!conf) {
		return;
	}
	appconf_set_defaults(conf);
}

bool conf_general_store_app_configuration(app_configuration *conf) {
	(void)conf;
	return false;
}

void conf_general_read_mc_configuration(mc_configuration *conf, bool is_motor_2) {
	(void)is_motor_2;
	if (!conf) {
		return;
	}
	mcconf_set_defaults(conf);
}

bool conf_general_store_mc_configuration(mc_configuration *conf, bool is_motor_2) {
	(void)conf;
	(void)is_motor_2;
	return false;
}

// The following are not implemented yet for this port.
bool conf_general_detect_motor_param(float current, float min_rpm, float low_duty,
									 float *int_limit, float *bemf_coupling_k, int8_t *hall_table, int *hall_res) {
	(void)current; (void)min_rpm; (void)low_duty; (void)int_limit; (void)bemf_coupling_k; (void)hall_table; (void)hall_res;
	return false;
}

bool conf_general_measure_flux_linkage(float current, float duty,
									   float min_erpm, float res, float *linkage) {
	(void)current; (void)duty; (void)min_erpm; (void)res; (void)linkage;
	return false;
}

uint8_t conf_general_calculate_deadtime(float deadtime_ns, float core_clock_freq) {
	(void)deadtime_ns;
	(void)core_clock_freq;
	return 0;
}

int conf_general_measure_flux_linkage_openloop(float current, float duty,
											   float erpm_per_sec, float res, float ind, float *linkage,
											   float *linkage_undriven, float *undriven_samples, bool *result,
											   float *enc_offset, float *enc_ratio, bool *enc_inverted) {
	(void)current; (void)duty; (void)erpm_per_sec; (void)res; (void)ind; (void)linkage;
	(void)linkage_undriven; (void)undriven_samples; (void)result; (void)enc_offset; (void)enc_ratio; (void)enc_inverted;
	return -1;
}

int conf_general_autodetect_apply_sensors_foc(float current,
											  bool store_mcconf_on_success, bool send_mcconf_on_success, int *result) {
	(void)current; (void)store_mcconf_on_success; (void)send_mcconf_on_success; (void)result;
	return -1;
}

void conf_general_calc_apply_foc_cc_kp_ki_gain(mc_configuration *mcconf, float tc) {
	(void)mcconf;
	(void)tc;
}

int conf_general_detect_apply_all_foc(float max_power_loss,
									  bool store_mcconf_on_success, bool send_mcconf_on_success) {
	(void)max_power_loss; (void)store_mcconf_on_success; (void)send_mcconf_on_success;
	return -1;
}

int conf_general_detect_apply_all_foc_can(bool detect_can, float max_power_loss,
										  float min_current_in, float max_current_in,
										  float openloop_rpm, float sl_erpm,
										  void(*reply_func)(unsigned char* data, unsigned int len)) {
	(void)detect_can; (void)max_power_loss; (void)min_current_in; (void)max_current_in;
	(void)openloop_rpm; (void)sl_erpm; (void)reply_func;
	return -1;
}

