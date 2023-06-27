#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define BANK1_START_ADDR 0x080000000
#define BANK2_START_ADDR 0x081000000
#define SECTOR_SIZE 0x001000000
#define FLASH_TIMEOUT_VALUE 50000U

//NOTE: For use with stm32h743
//Entire sector must be erased before being used

//Inspired on: https://stackoverflow.com/questions/67427977/stm32-flash-write-causes-multiple-hardfault-errors
//And also on examples from: ControllersTech.com

//INFO: Flash has 256bits per row. Buffers have 256 bit size. Erases are done per sector of 128KB. Writes can be done in . There are queues for write/read operations, and writes can be done async through interrupt callbacks.
//FROM MANUAL: A 256-bit write data buffer is associated with each AXI interface. It supports multiple
//write access types (64 bits, 32 bits, 16 bits and 8 bits).

//TODO: Flash programming in parallel up to 64 bits is possible. Check manual. I believe it is currently set to 32 bits in parallel. Should this be taken into account in the results?
//TODO: Flash protection settings may also affect performance of Flash
//TODO: "Using a force-write operation prevents the application from updating later the missing bits with a value different from 1." Why?
uint32_t erase_flash_sector(unsigned int sectorNumber, unsigned int bank){
	HAL_FLASH_Unlock();
	int result = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTORError;

	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks		  = bank;
	EraseInitStruct.Sector        = sectorNumber;
	EraseInitStruct.NbSectors     = 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return HAL_FLASH_GetError ();
	}


	HAL_FLASH_Lock();

	return result;

}


//From HAL library: FlashAddress specifies the address to be programmed.  --> address that is passed to HAL_FLASH_PROGRAM
//*         This parameter shall be aligned to the Flash word:
//*          - 256 bits for STM32H74x/5X devices (8x 32bits words)

HAL_StatusTypeDef flash_write_8bit(uint32_t address, uint8_t data){
	//HAL_StatusTypeDef result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data);
	*(__IO uint8_t*)(address) = data;
	return 0;
}

HAL_StatusTypeDef flash_write_16bit(uint32_t address, uint8_t data){
	HAL_StatusTypeDef result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data);
	return result;
}

__RAM_FUNC HAL_StatusTypeDef flash_write_32bit(uint32_t address, uint32_t data){
	__HAL_LOCK(&pFlash);
	FLASH_WaitForLastOperation((uint32_t)1000, 0x02U);
	SET_BIT(FLASH->CR2, FLASH_CR_PG);
	__ISB();
	__DSB();
	*(__IO uint32_t*)(address) = data;
	__ISB();
	__DSB();
	/*HAL_StatusTypeDef result =*/ FLASH_WaitForLastOperation(1000, 0x02U);
	CLEAR_BIT(FLASH->CR2, FLASH_CR_PG);
	__HAL_UNLOCK(&pFlash);

	return 0;
}


int flush_flash(){
	FLASH_WaitForLastOperation((uint32_t)1000, 0x02U);
	__DSB();
}

//WARNING: Assuming write starts at beggining of a 256bit row
//Basically a copy of the hal interface function
//TODO: test writes that are not multiples of 256 bits
__RAM_FUNC HAL_StatusTypeDef flash_write_32bit_hal_copy(uint32_t TypeProgram, uint32_t FlashAddress, uint32_t SrcAddress, uint16_t nrWords)
{
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_OK;
	__IO uint32_t *dest_addr = (__IO uint32_t *)FlashAddress;
	__IO uint32_t *src_addr = (__IO uint32_t*)SrcAddress;
	__IO uint32_t bank;
	__IO uint16_t nrRows = (nrWords * 32) / 256;

	/*__IO bool endsInFullWord = true;

	if((nrWords * 32) % 256){
		endsInFullWord = false;
	}*/
	__IO uint8_t row_index = FLASH_NB_32BITWORD_IN_FLASHWORD;

	/* Check the parameters */
	assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));
	assert_param(IS_FLASH_PROGRAM_ADDRESS(FlashAddress));


	  /* Process Locked */
	  __HAL_LOCK(&pFlash);

	  if(IS_FLASH_PROGRAM_ADDRESS_BANK1(FlashAddress))
	  {
		bank = FLASH_BANK_1;
	  }
	  else if(IS_FLASH_PROGRAM_ADDRESS_BANK2(FlashAddress))
	  {
		bank = FLASH_BANK_2;
	  }
	  else
	  {
		return HAL_ERROR;
	  }

	  /* Reset error code */
	  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

	  /* Wait for last operation to be completed */
	  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE, bank);

	  if(status == HAL_OK)
	  {
		  if(bank == FLASH_BANK_1)
		  {
			/* Set PG bit */
			SET_BIT(FLASH->CR1, FLASH_CR_PG);
		  }
		  else
		  {
		    /* Set PG bit */
			SET_BIT(FLASH->CR2, FLASH_CR_PG);
		  }


		  /* Program the flash word */
		  for(int i = 0; i < nrRows; i++){
			  row_index = FLASH_NB_32BITWORD_IN_FLASHWORD;
			  do
			  {
				*dest_addr = *src_addr;
				//HAL_Delay(100);
				dest_addr++;
				src_addr++;
				row_index--;
			  } while (row_index != 0U);

			  //Write instructions have a buffer of two operations. It seems one can be executing, and one in queue

			  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE, bank);
			  if (status != HAL_OK){
				return status;
			  }
		  }

	  }

	  /*
	  if(!endInFullWorld){
		 //Assert FW flag
	  }*/

	  /* Wait for last operation to be completed */
	  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE, bank);


	  if(bank == FLASH_BANK_1)
	  {
		 /* If the program operation is completed, disable the PG */
		 CLEAR_BIT(FLASH->CR1, FLASH_CR_PG);
	  }
	  else
	  {
		 /* If the program operation is completed, disable the PG */
		 CLEAR_BIT(FLASH->CR2, FLASH_CR_PG);
	  }


	 /* Process Unlocked */
	 __HAL_UNLOCK(&pFlash);

	return status;
}


HAL_StatusTypeDef flash_write_64bit(uint32_t address, uint8_t data){
	HAL_StatusTypeDef result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data);
	return result;
}
