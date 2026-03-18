/*
 * STM32F103 flash helper (minimal implementation for PlatformIO build).
 *
 * This is a simplified port of VESC flash_helper for F1 devices using page erase.
 * For now it provides the symbols needed by the migrated main.c and config logic.
 */

#include "flash_helper.h"

#include <string.h>
#include "stm32f1xx_hal.h"
#include "hw_config.h"

// NOTE: This port currently stubs advanced multi-image features (QML/Lisp).

uint16_t flash_helper_erase_new_app(uint32_t new_app_size) {
	(void)new_app_size;
	return 0;
}

uint16_t flash_helper_erase_bootloader(void) {
	return 0;
}

uint16_t flash_helper_write_new_app_data(uint32_t offset, uint8_t *data, uint32_t len) {
	(void)offset;
	(void)data;
	(void)len;
	return 0;
}

uint16_t flash_helper_erase_code(int ind) {
	(void)ind;
	return 0;
}

uint16_t flash_helper_write_code(int ind, uint32_t offset, uint8_t *data, uint32_t len) {
	(void)ind;
	(void)offset;
	(void)data;
	(void)len;
	return 0;
}

uint8_t* flash_helper_code_data(int ind) {
	(void)ind;
	return 0;
}

uint8_t* flash_helper_code_data_raw(int ind) {
	(void)ind;
	return 0;
}

uint32_t flash_helper_code_size(int ind) {
	(void)ind;
	return 0;
}

uint16_t flash_helper_code_flags(int ind) {
	(void)ind;
	return 0;
}

void flash_helper_jump_to_bootloader(void) {
	// Not implemented for this port.
}

uint8_t* flash_helper_get_sector_address(uint32_t fsector) {
	(void)fsector;
	return 0;
}

uint32_t flash_helper_verify_flash_memory(void) {
	// No integrity check yet for F103 port.
	return 0;
}

uint32_t flash_helper_verify_flash_memory_chunk(void) {
	return 0;
}

uint32_t flash_helper_app_crc(void) {
	return 0;
}

bool flash_helper_read_nvm(uint8_t *v, unsigned int len, unsigned int address) {
	const uint32_t base = FLASH_APP_PAGE_ADDR;
	if ((address + len) > FLASH_CONFIG_PAGE_SIZE) {
		return false;
	}
	memcpy(v, (const void *)(base + address), len);
	return true;
}

bool flash_helper_write_nvm(uint8_t *v, unsigned int len, unsigned int address) {
	// Simple implementation: unlock and program half-words; does not erase pages.
	// Suitable only for initial bring-up; full EEPROM emulation should be added later.
	const uint32_t base = FLASH_APP_PAGE_ADDR;
	if ((address + len) > FLASH_CONFIG_PAGE_SIZE) {
		return false;
	}

	if (HAL_FLASH_Unlock() != HAL_OK) {
		return false;
	}

	for (unsigned int i = 0; i < len; i += 2) {
		uint16_t half = 0xFFFF;
		half = v[i];
		if ((i + 1) < len) {
			half |= ((uint16_t)v[i + 1]) << 8;
		}
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, base + address + i, half) != HAL_OK) {
			(void)HAL_FLASH_Lock();
			return false;
		}
	}

	(void)HAL_FLASH_Lock();
	return true;
}

bool flash_helper_wipe_nvm(void) {
	// Erase the app config page.
	FLASH_EraseInitTypeDef erase = {0};
	uint32_t page_error = 0;

	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	erase.PageAddress = FLASH_APP_PAGE_ADDR;
	erase.NbPages = 1;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		return false;
	}
	HAL_StatusTypeDef st = HAL_FLASHEx_Erase(&erase, &page_error);
	(void)HAL_FLASH_Lock();

	return st == HAL_OK;
}

