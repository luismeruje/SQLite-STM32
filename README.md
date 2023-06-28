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

##Testing SQLite on STM32
