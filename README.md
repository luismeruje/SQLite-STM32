# SQLite port to STM32 microcontrollers

### Compatibility
This port has only been tested in the microconotrollers described below. If you are able to successfully run STM32 in a different microcontroller, please let me know so I can add it to the list.

STM32H743ZI

## Testing SQLite on a UNIX system

Before attempting to run SQLite on the STM32 it is a good idea to test it in a normal UNIX environment to check that it runs as expected.

First, make sure the benchmark is correctly configured, by setting the appropriate MACROS at the beginning of the  `bench_sqlite3.c.temp` file.

```
#define OPERATIONS_PER_TRANSACTION 2   //Number of operations per transaction (e.g, 2 inserts per transaction in this case)
#define NR_TRANSACTIONS 2500           //Total number of executed transactions
// #define STM32 1                  //Leave commented, unless you are running test on STM32
```

Then copy the file to the standalone folder, compile and run. 

The standalone folder contains the standard amalgamation of sqlite (i.e., sqlite3.c), which was not modified to run on STM32. For more details on the sqlite amalagamation check [this page]([https://www.google.com](https://www.sqlite.org/amalgamation.html)https://www.sqlite.org/amalgamation.html).

```
cp bench_sqlite3.c.temp SQLite-standalone/src/bench_sqlite3.c
cp bench_sqlite3.h.temp SQLite-standalone/inc/bench_sqlite3.h
cd SQLite-standalone
make
./sqlite3_bench
```
For a successful test, the following (or a similar) output is expected:
```
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

Notice that a series of `helloworld*` files are created in the standalone folder during the test. These are the SQLite files.

## Testing SQLite on STM32
The `MRAM-sqlite-STM32` folder contains an STM32IDE project to run SQLite on STM32 devices. The project is set for a NUCLEO-H743ZI board.
Inside there is an SQLite amalgamation, more specifically at `MRAM-sqlite-STM32IDE/Core/Src/sqlite3.c`, which is configured specifically to run sqlite on the STM32.
Some options include setting single thread execution, enabling the memsys5 memory manager, and omitting components to lower resource consumption.
For a detailed description of how this amalgamation was created click here [TODO].

Furthermore, it also includes LittleFS, which is used as the underlying filesystem for SQLite.

Let's start by enabling the STM32 mode at the beginning of the  `bench_sqlite3.c.temp` file.

```
#define OPERATIONS_PER_TRANSACTION 2   //Number of operations per transaction (e.g, 2 inserts per transaction in this case)
#define NR_TRANSACTIONS 2500           //Total number of executed transactions
#define STM32 1                  //Leave commented, unless you are running test on STM32
```

Then copy the benchmark files to the appropriate folder:
```
cp bench_sqlite3.c.temp MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c
cp bench_sqlite3.h.temp MRAM-sqlite-STM32IDE/Core/Inc/bench_sqlite3.h
```

Finally, import the project `MRAM-sqlite-STM32` to your STM32IDE.

There are a series of other configurations you should adjust to run SQLite on your STM32 device, especially if it is not on the supported devices list. Below is an example considering the onboard FLASH as the storage device.
**WARNING**: The embedded NOR Flash has a limited amount of erase-write cycles. Using it as the underlying storage for SQLite can be risky if you intend to write a lot of data.

Adjust your FLASH configurations `MRAM-sqlite-STM32IDE/Core/Inc/definitions.h` and make sure that MRAM is not enabled.
```
#define RESERVED_CODE_SECTORS 6                    //Number of flash sectors being used to store code. SQLite data cannot be stored in this sectors
#define FLASH_BANK_1_START_ADDRESS  0x08000000     //Start address of embedded FLASH's BANK 1
#define FLASH_BANK_2_START_ADDRESS  0x08100000     //Start address of embedded FLASH's BANK 2
//#define MRAM 1                                    //Set MRAM as the underlying storage device
```

In the file `MRAM-sqlite-STM32IDE/Core/Src/littlefs_sqlite_vfs.c` you can adjust LittleFS's settings, such as cache size:

```
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

If you are having issues with LittleFS, you can try wiping the FLASH sectors that are being used for storing your SQLite data by enabling the following line in `MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c`. Don't forget to also check the FLASH erasing function, to see if it is compatible with your device.
WARNING: You will, of course, lose any data you may have if you do this! And do not forget that the embedded FLASH has a limited number of erase-write cycles.

```
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
  //wipe_flash();  <------ ENABLE THIS CALL
[...]
}
```

Other files to take into account when configuring your device, especially if you are trying to adapt this project to a different STM32 MCU: `MRAM-sqlite-STM32IDE/Core/Drivers/flash_driver.c`.

Finally, you may want to adjust the runtime options passed to SQLite to match your device and to improve overall performance.

In `MRAM-sqlite-STM32IDE/Core/Src/Benchmark/bench_sqlite3.c`:

```
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
`SQLITE_CONFIG_HEAP` sets a buffer to be used by SQLite instead of using memory assigned by malloc. This is a mechanism made available by the memsys5 memory allocator (see [TODO:link]). 
Other options include the number of pages at which WAL checkpointing occurs, page size, among others. See [TODO:link]. 
Default values can also be set during compilation of the amalgamation file. For the sqlite amalgamation used in the STM32 the default page size was set to 512 bytes, WAL checkpoints default to intervals of 100 pages and the maximum number of pages is set to 400 (we refer again to the detailed description on how this amalgamation was built [TODO:link]).

### Running SQLite on an alternative storage medium (MRAM example)

Since onboard FLASH storage is often rather limited, external storage can be used instead. Here we give an example for how to run SQLite over MRAM. 
The code needed is already included in the `MRAM-sqlite-STM32IDE` project, needing only a few adjustments. We selected MRAM as an alternative for academic research purposes, but you can follow similar steps to use, for example, an external NAND Flash storage device.



## Known issues and recommendations








