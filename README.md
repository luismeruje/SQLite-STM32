# SQLite port to STM32 microcontrollers
This repository intends to help developers run SQLite on STM32 devices. 
We provide a basic benchmark to insert/read values from SQLite, which showcases the code necessary for basic SQLite setup and query execution.

We provide two versions of SQLite: the standard version, for experimenting with the benchmark and making sure everything is running as expected under a normal UNIX environment; and a version configured specifically for STM32, in the form of an STM32CubeIDE project. This version contains a modified SQLite amalgamation, i.e., the entire SQLite code in a single file ([click here](https://www.sqlite.org/amalgamation.html) for details), and contains code for running SQLite both on the devices embedded FLASH, and on an external storage device (MRAM). For a detailed description of how this amalgamation was created, click [here](./Amalgamation.md).


**This code is not intended to be used in production environments, use it at your own risk.**

### Compatibility
This port has only been tested in the microcontrollers described below. If you are able to successfully run STM32 in a different microcontroller, please make a push-request so that we can add it to the list of supported devices. Keep in mind that the current amalgamation occupies roughly 700KB, so you will need a microcontroller with at least 1MB of flash storage.

Supported devices:
- STM32H743ZI

## Testing SQLite on a UNIX system

Before attempting to run SQLite on the STM32 it is a good idea to test it in a normal UNIX environment to check that it runs as expected.

First, make sure the benchmark is correctly configured, by setting the appropriate MACROS at the beginning of the  `bench_sqlite3.c.temp` file.

```bash
vim bench_sqlite3.c.temp
```

```C++
#define OPERATIONS_PER_TRANSACTION 2   //Number of operations per transaction (e.g, 2 inserts per transaction in this case)
#define NR_TRANSACTIONS 2500           //Total number of executed transactions
// #define STM32 1                  //Leave commented, unless you are running this benchmark on STM32
```

Then copy the template files to the standalone folder, compile the code and run. 

```bash
cp bench_sqlite3.c.temp SQLite-standalone/src/bench_sqlite3.c
cp bench_sqlite3.h.temp SQLite-standalone/inc/bench_sqlite3.h
cd SQLite-standalone
make
./sqlite3_bench
```
For a successful test, the following (or a similar) output is expected:
```C++
SQLite version: 3.42.0
Good create
Test setup
Inserts per transaction: 2
Nr transactions: 2500
Number transactions per batch: 614
Nr batches: 4
LeftoverBatch? Yes
Nr transactions leftover batch: 44
Took 0.112262 ms to insert 5000 records 
Took 0.746329 ms to read 5000 records 
5000   //Number of records in the database  at the end of the test
Good drop
```

Notice that the benchmark splits execution into multiple batches. This is done to keep memory consumption low in the STM32, since the query strings are generated before the actual timed run. Also, a series of `helloworld*` files are created in the standalone folder during the test. These are the SQLite data files.

## Testing SQLite on STM32
The `MRAM-sqlite-STM32` folder contains an STM32CubeIDE project to run SQLite on STM32 devices. The project is set for a NUCLEO-H743ZI board.
Inside there is an SQLite amalgamation, more specifically at `MRAM-sqlite-STM32IDE/Core/Src/sqlite3.c`, which is configured specifically to run sqlite on the STM32.
Some options include setting single thread execution, enabling the memsys5 memory manager, and omitting components to lower resource consumption.
For a detailed description of how this amalgamation was created click [here](./Amalgamation.md).

SQLite requires access to a file system, so we have selected LittleFS to provide that functionality.

Let's start by enabling the STM32 mode at the beginning of the `bench_sqlite3.c.temp` file.

```bash
vim bench_sqlite3.c.temp
```

```C++
#define OPERATIONS_PER_TRANSACTION 2   //Number of operations per transaction (e.g, 2 inserts per transaction in this case)
#define NR_TRANSACTIONS 10             //Total number of executed transactions
#define STM32 1                  //<-- ENABLE THIS LINE
```

Then copy the benchmark files to the appropriate folder:
```bash
cp bench_sqlite3.c.temp MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c
cp bench_sqlite3.h.temp MRAM-sqlite-STM32IDE/Core/Inc/bench_sqlite3.h
```

Finally, open the root folder of the repository as an STM32CubeIDE workspace, or just import the project `MRAM-sqlite-STM32` to your STM32IDE. 

**The standard output is redirected to the SWV ITM Data Console,** so make sure you have SWV correctly enabled. For further details, see [this tutorial](https://www.steppeschool.com/pages/blog?p=stm32-printf-function-swv-stm32cubeide).

As long as you are using the same STM32 device (NUCLEO-H743ZI), then that should be everything you need to run SQLite on your device.

By default, SQLite will use the onboard Flash storage. **WARNING**: The embedded NOR Flash has a limited amount of erase-write cycles. Using it as the underlying storage for SQLite can be risky if you intend to write a lot of data.

To run the test just press the debug symbol 

---
---


## Running SQLite on other STM32 microcontrollers

There are a series of other configurations you should adjust to run SQLite on your STM32 device, especially if it is not on the supported devices list.

First, you should set the project to your own type of microcontroller. The easiest way to do so would be to create a new project, and then copy the code over from the files in this repository's `MRAM-sqlite-STM32` folder. 

Then, adjust the FLASH configurations in `MRAM-sqlite-STM32IDE/Core/Inc/definitions.h` to match your own MCU.
```C++
#define RESERVED_CODE_SECTORS 6                    //Number of flash sectors being used to store code. SQLite data cannot be stored in this sectors
#define FLASH_BANK_1_START_ADDRESS  0x08000000     //Start address of embedded FLASH's BANK 1
#define FLASH_BANK_2_START_ADDRESS  0x08100000     //Start address of embedded FLASH's BANK 2
//#define MRAM 1                                    //Set MRAM as the underlying storage device
```

In the file `MRAM-sqlite-STM32IDE/Core/Src/littlefs_sqlite_vfs.c` you can adjust LittleFS's settings, including the ones that are specific to your device's FLASH specifications, such as block_size, block count, read_size, prog_size, and so on:

```C++
//Config for embedded NOR FLASH
static const struct lfs_config lfs_cfg = {
    // block device operations
    .read  = lfs_read_flash,
    .prog  = lfs_prog_flash,
    .erase = lfs_erase_sector_flash,
    .sync  = lfs_sync_flash,

    // block device configuration
    .read_size = 4,
    .prog_size = 32,
    .block_size = 131072,
    .block_count = 16 - RESERVED_CODE_SECTORS,
    //.lookahead = 32
	.cache_size = 32,
    .lookahead_size = 32,
    .block_cycles = 500,
};
```

If you are having issues with LittleFS, you can try wiping the FLASH sectors that are being used for storing your SQLite data by enabling the following line in `MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c`. Don't forget to also check the FLASH erasing function, to see if it is compatible with your device's FLASH.

**WARNING: You will, of course, lose any data you may have if you do this!**

```C++
#ifdef STM32
// CHECK IF WIPE_FLASH() FUNCTION MATCHES YOUR DEVICE AND DOES NOT TRY TO ERASE SPACE OCCUPIED BY THE CODE.
void wipe_flash(){
	//FIrst six sectors are occupied with code
	erase_flash_sector(6, 1);
	erase_flash_sector(7, 1);
	for(int i = 0; i < 8; i++){
		erase_flash_sector(i, 2);
	}
}
#endif

int test_sqlite3(){
  sqlite3_stmt *res;
  int rc;
#ifdef STM32
  uint32_t szBuf = 500400;
  void * pBuf = malloc(szBuf);

  //mram_wipe();
  wipe_flash();  <------ ENABLE THIS CALL
[...]
}
```

Finally, you may want to adjust the runtime options passed to SQLite to match your device and to improve overall performance.

In `MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c`:

```C++
sqlite3 * sqlite_init(void * heap_buffer, uint32_t szBuf){
	sqlite3 *db;
	int rc;
#ifdef STM32
	//Using memsys5, zero-malloc mechanism. 475KBi heap
	//Trying to set cache on separate memory zone (RAM_D2), however I do not know if SQLITE_CONFIG_HEAP overrides this configuration

	//sqlite3_config(SQLITE_CONFIG_PAGECACHE,0x20000000,32,4000);
	sqlite3_config(SQLITE_CONFIG_HEAP, heap_buffer, szBuf, 2);
#endif
[...]
}
```
`SQLITE_CONFIG_HEAP` sets a buffer to be used by SQLite instead of using memory assigned by malloc. This is a mechanism made available by the memsys5 memory allocator ( more details [here](https://www.sqlite.org/malloc.html) ). 

Other options include the number of pages at which WAL checkpointing occurs, page size, among others. All run-time configurations can be consulted [here](https://www.sqlite.org/pragma.html). 

Default values can also be set during compilation of the amalgamation file. For the sqlite amalgamation used in the STM32, the default page size was set to 512 bytes, WAL checkpoints default to intervals of 100 pages and the maximum number of pages is set to 400 (we refer again to the detailed description on [how this amalgamation was built](./Amalgamation.md)).

### Running SQLite on an alternative storage medium (MRAM example)

Since onboard FLASH storage is often rather limited, external storage can be used instead. Here we give an example for how to run SQLite over MRAM. 
The code needed is already included in the `MRAM-sqlite-STM32IDE` project, needing only a few adjustments. We selected MRAM as an alternative for academic research purposes, but you can follow similar steps to use, for example, an external NAND Flash storage device.

First, you must set up your `.ioc` configuration so that you can interface your external storage device. We set our MRAM to be accessible through the FMC at 0xc0000000. Details on FMC usage and configuration can be found in the microcontroller's manual.

It is a good idea to also set your external device as write-through using the Memory Protection Unit (MPU). Since data cache is enabled in this project, the write-through configuration is required to ensure consistency of the database data in case of failure. To do so you must enable the highlighted call belo from the `MRAM-sqlite-STM32IDE/Core/Src/main.c` file, as well as make any necessary adjustments to the `MPU_RegionConfig()` function.

```C++
void MPU_RegionConfig()
{
	MPU_Region_InitTypeDef MPU_InitStruct = {0};

	HAL_MPU_Disable();

	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = MRAM_BANK_ADDR;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	MPU_RegionConfig(); < ----- ENABLE THIS CALL
[...]
}
```

The `MRAM-sqlite-STM32IDE/Core/Inc/mram_commons.h` file sets the configuration variables of the MRAM device.

```
#define MRAM_BANK_ADDR ((uint32_t)(0xc0000000))             // Start address for the external MRAM device
#define MRAM_BANK_ADDR_LIMIT_4Mb ((uint32_t) (0xc0080000))  // Max address for the external MRAM device
#define MRAM_NR_BYTES 524288                                // Capacity of the MRAM device, in bytes
```

Then, we must enable the use of MRAM specific code by setting the appropriate MACRO in `MRAM-sqlite-STM32IDE/Core/Inc/definitions.h`. 

```C++
#define RESERVED_CODE_SECTORS 6
#define FLASH_BANK_1_START_ADDRESS  0x08000000
#define FLASH_BANK_2_START_ADDRESS  0x08100000
#define MRAM 1                                  <---- ENABLE THIS
```


This will define a different set of functions to be used by the LittleFS driver in `MRAM-sqlite-STM32IDE/Core/Src/Drivers/littlefs_driver.c`. In order to provide support for a different device (e.g., external NAND FLASH), you must provide your own implementations for the following functions:

```C++
int lfs_read_flash(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);

int lfs_prog_flash(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, const void *buffer, lfs_size_t size);

int lfs_erase_sector_flash(const struct lfs_config *c, lfs_block_t block);

int lfs_sync_flash(const struct lfs_config *c);
```

It will also select a different configuration for LittleFS in the `MRAM-sqlite-STM32IDE/Core/Src/littlefs_sqlite_vfs.c` file, which you should also consider if implementing a different storage medium. 


## Known issues and recommendations

If you want to do more complicated operations than simply storing a couple of records in SQLite, you will likely run into problems with memory limitations. We recommend trying to use external memory (e.g. external SRAM) and using the SQLITE_CONFIG_HEAP configuration to set the external device as SQLite's heap. 

To improve performance, go to project settings, adjust the compiler's optimization level and disable debugging. We have observed a significant impact on performance from using the -O3 level of optimization.






