#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include "stm32h7xx_hal.h"

uint32_t erase_flash_sector(unsigned int sectorNumber, unsigned int bank);

HAL_StatusTypeDef flash_write_32bit(uint32_t address, uint32_t data);
__RAM_FUNC HAL_StatusTypeDef flash_write_32bit_hal_copy(uint32_t TypeProgram, uint32_t FlashAddress, uint32_t RowAddress, uint16_t nrRows);

int flush_flash();

#endif //FLASH_MANAGER_H
