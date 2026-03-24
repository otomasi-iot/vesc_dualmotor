/*
	Copyright 2024 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "pwm_servo.h"
#include "ch.h"
#include "hal.h"
#include "conf_general.h"
#include "utils_math.h"
#include "hw_config.h"

#pragma GCC optimize ("Os")

// Settings
#define TIM_CLOCK			2000000 // Hz

// F103 hoverboard: PWM servo output on TIM2_CH2 (PB3)
#ifndef SERVO_OUT_RATE_HZ
#define SERVO_OUT_RATE_HZ		50
#endif
#ifndef SERVO_OUT_PULSE_MIN_US
#define SERVO_OUT_PULSE_MIN_US	1000
#endif
#ifndef SERVO_OUT_PULSE_MAX_US
#define SERVO_OUT_PULSE_MAX_US	2000
#endif

// Private variables
static volatile bool m_is_running = false;

void pwm_servo_init_servo(void) {

	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->CR1 = 0;
	TIM2->PSC = (uint16_t)((SYSTEM_CORE_CLOCK) / TIM_CLOCK) - 1;
	TIM2->ARR = (uint16_t)((uint32_t)TIM_CLOCK / (uint32_t)SERVO_OUT_RATE_HZ);
	TIM2->EGR = TIM_EGR_UG;

	// CH2 PWM mode 1
	TIM2->CCMR1 = (TIM2->CCMR1 & ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S)) |
				  (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE);
	TIM2->CCER |= TIM_CCER_CC2E;
	TIM2->CCR2 = (uint32_t)(((uint64_t)(SERVO_OUT_PULSE_MIN_US + SERVO_OUT_PULSE_MAX_US) / 2) * TIM_CLOCK / 1000000);

	TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;

	m_is_running = true;
}

void pwm_servo_stop(void) {
	if (m_is_running) {
		TIM2->CCER &= ~TIM_CCER_CC2E;
	}
	m_is_running = false;
}

void pwm_servo_set_servo_out(float output) {
	if (!m_is_running) return;

	utils_truncate_number(&output, 0.0, 1.0);

	float us = (float)SERVO_OUT_PULSE_MIN_US + output *
			(float)(SERVO_OUT_PULSE_MAX_US - SERVO_OUT_PULSE_MIN_US);
	TIM2->CCR2 = (uint32_t)(us * (float)TIM_CLOCK / 1000000.0f);
}

bool pwm_servo_is_running(void) {
	return m_is_running;
}
