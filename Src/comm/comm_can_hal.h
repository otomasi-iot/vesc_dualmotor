/*
 * CAN Hardware Abstraction Layer Header
 * Bridges STM32 HAL CAN interface with VESC comm_can.c
 * Converts ChibiOS CANRxFrame/CANTxFrame to STM32 HAL equivalents
 */

#ifndef COMM_CAN_HAL_H_
#define COMM_CAN_HAL_H_

#include "stm32f1xx_hal.h"
#include "datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// CAN Handle Structure (replaces ChibiOS CANDriver)
typedef struct {
	CAN_HandleTypeDef hal_handle;
	uint32_t tx_mailbox;
	bool initialized;
} can_hal_driver_t;

// RX Frame Structure (compatible with ChibiOS CANRxFrame)
typedef struct {
	uint32_t id;           // Standard or Extended ID
	uint8_t IDE;           // CAN_ID_STD or CAN_ID_EXT
	uint8_t RTR;           // CAN_RTR_DATA or CAN_RTR_REMOTE
	uint8_t DLC;           // Data Length Code (0-8)
	uint8_t data8[8];      // Data bytes
	uint32_t timestamp;    // RX timestamp
} CANRxFrame;

// TX Frame Structure (compatible with ChibiOS CANTxFrame)
typedef struct {
	uint32_t SID;          // Standard ID (11-bit)
	uint32_t EID;          // Extended ID (29-bit)
	uint8_t IDE;           // CAN_ID_STD or CAN_ID_EXT
	uint8_t RTR;           // CAN_RTR_DATA or CAN_RTR_REMOTE
	uint8_t DLC;           // Data Length Code (0-8)
	uint8_t data8[8];      // Data bytes
} CANTxFrame;

// CAN Configuration (replaces ChibiOS CANConfig)
typedef struct {
	uint32_t mcr;          // MCR register bits (ABOM, AWUM, TXFP, etc.)
	uint32_t btr;          // BTR register bits (SJW, TS2, TS1, BRP)
} CANConfig;

// CAN Return Values (compatible with msg_t)
#define MSG_OK        0
#define MSG_TIMEOUT   -1
#define MSG_RESET     -2

// CAN Mailbox (for compatibility)
#define CAN_ANY_MAILBOX 0xFF

// CAN RTR Flags
#define CAN_RTR_DATA   0
#define CAN_RTR_REMOTE 1

// CAN IDE Flags
#define CAN_IDE_STD 0
#define CAN_IDE_EXT 1

// CAN Constants
#define CAN_MCR_ABOM  (1 << 0)  // Automatic Bus-Off Management
#define CAN_MCR_AWUM  (1 << 1)  // Automatic Wake-Up Mode
#define CAN_MCR_TXFP  (1 << 2)  // Transmit FIFO Priority

// BTR Field Helpers (for 500 kBaud @ 72 MHz)
#define CAN_BTR_SJW(x)   ((x & 3) << 24)   // Synchronization Jump Width
#define CAN_BTR_TS2(x)   ((x & 7) << 20)   // Time Segment 2
#define CAN_BTR_TS1(x)   ((x & 15) << 16)  // Time Segment 1
#define CAN_BTR_BRP(x)   (x & 1023)        // Baud Rate Prescaler

// Function Prototypes (replacing ChibiOS equivalents)
int can_hal_init(can_hal_driver_t *driver, CAN_TypeDef *CANx, const CANConfig *config);
int can_hal_deinit(can_hal_driver_t *driver);
int can_hal_start(can_hal_driver_t *driver);
int can_hal_stop(can_hal_driver_t *driver);
int can_hal_transmit(can_hal_driver_t *driver, uint32_t mailbox, const CANTxFrame *frame, uint32_t timeout_ms);
int can_hal_get_rx_message(can_hal_driver_t *driver, CANRxFrame *frame);
int can_hal_set_baud_rate(can_hal_driver_t *driver, uint32_t baud_rate);

// Macro Compatibility Layer
#define canStart(driver, config)          can_hal_start(driver)
#define canTransmit(driver, mailbox, frame, timeout) can_hal_transmit(driver, mailbox, frame, timeout)

#ifdef __cplusplus
}
#endif

#endif /* COMM_CAN_HAL_H_ */
