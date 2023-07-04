/*
Copyright 2023 INESC TEC
This software is authored by:
Luís Manuel Meruje Ferreira (INESC TEC) *
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at *
http://www.apache.org/licenses/LICENSE-2.0 */

#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include "stm32h7xx_hal.h"

uint32_t erase_flash_sector(unsigned int sectorNumber, unsigned int bank);

HAL_StatusTypeDef flash_write_32bit(uint32_t address, uint32_t data);
__RAM_FUNC HAL_StatusTypeDef flash_write_32bit_hal_copy(uint32_t TypeProgram, uint32_t FlashAddress, uint32_t RowAddress, uint16_t nrRows);

int flush_flash();

#endif //FLASH_MANAGER_H
