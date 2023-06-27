#include "stm32h7xx_hal.h"
#include "mram_commons.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Note: Accessing memory without bit shift. Not needed!

__RAM_FUNC void mram_write_8bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint8_t *mramDest = (uint8_t *) address;
	uint8_t *dataSrc = (uint8_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint8_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
}


__RAM_FUNC void mram_write_16bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint16_t *mramDest = (uint16_t *) address;
	uint16_t *dataSrc = (uint16_t *) data;

	for(int i = 0; i < nrWords; i++){
		*(uint16_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
}

__RAM_FUNC void mram_write_32bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint32_t *mramDest = (uint32_t *) address;
	uint32_t *dataSrc = (uint32_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint32_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc++;
	}
}

__RAM_FUNC void mram_write_64bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint64_t *mramDest = (uint64_t *) address;
	uint64_t *dataSrc = (uint64_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint64_t *) mramDest = *dataSrc;
		mramDest++;
		dataSrc ++;
	}
}

__RAM_FUNC void mram_read_8bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint8_t *mramSrc = (uint8_t *) address;
	uint8_t *dataDest = (uint8_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint8_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
}


__RAM_FUNC void mram_read_16bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint16_t *mramSrc = (uint16_t *) address;
	uint16_t *dataDest = (uint16_t *) data;

	for(int i = 0; i < nrWords; i++){
		*(uint16_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
}

__RAM_FUNC void mram_read_32bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint32_t *mramSrc = (uint32_t *) address;
	uint32_t *dataDest = (uint32_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint32_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest++;
	}
}

__RAM_FUNC void mram_read_64bit_blocks(uint32_t address, void *data, uint16_t nrWords){
	uint64_t *mramSrc = (uint64_t *) address;
	uint64_t *dataDest = (uint64_t *) data;
	for(int i = 0; i < nrWords; i++){
		*(uint64_t *) dataDest = *mramSrc;
		mramSrc++;
		dataDest ++;
	}
}


uint16_t compare_array_with_MRAM(uint32_t mramAddress, const void *data, uint32_t numBytes){
	uint32_t nrMismatchedBlocks = 0;

	//if(memcmp((void *) mramAddress, (void *) data, (size_t) numBytes)){
		//nrMismatchedBlocks = 1;
	for(int i = 0; i < numBytes / 2; i++){
		if(* (uint16_t *) mramAddress != *(uint16_t *) data){
			nrMismatchedBlocks++;

			printf("Data: %s\n\n MRAM: %s\n", (char *) data, (char *) mramAddress);
			printf("Data address: %p\nMRAM address: %p\n",(void *) data, (void *) mramAddress);
			if(* (uint16_t *) mramAddress != *(uint16_t *) data){
				printf("No error on second comparison\n");
			}
		}
	}

	return nrMismatchedBlocks;
}

void mram_read(void * dest, uint32_t mramAddr, uint16_t numBytes){
	//TODO: handle misaligned single byte reads with bit shift
	if(numBytes % 2){
		//Single byte reads are not supported at misaligned addresses, so I just force numBytes to be even.
		printf("ERROR: mram memory copy only supports copies of an even number of bytes\n");
		return;
	}

	uint16_t *destAddr = (uint16_t *) dest;
	uint16_t * mramSrcAddr = (uint16_t *) mramAddr;


	for(int i = 0; i < numBytes / 2; i++){
		*destAddr = *mramSrcAddr;
		//printf("Read %u from address %p\n", *mramSrcAddr, mramSrcAddr);
		//HAL_Delay(10*1000);
		destAddr++;
		mramSrcAddr++;


	}

}

void mram_write (uint32_t mramAddr, void * src, uint16_t numBytes){
	//TODO: change to uint8_t if there is one byte left
	if(numBytes % 2){
		printf("ERROR: mram single byte write not implemented for this method\n");
		return;
	}

	uint16_t *srcAddr = (uint16_t *) src;
	uint16_t * mramDestAddr = (uint16_t *) mramAddr;

	/*if(mramAddr >= MRAM_BANK_ADDR_LIMIT_4Mb - 1){
		printf("Trying to write to address %p which is outside limits.\n", (void *) mramAddr);
		HAL_Delay(10*1000);
	}*/
	//printf("Trying to write to address %p \n", (void *) mramAddr);

	for(int i = 0; i < numBytes / 2; i++){
		*mramDestAddr = *srcAddr;
		mramDestAddr++;
		srcAddr++;
	}

}

__RAM_FUNC void mram_wipe(){
	uint32_t addr = MRAM_BANK_ADDR;
	for(; addr < MRAM_BANK_ADDR_LIMIT_4Mb; addr+=2){
		* (uint16_t *) addr = 0;
	}
}

__RAM_FUNC void mram_wipe_bytes(uint32_t address, uint32_t bytes){
	for(int i = 0; i < bytes / 2; i++){
		* (uint16_t *) address = 0;
		address+=2;
	}
}
