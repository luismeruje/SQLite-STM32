# SQLite amalgamation for use with STM32

The SQLite amalgamation is an aggregation of all SQLite code in a single file, so that it can be easily included by an application. The steps needed to build this amalgamation are given by SQLite itself [here](https://www.sqlite.org/amalgamation.html). Make sure you can successfully build the amalgamation files (sqlite3.c and sqlite3.h) before proceeding.

SQLite provides a series of compilation options such has determining database behavior, omitting certain components of the database, setting default configurations and defining the target OS. All options and corresponding descriptions can be found [here](https://www.sqlite.org/compile.html).

For our sqlite3 amalgamation, the following options are used:

 * SQLITE_ENABLE_MEMSYS5 -> allows for zero-malloc memory. 
 * SQLITE_THREADSAFE=0 -> single-thread execution
 * SQLITE_OS_OTHER=1 -> do not include default OS layers (i.e., VFSs)
 * SQLITE_DEFAULT_PAGE_SIZE=512 -> power 2 between 512 and 65536. Default is 4096
 * SQLITE_DEFAULT_CACHE_SIZE=100 -> Will give you 100*1024bytes of cache. Default value is 2000.
 * SQLITE_DEFAULT_WAL_AUTOCHECKPOINT=100 -> Number of pages before checkpoint (flushing wal). Default is 1000. Allows you to limit maximum wal file size, very important!
 * SQLITE_MAX_PAGE_COUNT=400 -> 450*512 bytes ≈ 200KB of storage. You should increase/decrease this value according to the amount of available space in your device.
 * SQLITE_OMIT_ALTERTABLE
 * SQLITE_OMIT_ANALYZE 
 * SQLITE_OMIT_ATTACH
 * SQLITE_OMIT_AUTHORIZATION 
 * SQLITE_OMIT_AUTOINCREMENT 
 * SQLITE_OMIT_SHARED_CACHE

The *memsys5* memory allocator allows you to pass SQLite a buffer that it will use in place of malloc allocations. Not only does this allow you to set a maximum size for used memory, but it also allows you to set the buffer in memory regions which are not being used and where heap is not implemented. For example, in the STM32H743ZI MCU there are 3 different regions of memory, and only one of them is used for heap and stack memory, leaving the remaining regions unused. 

The *DEFAULT_PAGE_SIZE*, *DEFAULT_CACHE_SIZE*, *DEFAULT_WAL_AUTOCHECKPOINT*  and MAX_PAGE_COUNT let you control how much resources SQLite consumes. They also have an impact in database performance.

The *OS_OTHER* option disables the default OS layers, which do not compile on STM32 devices. You must implement your own OS layer. For more information see the [README](./README.md) and also check SQLite's [documentation](https://www.sqlite.org/vfs.html).

The *OMIT* clauses reduce the final size of the code, however they are not fully supported, so using them may result in code with errors which you must fix. Feel free to try adding/removing different *OMIT* clauses and see how it affects final code size and performance. A full list of *OMIT* clauses and their effects can be found [here](https://www.sqlite.org/compile.html). 

## Building the amalgamation
Below are the steps needed to build the amalgamation file.

### 1. Download the files and generate Makefile

After downloading the source files for SQLite and running `./configure`, you should have a structure similar to this:

```
sqlite-src-3410200
├── LICENSE.md
├── Makefile
├── Makefile.in
├── Makefile.msc
├── README.md
├── ...
```

### 2. Adding compile-time option to the Makefile

Edit the Makefile

```
vim Makefile
```

Find the thread safe option and set it to zero

```bash
# Should the database engine be compiled threadsafe
#
TCC += -DSQLITE_THREADSAFE=0
```

Then set the `OPTS` variable

```bash
# Add in any optional parameters specified on the make commane line
# ie.  make "OPTS=-DSQLITE_ENABLE_FOO=1 -DSQLITE_OMIT_FOO=1".
OPTS = -DSQLITE_ENABLE_MEMSYS5 -DSQLITE_OS_OTHER=1 -DSQLITE_DEFAULT_WAL_AUTOCHECKPOINT=100 -DSQLITE_DEFAULT_PAGE_SIZE=512 -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_DEFAULT_MMAP_SIZE=0
OPTS += -DSQLITE_MAX_PAGE_COUNT=400 -DSQLITE_OMIT_ALTERTABLE -DSQLITE_OMIT_ANALYZE -DSQLITE_OMIT_ATTACH -DSQLITE_OMIT_AUTHORIZATION -DSQLITE_OMIT_AUTOINCREMENT -DSQLITE_OMIT_SHARED_CACHE
TCC += $(OPTS)
```

### 3. Build the amalgamation file
 Build the amalgamation by running:

```
make
```

You should now have a `sqlite3.c` and a `sqlite3.h` file.

### 4. Adding the compile-time options to the generated amalgamation

Although the compile-time options are used during the compilation process, they are also needed in the amalgamation file in order to avoid certain errors. As such, we must add the following MACROS to the top of the `sqlite3.c` file:

```C++
#define SQLITE_THREADSAFE 0
#define SQLITE_OS_OTHER 1
#define SQLITE_ENABLE_MEMSYS5 1
#define SQLITE_DEFAULT_WAL_AUTOCHECKPOINT 100
#define SQLITE_DEFAULT_PAGE_SIZE 512
#define SQLITE_MAX_PAGE_COUNT 400
#define SQLITE_OMIT_ALTERTABLE 1
#define SQLITE_OMIT_ANALYZE 1
#define SQLITE_OMIT_ATTACH 1
#define SQLITE_OMIT_AUTHORIZATION 1
#define SQLITE_OMIT_AUTOINCREMENT 1
#define SQLITE_OMIT_SHARED_CACHE 1
#define SQLITE_MAX_MMAP_SIZE 0
#define SQLITE_DEFAULT_MMAP_SIZE 0
```

### 5. Set OS init, end,and randomness functions

When the default OS layers are disabled (`SQLITE_OS_OTHER=1`), the default OS initialization and termination functions are omitted, so we must implement our own functions, which should register a custom OS layer. For example:

```C++
#include "littlefs_sqlite_vfs.h"
SQLITE_API int sqlite3_os_init(){

   return sqlite3_vfs_register(get_littlefs_vfs(), 1);
}

SQLITE_API int sqlite3_os_end(){
        return SQLITE_OK;
}
```

Also, the randomness function recurs to *mutex* function which are not available to the STM32. As such, we recommend **commenting out the default implementation of the** `sqlite3_randomness()` function. We provide a simple substitute below, however it would be a good idea to try to adapt the original function avoiding the use of *mutex* operations.

```C++
SQLITE_API void sqlite3_randomness(int N, void *pBuf){
	for(int i = 0; i < N; i++){
			((char*)pBuf)[i] = (char) (rand() % 255);
	}
}
```

### 6. Set wal mode to use heap memory

Shared memory primitives are not available in STM32 devices, so we must force wal mode to use heap memory. We can do so but modifying the `sqlite3WalOpen` function:

```C++ #13
SQLITE_PRIVATE int sqlite3WalOpen(
  sqlite3_vfs *pVfs,              /* vfs module to open wal and wal-index */
  sqlite3_file *pDbFd,            /* The open database file */
  const char *zWalName,           /* Name of the WAL file */
  int bNoShm,                     /* True to run in heap-memory mode */
  i64 mxWalSize,                  /* Truncate WAL to this size on reset */
  Wal **ppWal                     /* OUT: Allocated Wal handle */
){
    ...
  pRet->zWalName = zWalName;
  pRet->syncHeader = 1;
  pRet->padToSectorBoundary = 1;
  pRet->exclusiveMode = WAL_HEAPMEMORY_MODE;// (bNoShm ? WAL_HEAPMEMORY_MODE: WAL_NORMAL_MODE); <-- MODIFICATION HERE

}
```

### 7. Test it out (some errors may have to be corrected)

The only thing left is to add the `sqlite3.c` and `sqlite3.h` files to your application and try them out.

The OMIT compile-time options are not guaranteed to work, so there may be some errors to correct. In the version we tested, a single error was corrected where a function is incorrectly declared inside an `ifdef` block.

Also, do not forget about any runtime options you may want to use. For example, to use the memsys5 memory allocator you must enable it during compilation (which we have already done), but also set the correct run-time options to use it. Below is a simple example of a program you can use to test that SQLite is working:

```C++

void check_error(int rc, sqlite3 * db){
	if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        exit(-1);
    }
}

int test_sqlite3(){
  sqlite3 *db;
  sqlite3_stmt *res;
  int rc;

  uint32_t szBuf = 500400;
  void * pBuf = malloc(szBuf);

  //Initialize underlying 

  //Enable memsys5 zero-malloc memory allocator
  sqlite3_config(SQLITE_CONFIG_HEAP, heap_buffer, szBuf, 2);
  
  sqlite3_open_v2("helloworld", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "littlefs");

  rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL", 0, 0, 0);
  
}
```



