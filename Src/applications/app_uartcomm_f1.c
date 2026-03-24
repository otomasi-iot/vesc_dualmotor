/*
	Copyright 2016 - 2019 Benjamin Vedder	benjamin@vedder.se
	Rewritten for STM32F103 HAL UART by VESC F103 port.

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

#pragma GCC optimize ("Os")

#include "app.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "packet.h"
#include "commands.h"
#include "hw_config.h"

// Settings
#define BAUDRATE				115200
#define UART_RX_BUF_SIZE		256

// Only one UART port on F103 hoverboard (USART3)
#define UART_NUMBER 1

// Thread
static StaticTask_t uart_thread_buffer;
static StackType_t uart_thread_stack[512];
static void *uart_process_thread(void *arg);

// Variables
static volatile bool thread_is_running = false;
static volatile bool uart_is_running[UART_NUMBER] = {false};
static SemaphoreHandle_t send_mutex = NULL;
static StaticSemaphore_t send_mutex_buf;
static PACKET_STATE_t packet_state[UART_NUMBER];

// UART RX circular buffer
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

// External UART handle from hw_setup.c
extern UART_HandleTypeDef huart3;

// Private functions
static void process_packet(unsigned char *data, unsigned int len, unsigned int port_number);
static void write_packet(unsigned char *data, unsigned int len, unsigned int port_number);

static void process_packet_1(unsigned char *data, unsigned int len) { process_packet(data, len, 0); }
static void write_packet_1(unsigned char *data, unsigned int len) { write_packet(data, len, 0); }
static void send_packet_1(unsigned char *data, unsigned int len) { app_uartcomm_send_packet(data, len, 0); }

static void write_packet(unsigned char *data, unsigned int len, unsigned int port_number) {
	if (port_number >= UART_NUMBER) return;
	if (uart_is_running[port_number]) {
		HAL_UART_Transmit(&huart3, data, len, 100);
	}
}

static void process_packet(unsigned char *data, unsigned int len, unsigned int port_number) {
	if (port_number >= UART_NUMBER) return;
	commands_process_packet(data, len, send_packet_1);
}

void app_uartcomm_initialize(void) {
	// UART3 is already initialized in hw_setup.c (MX_USART3_Init)
}

void app_uartcomm_start(UART_PORT port_number) {
	if ((int)port_number >= UART_NUMBER) return;

	packet_init(write_packet_1, process_packet_1, &packet_state[port_number]);

	if (!thread_is_running) {
		// Enable UART RX interrupt
		__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);

		osThreadNew((osThreadFunc_t)uart_process_thread, NULL,
			&(const osThreadAttr_t){
				.name = "uartcomm",
				.priority = osPriorityNormal,
				.stack_mem = uart_thread_stack,
				.stack_size = sizeof(uart_thread_stack),
				.cb_mem = &uart_thread_buffer,
				.cb_size = sizeof(uart_thread_buffer)
			});
		thread_is_running = true;
	}

	uart_is_running[port_number] = true;
}

void app_uartcomm_stop(UART_PORT port_number) {
	if ((int)port_number >= UART_NUMBER) return;
	if (uart_is_running[port_number]) {
		__HAL_UART_DISABLE_IT(&huart3, UART_IT_RXNE);
		uart_is_running[port_number] = false;
	}
}

void app_uartcomm_send_packet(unsigned char *data, unsigned int len, UART_PORT port_number) {
	if ((int)port_number >= UART_NUMBER) return;

	if (!send_mutex) {
		send_mutex = xSemaphoreCreateMutexStatic(&send_mutex_buf);
	}

	xSemaphoreTake(send_mutex, portMAX_DELAY);
	packet_send_packet(data, len, &packet_state[port_number]);
	xSemaphoreGive(send_mutex);
}

void app_uartcomm_configure(uint32_t baudrate, bool permanent_enabled, UART_PORT port_number) {
	(void)permanent_enabled;
	if ((int)port_number >= UART_NUMBER) return;

	if (baudrate > 0 && baudrate != huart3.Init.BaudRate) {
		huart3.Init.BaudRate = baudrate;
		HAL_UART_Init(&huart3);
	}
}

// Called from USART3 IRQ handler to push received bytes
void app_uartcomm_rx_irq(uint8_t byte) {
	uint16_t next = (rx_head + 1) % UART_RX_BUF_SIZE;
	if (next != rx_tail) {
		rx_buf[rx_head] = byte;
		rx_head = next;
	}
}

static void *uart_process_thread(void *arg) {
	(void)arg;

	for (;;) {
		osDelay(1);

		while (rx_tail != rx_head) {
			uint8_t byte = rx_buf[rx_tail];
			rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;

			for (int port = 0; port < UART_NUMBER; port++) {
				if (uart_is_running[port]) {
					packet_process_byte(byte, &packet_state[port]);
				}
			}
		}
	}

	return NULL;
}
