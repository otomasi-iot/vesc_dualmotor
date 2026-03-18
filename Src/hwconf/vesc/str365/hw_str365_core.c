/*
	Copyright 2018 Benjamin Vedder	benjamin@vedder.se

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "hw.h"
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"
#include "utils_math.h"
#include <math.h>
#include "mc_interface.h"
#include "lispif.h"
#include "lispbm.h"

// Variables
static volatile bool i2c_running = false;
static mutex_t shutdown_mutex;
static float bt_diff = 0.0;
static THD_WORKING_AREA(mux_thread_wa, 256);
static THD_FUNCTION(mux_thread, arg);

// I2C configuration
static const I2CConfig i2cfg = {
		OPMODE_I2C,
		100000,
		STD_DUTY_CYCLE
};

static lbm_value ext_reg_adj(lbm_value *args, lbm_uint argn) {
	LBM_CHECK_ARGN_NUMBER(1);

	int val = lbm_dec_as_i32(args[0]);
	utils_truncate_number_int(&val, 0, 4095);

	DAC->DHR12R2 = val;

	return ENC_SYM_TRUE;
}

static lbm_value ext_reg_en(lbm_value *args, lbm_uint argn) {
	LBM_CHECK_ARGN_NUMBER(1);
	lbm_dec_as_i32(args[0]) ? REG_ON() : REG_OFF();
	return ENC_SYM_TRUE;
}

static lbm_value ext_reg_v(lbm_value *args, lbm_uint argn) {
	(void)args; (void)argn;
	float adc = (float)ADC_Value[ADC_IND_12V_SENSE_V];
	// V-div 22k - 2.2k
	return lbm_enc_float(adc * (V_REG / 4095.0) * ((22.0 + 2.2) / 2.2));
}

static lbm_value ext_reg_i(lbm_value *args, lbm_uint argn) {
	(void)args; (void)argn;
	float adc = (float)ADC_Value[ADC_IND_12V_SENSE_I];
	// 0.01 ohm, shunt amp same as rest of hw
	return lbm_enc_float((adc * (V_REG / 4095.0) - (V_REG / 2.0)) / (CURRENT_AMP_GAIN * 0.01));
}

static lbm_value ext_reg_t(lbm_value *args, lbm_uint argn) {
	(void)args; (void)argn;
	return lbm_enc_float(NTC_TEMP_DCDC());
}

static lbm_value ext_reg5_v(lbm_value *args, lbm_uint argn) {
	(void)args; (void)argn;
	float adc = (float)ADC_Value[ADC_IND_5V_SENSE_V];
	// V-div 10k - 10k
	return lbm_enc_float(adc * (V_REG / 4095.0) * ((10.0 + 10.0) / 10.0));
}

static lbm_value ext_sw_hv(lbm_value *args, lbm_uint argn) {
	LBM_CHECK_ARGN_NUMBER(1);
	lbm_dec_as_i32(args[0]) ? SWHV_ON() : SWHV_OFF();
	return ENC_SYM_TRUE;
}

static void load_extensions(bool main_found) {
	if (!main_found) {
		lbm_add_extension("hw-reg-adj", ext_reg_adj);
		lbm_add_extension("hw-reg-en", ext_reg_en);
		lbm_add_extension("hw-reg-v", ext_reg_v);
		lbm_add_extension("hw-reg-i", ext_reg_i);
		lbm_add_extension("hw-reg-t", ext_reg_t);
		lbm_add_extension("hw-reg5-v", ext_reg5_v);
		lbm_add_extension("hw-sw-hv", ext_sw_hv);
	}
}

void hw_init_gpio(void) {
	#include "hwconf/hal_gpio.h"
	
	chMtxObjectInit(&shutdown_mutex);

	// GPIO clock enable - handled by hal_gpio functions but enable for safety
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// LEDs
	hal_gpio_init_output(LED_GREEN_GPIO, GPIO_PIN_9);
	hal_gpio_init_output(LED_RED_GPIO, GPIO_PIN_12);

	// GPIOA Configuration: Channel 1 to 3 as alternate function push-pull for TIM1
	// Note: Timer pins are configured separately by the timer initialization code
	// GPIO Pins will be configured when timers are enabled

	// Hall sensors with pull-up
	hal_gpio_init_input_pullup(HW_HALL_ENC_GPIO1, GPIO_PIN_6);
	hal_gpio_init_input_pullup(HW_HALL_ENC_GPIO2, GPIO_PIN_7);
	hal_gpio_init_input_pullup(HW_HALL_ENC_GPIO3, GPIO_PIN_8);

	// Phase filters
	hal_gpio_init_output(PHASE_FILTER_GPIO, GPIO_PIN_12);
	PHASE_FILTER_OFF();

	// AUX pins
	AUX_OFF();
	hal_gpio_init_output(AUX_GPIO, GPIO_PIN_7);
	AUX2_OFF();
	hal_gpio_init_output(AUX2_GPIO, GPIO_PIN_13);

	// ADC Pins as analog input
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_0);
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_1);
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_2);
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_3);
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_6);
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_7);

	hal_gpio_init_input_analog(GPIOB, GPIO_PIN_0);
	hal_gpio_init_input_analog(GPIOB, GPIO_PIN_1);

	hal_gpio_init_input_analog(GPIOC, GPIO_PIN_0);
	hal_gpio_init_input_analog(GPIOC, GPIO_PIN_1);
	hal_gpio_init_input_analog(GPIOC, GPIO_PIN_2);
	hal_gpio_init_input_analog(GPIOC, GPIO_PIN_3);
	hal_gpio_init_input_analog(GPIOC, GPIO_PIN_4);

	// DAC as voltage reference for shunt amps
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_4);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	DAC->CR |= DAC_CR_EN1;
	DAC->DHR12R1 = 2047;

	// Regulator
	hal_gpio_init_input_analog(GPIOA, GPIO_PIN_5);
	DAC->CR |= DAC_CR_EN2;
	DAC->DHR12R2 = 4095;

	REG_OFF();
	hal_gpio_init_output(REG_GPIO, GPIO_PIN_2);

	SWHV_OFF();
	hal_gpio_init_output(SWHV_GPIO, GPIO_PIN_13);

	lispif_add_ext_load_callback(load_extensions);
}

void hw_setup_adc_channels(void) {
	// ADC1 regular channels
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);			// 0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 2, ADC_SampleTime_15Cycles);			// 3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 3, ADC_SampleTime_15Cycles);			// 6
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_15Cycles);			// 9
	ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 5, ADC_SampleTime_15Cycles);	// 12
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 6, ADC_SampleTime_15Cycles);			// 15

	// ADC2 regular channels
	ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_15Cycles);			// 1
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 2, ADC_SampleTime_15Cycles);			// 4
	ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 3, ADC_SampleTime_15Cycles);			// 7
	ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 4, ADC_SampleTime_15Cycles);			// 10
	ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 5, ADC_SampleTime_15Cycles);			// 13
	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 6, ADC_SampleTime_15Cycles);			// 16

	// ADC3 regular channels
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);			// 2
	ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 2, ADC_SampleTime_15Cycles);			// 5
	ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 3, ADC_SampleTime_15Cycles);			// 8
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 4, ADC_SampleTime_15Cycles);			// 11
	ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 5, ADC_SampleTime_15Cycles);			// 14
	ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 6, ADC_SampleTime_15Cycles);			// 17

	// Injected channels
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_11, 2, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC3, ADC_Channel_12, 2, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_11, 3, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC3, ADC_Channel_12, 3, ADC_SampleTime_15Cycles);

	chThdCreateStatic(mux_thread_wa, sizeof(mux_thread_wa), NORMALPRIO, mux_thread, NULL);
}

void hw_start_i2c(void) {
	i2cAcquireBus(&HW_I2C_DEV);

	if (!i2c_running) {
		// Configure I2C pins for alternate function (peripheral control)
		// For STM32F1xx, I2C pins need to be configured as alternate function open-drain with pull-up
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(HW_I2C_SCL_PORT, &GPIO_InitStruct);

		i2cStart(&HW_I2C_DEV, &i2cfg);
		i2c_running = true;
	}

	i2cReleaseBus(&HW_I2C_DEV);
}

void hw_stop_i2c(void) {
	i2cAcquireBus(&HW_I2C_DEV);

	if (i2c_running) {
		hal_gpio_init_input(HW_I2C_SCL_PORT, GPIO_PIN_10);
		hal_gpio_init_input(HW_I2C_SDA_PORT, GPIO_PIN_11);

		i2cStop(&HW_I2C_DEV);
		i2c_running = false;

	}

	i2cReleaseBus(&HW_I2C_DEV);
}

/**
 * Try to restore the i2c bus
 */
