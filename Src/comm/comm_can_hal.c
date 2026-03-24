/*
 * CAN Hardware Abstraction Layer Implementation
 * STM32 HAL CAN driver for VESC firmware
 */

#include "comm_can_hal.h"
#include "stm32f1xx_hal.h"

/**
 * Initialize CAN peripheral with HAL
 * Converts ChibiOS CANConfig to STM32 HAL initialization
 */
int can_hal_init(can_hal_driver_t *driver, CAN_TypeDef *CANx, const CANConfig *config) {
	if (!driver || !CANx) {
		return MSG_RESET;
	}

	driver->hal_handle.Instance = CANx;
	driver->hal_handle.Init.Mode = CAN_MODE_NORMAL;
	driver->hal_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
	driver->hal_handle.Init.AutoBusOff = ENABLE;
	driver->hal_handle.Init.AutoWakeUp = DISABLE;
	driver->hal_handle.Init.AutoRetransmission = ENABLE;
	driver->hal_handle.Init.ReceiveFifoLocked = DISABLE;
	driver->hal_handle.Init.TimeTriggeredMode = DISABLE;

	// Parse BTR register from config
	// BTR format: [SJW(2), TS2(3), TS1(4), BRP(10)]
	uint32_t btr = config->btr;
	uint32_t sjw = (btr >> 24) & 0x3;
	uint32_t ts2 = (btr >> 20) & 0x7;
	uint32_t ts1 = (btr >> 16) & 0xF;
	uint32_t brp = btr & 0x3FF;

	// Set prescaler and timing segments
	driver->hal_handle.Init.Prescaler = brp + 1;  // STM32 HAL uses 1-based prescaler
	
	// Map ChibiOS time segments to HAL equivalents
	switch (ts1) {
		case 9: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_9TQ; break;
		case 8: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_8TQ; break;
		case 7: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_7TQ; break;
		case 6: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_6TQ; break;
		case 5: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_5TQ; break;
		case 4: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_4TQ; break;
		case 3: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_3TQ; break;
		default: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_3TQ; break;
	}

	switch (ts2) {
		case 2: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_2TQ; break;
		case 3: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_3TQ; break;
		case 4: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_4TQ; break;
		default: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_2TQ; break;
	}

	// Initialize HAL CAN
	if (HAL_CAN_Init(&driver->hal_handle) != HAL_OK) {
		return MSG_RESET;
	}

	driver->initialized = true;
	return MSG_OK;
}

/**
 * Deinitialize CAN peripheral
 */
int can_hal_deinit(can_hal_driver_t *driver) {
	if (!driver) {
		return MSG_RESET;
	}

	HAL_CAN_DeInit(&driver->hal_handle);
	driver->initialized = false;
	return MSG_OK;
}

/**
 * Start CAN peripheral
 */
