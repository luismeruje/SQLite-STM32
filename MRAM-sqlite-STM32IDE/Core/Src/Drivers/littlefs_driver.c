/*
 * littlefs_driver.c
 *
 *  Created on: 05/05/2023
 *      Author: luisferreira
 */

#include "lfs.h"
#include "littlefs_driver.h"
#include "stm32h7xx_hal.h"
#include "flash_driver.h"
#include "FLASH_SECTOR_H7.h"
#include "definitions.h"
#include "MRAM_driver.h"
#include "mram_commons.h"

#define MRAM

#ifndef MRAM
int lfs_read_flash(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size){
	block += RESERVED_CODE_SECTORS;
	uint32_t address;
	if(block >= 8){
		address = FLASH_BANK_2_START_ADDRESS + ( (block - 8) * c->block_size) + off;
	} else {
		address = FLASH_BANK_1_START_ADDRESS + (block * c->block_size) + off;
	}

	Flash_Read_Data(address, buffer, size/4);

	//printf("Asked to read %ld bytes from flash block number %ld with offset %ld to buffer at address %lx\n", size, block, off, (uint32_t) buffer);
	return 0;
}



    // Program a region in a block. The block must have previously
    // been erased. Negative error codes are propagated to the user.
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_prog_flash(const struct lfs_config *c, lfs_block_t block,
		lfs_off_t off, const void *buffer, lfs_size_t size){
	block += RESERVED_CODE_SECTORS; //Leaving sector 0 of bank 1 for code.
	//printf("Asked to write %ld bytes from flash block number %ld with offset %ld to buffer at addres %lx\n", size, block, off, (uint32_t) buffer);
	uint32_t address = 0;
	if(block >= 8){
		address = FLASH_BANK_2_START_ADDRESS + ( (block - 8) * c->block_size) + off;
	} else {
		address = FLASH_BANK_1_START_ADDRESS + (block * c->block_size) + off;
	}

	flash_write_32bit_hal_copy(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint32_t) buffer, size/4);

	return 0;
}

    // Erase a block. A block must be erased before being programmed.
    // The state of an erased block is undefined. Negative error codes
    // are propagated to the user.
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_erase_sector_flash(const struct lfs_config *c, lfs_block_t block){
	block += RESERVED_CODE_SECTORS; //Leaving sector 0 of bank 1 for code.
	uint8_t bank = -1, sectorNumber = -1;
	if (block >= 8){
		bank = 2;
		sectorNumber = block-8;
	} else{
		bank = 1;
		sectorNumber = block;
	}
	erase_flash_sector( sectorNumber, bank);
	return 0;
}

    // Sync the state of the underlying block device. Negative error codes
    // are propagated to the user.
int lfs_sync_flash(const struct lfs_config *c){
	flush_flash();
	//printf("Asked to sync flash\n");
	return 0;
}
#else

//32-bit multiple reads
int lfs_read_flash(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size){
	__IO uint32_t address = MRAM_BANK_ADDR + ( block * c->block_size) + off;

    mram_read_32bit_blocks(address, buffer, size/4);

	return 0;
}


int lfs_prog_flash(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, const void *buffer, lfs_size_t size){
	__IO uint32_t address = MRAM_BANK_ADDR + ( block * c->block_size) + off;

	mram_write_32bit_blocks(address, buffer, size/4);

	return 0;
}

// Not sure it is needed with MRAM, but we zero out the values
int lfs_erase_sector_flash(const struct lfs_config *c, lfs_block_t block){

	__IO uint32_t address = MRAM_BANK_ADDR + ( block * c->block_size);


	mram_wipe_bytes( address, c->block_size);

	return 0;
}

int lfs_sync_flash(const struct lfs_config *c){
	//printf("Asked to sync flash\n");
	__DSB();
	return 0;
}

#endif