void hw_try_restore_i2c(void) {
	if (i2c_running) {
		i2cAcquireBus(&HW_I2C_DEV);

		hal_gpio_init_output_od(HW_I2C_SCL_PORT, GPIO_PIN_10);
		hal_gpio_init_output_od(HW_I2C_SDA_PORT, GPIO_PIN_11);

		hal_gpio_set(HW_I2C_SCL_PORT, GPIO_PIN_10);
		hal_gpio_set(HW_I2C_SDA_PORT, GPIO_PIN_11);

		chThdSleep(1);

		for(int i = 0;i < 16;i++) {
			hal_gpio_clear(HW_I2C_SCL_PORT, GPIO_PIN_10);
			chThdSleep(1);
			hal_gpio_set(HW_I2C_SCL_PORT, GPIO_PIN_10);
			chThdSleep(1);
		}

		// Generate start then stop condition
		hal_gpio_clear(HW_I2C_SDA_PORT, GPIO_PIN_11);
		chThdSleep(1);
		hal_gpio_clear(HW_I2C_SCL_PORT, GPIO_PIN_10);
		chThdSleep(1);
		hal_gpio_set(HW_I2C_SCL_PORT, GPIO_PIN_10);
		chThdSleep(1);
		hal_gpio_set(HW_I2C_SDA_PORT, GPIO_PIN_11);

		// Restore I2C alternate function configuration
		// For STM32F1xx, I2C pins need to be configured as alternate function open-drain
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(HW_I2C_SCL_PORT, &GPIO_InitStruct);

		HW_I2C_DEV.state = I2C_STOP;
		i2cStart(&HW_I2C_DEV, &i2cfg);

		i2cReleaseBus(&HW_I2C_DEV);
	}
}