int can_hal_start(can_hal_driver_t *driver) {
	if (!driver || !driver->initialized) {
		return MSG_RESET;
	}

	if (HAL_CAN_Start(&driver->hal_handle) != HAL_OK) {
		return MSG_RESET;
	}

	// Activate RX interrupts
	if (HAL_CAN_ActivateNotification(&driver->hal_handle, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK) {
		return MSG_RESET;
	}

	return MSG_OK;
}

/**
 * Stop CAN peripheral
 */
int can_hal_stop(can_hal_driver_t *driver) {
	if (!driver) {
		return MSG_RESET;
	}

	HAL_CAN_DeactivateNotification(&driver->hal_handle, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING);
	HAL_CAN_Stop(&driver->hal_handle);

	return MSG_OK;
}

/**
 * Transmit CAN message
 * Converts CANTxFrame to STM32 HAL CAN_TxHeaderTypeDef
 */
int can_hal_transmit(can_hal_driver_t *driver, uint32_t mailbox, const CANTxFrame *frame, uint32_t timeout_ms) {
	if (!driver || !frame) {
		return MSG_TIMEOUT;
	}

	CAN_TxHeaderTypeDef tx_header = {0};
	uint32_t tx_mailbox;
	uint32_t ticks;

	// Set frame ID
	if (frame->IDE == CAN_IDE_EXT) {
		tx_header.IDE = CAN_ID_EXT;
		tx_header.ExtId = frame->EID & 0x1FFFFFFF;  // 29-bit extended ID
	} else {
		tx_header.IDE = CAN_ID_STD;
		tx_header.StdId = frame->SID & 0x7FF;  // 11-bit standard ID
	}

	// Set RTR and DLC
	tx_header.RTR = (frame->RTR == CAN_RTR_REMOTE) ? CAN_RTR_REMOTE : CAN_RTR_DATA;
	tx_header.DLC = (frame->DLC > 8) ? 8 : frame->DLC;

	// Convert timeout_ms to ticks (approximate)
	// Assuming osKernelGetTickFreq() = 1000 (1 ms per tick)
	ticks = (timeout_ms > 0) ? timeout_ms : 0xFFFFFFFF;

	// Try to add message to TX mailbox
	if (HAL_CAN_AddTxMessage(&driver->hal_handle, &tx_header, (uint8_t *)frame->data8, &tx_mailbox) != HAL_OK) {
		return MSG_TIMEOUT;
	}

	return MSG_OK;
}

/**
 * Receive CAN message
 * Converts STM32 HAL CAN_RxHeaderTypeDef to CANRxFrame
 */
int can_hal_get_rx_message(can_hal_driver_t *driver, CANRxFrame *frame) {
	if (!driver || !frame) {
		return MSG_RESET;
	}

	CAN_RxHeaderTypeDef rx_header = {0};
	uint8_t rx_data[8] = {0};

	// Try to get from FIFO0 first
	if (HAL_CAN_GetRxMessage(&driver->hal_handle, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
		goto process_frame;
	}

	// Try FIFO1
	if (HAL_CAN_GetRxMessage(&driver->hal_handle, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
		goto process_frame;
	}

	// No message available
	return MSG_TIMEOUT;

process_frame:
	// Convert HAL header to CANRxFrame
	frame->IDE = rx_header.IDE;
	frame->RTR = rx_header.RTR;
	frame->DLC = rx_header.DLC;
	frame->FMI = 0;  // Filter Match Index (not used)

	if (rx_header.IDE == CAN_ID_EXT) {
		frame->EID = rx_header.ExtId;
		frame->SID = 0;
	} else {
		frame->SID = rx_header.StdId;
		frame->EID = 0;
	}

	// Copy data
	memcpy(frame->data8, rx_data, frame->DLC);

	return MSG_OK;
}

/**
 * Set CAN Baud Rate
 * Common baud rates: 250k, 500k, 1000k
 */
int can_hal_set_baud_rate(can_hal_driver_t *driver, uint32_t baud_rate) {
	if (!driver || !driver->initialized) {
		return MSG_RESET;
	}

	// For 72 MHz system clock, calculate prescaler and timing segments
	// Assuming 1 sample point per bit
	uint32_t prescaler = 0;
	uint32_t ts1 = 6;
	uint32_t ts2 = 1;
	uint32_t sjw = 1;

	switch (baud_rate) {
		case 125000:  // 125 kBaud
			prescaler = 36;
			ts1 = 6;
			ts2 = 1;
			break;
		case 250000:  // 250 kBaud
			prescaler = 18;
			ts1 = 6;
			ts2 = 1;
			break;
		case 500000:  // 500 kBaud
			prescaler = 9;
			ts1 = 6;
			ts2 = 1;
			break;
		case 1000000: // 1 MBaud
			prescaler = 4;
			ts1 = 7;
			ts2 = 2;
			break;
		default:
			return MSG_RESET;
	}

	// Update HAL init and reinitialize
	driver->hal_handle.Init.Prescaler = prescaler;

	// Map time segments
	switch (ts1) {
		case 7: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_7TQ; break;
		case 6: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_6TQ; break;
		default: driver->hal_handle.Init.TimeSeg1 = CAN_BS1_6TQ; break;
	}
	switch (ts2) {
		case 2: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_2TQ; break;
		case 1: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_1TQ; break;
		default: driver->hal_handle.Init.TimeSeg2 = CAN_BS2_1TQ; break;
	}

	if (HAL_CAN_Init(&driver->hal_handle) != HAL_OK) {
		return MSG_RESET;
	}

	return MSG_OK;
}

// CAN Receive Callback (called from ISR)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	// This callback will be handled by comm_can.c
	// Signal the CAN read thread that a message is available
	extern osEventFlagsId_t can_process_event;
	if (can_process_event) {
		osEventFlagsSet(can_process_event, 0x01);
	}
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	// Same handling as FIFO0
	extern osEventFlagsId_t can_process_event;
	if (can_process_event) {
		osEventFlagsSet(can_process_event, 0x01);
	}
}

// CAN Error Callback
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
	// Handle CAN errors (bus off, warning, passive, etc.)
	uint32_t error = HAL_CAN_GetError(hcan);
	
	if (error & HAL_CAN_ERROR_EWG) {
		// Error warning
	}
	if (error & HAL_CAN_ERROR_EPV) {
		// Error passive
	}
	if (error & HAL_CAN_ERROR_BOF) {
		// Bus off
	}
	if (error & HAL_CAN_ERROR_STF) {
		// Stuff error
	}
	if (error & HAL_CAN_ERROR_FOR) {
		// Form error
	}
	if (error & HAL_CAN_ERROR_ACK) {
		// Acknowledge error
	}
	if (error & HAL_CAN_ERROR_BR) {
		// Bit recessive error
	}
	if (error & HAL_CAN_ERROR_BD) {
		// Bit dominant error
	}
	if (error & HAL_CAN_ERROR_CRC) {
		// CRC error
	}
}
