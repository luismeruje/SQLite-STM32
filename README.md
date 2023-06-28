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
cp  bench_sqlite3.c.temp SQLite-standalone/src/bench_sqlite3.c
cp  bench_sqlite3.h.temp SQLite-standalone/src/bench_sqlite3.h
cd SQLite-standalone
make
./sqlite3_bench
```

##Testing SQLite on STM32