static THD_FUNCTION(mux_thread, arg) {
	(void)arg;

	chRegSetThreadName("adc_mux");

	hal_gpio_init_output(ADC_SW_1_PORT, GPIO_PIN_14);
	hal_gpio_init_output(ADC_SW_2_PORT, GPIO_PIN_15);
	hal_gpio_init_output(ADC_SW_3_PORT, GPIO_PIN_2);

#define T_SAMP_US		500

	for (;;) {
		ADCMUX_MOT_TEMP();
		// Wait longer on this one as some temperature sensors, e.g. the PT1000 change very little
		// and the voltage divider gives us bad resolution for it.
		chThdSleepMilliseconds(5);
		ADC_Value[ADC_IND_TEMP_MOTOR] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_12V_SENSE_V();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_12V_SENSE_V] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_MOS_TEMP1();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_TEMP_MOS] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_MOS_TEMP2();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_TEMP_MOS_2] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_12V_SENSE_I();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_12V_SENSE_I] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_5V_SENSE_V();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_5V_SENSE_V] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_MOS_TEMP3();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_TEMP_MOS_3] = ADC_Value[ADC_IND_ADC_MUX];

		ADCMUX_TEMP_DCDC();
		chThdSleepMicroseconds(T_SAMP_US);
		ADC_Value[ADC_IND_TEMP_DCDC] = ADC_Value[ADC_IND_ADC_MUX];
	}
}

bool hw_sample_shutdown_button(void) {
	chMtxLock(&shutdown_mutex);

	bt_diff = 0.0;

	for (int i = 0;i < 3;i++) {
		hal_gpio_init_input_analog(HW_SHUTDOWN_GPIO, GPIO_PIN_5);
		chThdSleep(5);
		float val1 = ADC_VOLTS(ADC_IND_SHUTDOWN);
		chThdSleepMilliseconds(1);
		float val2 = ADC_VOLTS(ADC_IND_SHUTDOWN);
		hal_gpio_init_output(HW_SHUTDOWN_GPIO, GPIO_PIN_5);
		chThdSleepMilliseconds(1);

		bt_diff += (val1 - val2);
	}

	chMtxUnlock(&shutdown_mutex);

	return (bt_diff > 0.12);
}

float hw100_400_get_temp(void) {
	float t1 = (1.0 / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOS]) / 10000.0) / 3380.0) + (1.0 / 298.15)) - 273.15);
	float t2 = (1.0 / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOS_2]) / 10000.0) / 3380.0) + (1.0 / 298.15)) - 273.15);
	float t3 = (1.0 / ((logf(NTC_RES(ADC_Value[ADC_IND_TEMP_MOS_3]) / 10000.0) / 3380.0) + (1.0 / 298.15)) - 273.15);
	float res = 0.0;

	if (t1 > t2 && t1 > t3) {
		res = t1;
	} else if (t2 > t1 && t2 > t3) {
		res = t2;
	} else {
		res = t3;
	}

	return res;
}
