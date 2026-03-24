/**
 ******************************************************************************
 * @file    eeprom_f103.h
 * @brief   EEPROM emulation for STM32F103 using HAL Flash API
 *          Adapted from STMicroelectronics EEPROM emulation example
 *          Modified for STM32F1 HAL compatibility
 ******************************************************************************
 */

#ifndef __EEPROM_F103_H
#define __EEPROM_F103_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "datatypes.h"

/* Exported constants --------------------------------------------------------*/

/* STM32F103RET6 Flash organization:
 * Total Flash: 512KB (0x08000000 - 0x0807FFFF)
 * Page size: 2KB (2048 bytes)
 * We use last 2 pages for EEPROM emulation:
 * - Page 254 (0x0807F000 - 0x0807F7FF): 2KB
 * - Page 255 (0x0807F800 - 0x0807FFFF): 2KB
 */

#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE         ((uint32_t)0x800)  /* 2KB per page on STM32F103 */
#endif

/* EEPROM start address in Flash - use last 4KB of 512KB Flash */
#define EEPROM_START_ADDRESS    ((uint32_t)0x0807F000) /* Last 4KB of Flash */

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (FLASH_PAGE_SIZE - 1)))
#define PAGE0_ID                254  /* Flash page number */

#define PAGE1_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 0x0800))
#define PAGE1_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (2 * FLASH_PAGE_SIZE - 1)))
#define PAGE1_ID                255  /* Flash page number */

/* Used Flash pages for EEPROM emulation */
#define PAGE0                   ((uint16_t)0x0000)
#define PAGE1                   ((uint16_t)0x0001)

/* No valid page define */
#define NO_VALID_PAGE           ((uint16_t)0x00AB)

/* Page status definitions */
#define ERASED                  ((uint16_t)0xFFFF)     /* Page is empty */
#define RECEIVE_DATA            ((uint16_t)0xEEEE)     /* Page is marked to receive data */
#define VALID_PAGE              ((uint16_t)0x0000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE    ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE     ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL               ((uint8_t)0x80)

/* Variables' number - calculate based on configuration sizes */
#define NB_OF_VAR               ((uint16_t)((2 * sizeof(mc_configuration) + sizeof(app_configuration) + 1) / 2) + \
                                EEPROM_VARS_HW * 2 + EEPROM_VARS_CUSTOM * 2 + (sizeof(backup_data) + 1) / 2)

/* Flash operation status for STM32F1 HAL */
#define FLASH_COMPLETE          HAL_OK
#define FLASH_ERROR             HAL_ERROR
#define FLASH_TIMEOUT           HAL_TIMEOUT

/* Exported types ------------------------------------------------------------*/
typedef HAL_StatusTypeDef FLASH_Status;

/* Exported functions ------------------------------------------------------- */
uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_F103_H */
