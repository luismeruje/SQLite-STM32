#ifndef MRAM_DRIVER
#define MRAM_DRIVER

#include "stm32h7xx_hal.h"

//TODO: as inline functions
__RAM_FUNC void mram_write_16bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_write_8bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_write_32bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_write_64bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_read_16bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_read_8bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_read_32bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_read_64bit_blocks(uint32_t address, void *data, uint16_t nrWords);

__RAM_FUNC void mram_wipe();

uint16_t compare_array_with_MRAM(uint32_t mramAddress, void *data, uint16_t numBytes);

void mram_read(void * dest, uint32_t mramAddr, uint16_t numBytes);
void mram_write (uint32_t mramAddr, void * src, uint16_t numBytes);
void * mram_malloc(uint32_t num_bytes);
void * mram_free(void *);
void mram_wipe_bytes(uint32_t address, uint32_t bytes);
#endif
