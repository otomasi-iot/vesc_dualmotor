/*
	Copyright 2016 - 2021 Benjamin Vedder	benjamin@vedder.se

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

#include "timeout.h"
#include "hwconf/hal_gpio.h"
#include "mc_interface.h"
#include "shutdown.h"
#include "utils.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"

extern IWDG_HandleTypeDef hiwdg;

// Private variables
static volatile bool init_done = false;
static volatile systime_t timeout_msec;
static volatile systime_t last_update_time;
static volatile float timeout_brake_current;
static volatile KILL_SW_MODE timeout_kill_sw_mode;
static volatile bool has_timeout;
static volatile bool kill_sw_active;
static volatile bool kill_sw_ext_set = false;
static volatile uint32_t feed_counter[MAX_THREADS_MONITOR];

static void *timeout_thread(void *arg);

void timeout_init(void) {
	timeout_msec = 1000;
	last_update_time = 0;
	timeout_brake_current = 0.0;
	timeout_kill_sw_mode = KILL_SW_MODE_DISABLED;
	has_timeout = false;
	kill_sw_active = false;
	init_done = true;

	// IWDG is initialized in hw_setup.c for this STM32F103 port.
	osDelay(10);

	(void)osThreadNew((osThreadFunc_t)timeout_thread, NULL,
		&(const osThreadAttr_t){
			.name = "timeout",
			.priority = osPriorityHigh,
			.stack_size = 512
		});
}

void timeout_configure(systime_t timeout, float brake_current, KILL_SW_MODE kill_sw_mode) {
	timeout_msec = timeout;
	timeout_brake_current = brake_current;
	timeout_kill_sw_mode = kill_sw_mode;
}

void timeout_reset(void) {
	last_update_time = chVTGetSystemTimeX();
}

bool timeout_has_timeout(void) {
	return has_timeout;
}

float timeout_secs_since_update(void) {
	return UTILS_AGE_S(last_update_time);
}

bool timeout_kill_sw_active(void) {
	return kill_sw_active;
}

systime_t timeout_get_timeout_msec(void) {
	return timeout_msec;
}

float timeout_get_brake_current(void) {
	return timeout_brake_current;
}

KILL_SW_MODE timeout_get_kill_sw_mode(void) {
	return timeout_kill_sw_mode;
}

void timeout_set_kill_sw_ext(bool kill_set) {
	kill_sw_ext_set = kill_set;
}

void timeout_feed_WDT(uint8_t index) {
	++feed_counter[index];
}

void timeout_configure_IWDT_slowest(void) {
	if (!init_done) {
		return;
	}

	// For this port, watchdog configuration is owned by hw_setup.c.
	SHUTDOWN_SET_SAMPLING_DISABLED(true);
}

void timeout_configure_IWDT(void) {
	if (!init_done) {
		return;
	}

	SHUTDOWN_SET_SAMPLING_DISABLED(false);
}

bool timeout_had_IWDG_reset(void) {
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
		__HAL_RCC_CLEAR_RESET_FLAGS();
		return true;
	}
	return false;
}

static void *timeout_thread(void *arg) {
	(void)arg;

	for(;;) {
		// Kill-switch inputs not mapped yet for this port.
		(void)timeout_kill_sw_mode;
		bool kill_sw = false;

		if (kill_sw_ext_set) {
			kill_sw = true;
		}

		if (kill_sw || (timeout_msec != 0 && chVTTimeElapsedSinceX(last_update_time) > MS2ST(timeout_msec))) {
			if (!has_timeout && !kill_sw_active) {
				mc_interface_release_motor_override();
			}
			mc_interface_unlock();
			mc_interface_select_motor_thread(1);
			mc_interface_set_brake_current(timeout_brake_current);
			mc_interface_select_motor_thread(2);
			mc_interface_set_brake_current(timeout_brake_current);

			if (kill_sw) {
				mc_interface_ignore_input_both(20);
			} else {
				has_timeout = true;
			}
		} else {
			has_timeout = false;
		}

		kill_sw_active = kill_sw;

		for (int i = 0; i < MAX_THREADS_MONITOR; i++) {
			feed_counter[i] = 0;
		}

		(void)HAL_IWDG_Refresh(&hiwdg);

		osDelay(10);
	}

	return NULL;
}
