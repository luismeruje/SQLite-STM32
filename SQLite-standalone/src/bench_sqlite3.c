/*
 * bench_sqlite3.c
 *
 *  Created on: Jun 20, 2023
 *      Author: luisferreira
 */

#include "bench_sqlite3.h"
#include <stdio.h>
#include "sqlite3.h"


#ifdef MRAMEN
#include "stm32h7xx_hal.h"
#include "MRAM_driver.h"
#include "lfs.h"
#include "flash_driver.h"
#include "littlefs_sqlite_vfs.h"

#else
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define BILLION  1000000000.0
#endif

//========== SETTINGS ===========
#define INSERTS_PER_TRANSACTION 2
#define NR_TRANSACTIONS 2500
//===============================

//Individual query max size
#define TRANSACTION_STRING_SIZE (60 + (20*INSERTS_PER_TRANSACTION))

void check_error(int rc, sqlite3 * db){
	if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        exit(-1);
    }
}

sqlite3 * sqlite_init(void * heap_buffer, uint32_t szBuf){
	sqlite3 *db;
	int rc;
#ifdef MRAMEN
	//Using memsys5, zero-malloc mechanism. 475KBi heap
	//Trying to set cache on separate memory zone (RAM_D2), however I do not know if SQLITE_CONFIG_HEAP overrides this configuration

	sqlite3_config(SQLITE_CONFIG_PAGECACHE,0x20000000,32,4000);
	sqlite3_config(SQLITE_CONFIG_HEAP, heap_buffer, szBuf, 2);
	sqlite3_config(SQLITE_CONFIG_MMAP_SIZE,0,0);
#endif

#ifdef MRAMEN
	sqlite3_open_v2("helloworld7", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "littlefs");
#else
	rc =  sqlite3_open("helloworld", &db);
	check_error(rc,db);
	#endif

	rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL", 0, 0, 0);
	check_error(rc,db);

	return db;
}

void create_table(sqlite3 *db){
	int rc;
	char *err_msg = 0;
	char * createTable = "CREATE TABLE Sensor(Timestamp INT, Device INT, Zone INT, Pressure INT);";
	rc = sqlite3_exec(db, createTable, 0, 0, &err_msg);

	check_error(rc,db);
	printf("Good create\n");
	
}

void drop_table(sqlite3 *db){
	int rc;
	char *err_msg = 0;
	char * dropTableIfExists = "DROP TABLE IF EXISTS Sensor;";
	rc = sqlite3_exec(db, dropTableIfExists, 0, 0, &err_msg);
	check_error(rc,db);
	printf("Good drop\n");
	
}

void print_sqlite_version(sqlite3 * db){
	sqlite3_stmt *res;
	int rc;
	rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);
	check_error(rc,db);

	rc = sqlite3_step(res);

	if (rc == SQLITE_ROW) {
		printf("SQLite version: %s\n", sqlite3_column_text(res, 0));
	} else {
		check_error(rc,db);
	}
}

void prepare_write_queries(char (* sqlQueries)[TRANSACTION_STRING_SIZE], uint32_t * value, uint32_t nrQueries){
	char aux[60];

	for(int t = 0; t < nrQueries; t++){
	  //Create queries
	  sprintf(sqlQueries[t], "INSERT INTO Sensor VALUES ");

	  for (int i = 0; i < INSERTS_PER_TRANSACTION - 1; i++, *value += 1){
		  sprintf(aux, "(%u,%d,%d,%d),",*value, rand()%255,rand()%255,rand()%255);
		  strcat(sqlQueries[t], aux);
	  }
	  sprintf(aux, "(%u,%d,%d,%d);",*value,rand()%255,rand()%255,rand()%255);
	  *value += 1;
	  strcat(sqlQueries[t], aux);
	}

	//printf("Example query: %s\n", sqlQueries[1]);
}

void prepare_read_queries(char (* sqlQueries)[TRANSACTION_STRING_SIZE], uint32_t numQueries, uint32_t queryOffset){

	for(int t = 0; t < numQueries; t++){
	  //Create queries
	  sprintf(sqlQueries[t], "SELECT Pressure from Sensor WHERE %u <= Timestamp < %u;", INSERTS_PER_TRANSACTION * (queryOffset+t), INSERTS_PER_TRANSACTION * (1+queryOffset+t));
	}

	//printf("Example query: %s\n", sqlQueries[1]);
}

void measure_basic_throughput(sqlite3 *db){
	#ifdef MRAMEN
	char (* sqlQueries)[TRANSACTION_STRING_SIZE] = (void *) 0x38000000;
	#else
	char (* sqlQueries)[TRANSACTION_STRING_SIZE] = malloc(128000);
	#endif


	char *err_msg = 0;
	int rc;

	//Prepare queries
	uint32_t counter = 0;
	int leftOverBatch = 0;
	//Memory zone where array is located only supports 64KBi. Using 60KBi
	uint32_t nrTransactionsPerBatch = 61440/(TRANSACTION_STRING_SIZE);
	uint32_t nrBatches = NR_TRANSACTIONS / nrTransactionsPerBatch;
	uint32_t nrTransactionsLeftoverBatch = 0;
	if(NR_TRANSACTIONS % nrTransactionsPerBatch){
		leftOverBatch=1;
		nrTransactionsLeftoverBatch = NR_TRANSACTIONS % nrTransactionsPerBatch;
	}

	printf("Test setup\n"
			"Inserts per transaction: %d\n"
			"Nr transactions: %d\n"
			"Number transactions per batch: %u\n"
			"Nr batches: %u\nLeftoverBatch? %s\n"
			"Nr transactions leftover batch: %u\n",
				INSERTS_PER_TRANSACTION,
				NR_TRANSACTIONS,
				nrTransactionsPerBatch,
				nrBatches,
				leftOverBatch ? "Yes" : "No",
				NR_TRANSACTIONS % nrTransactionsPerBatch);

//========= INSERTION THROUGHPUT ===========

#ifdef MRAMEN
	int startTime = 0, elapsedTime = 0;
#else
	double timeSpent = 0;
	struct timespec start, end;
#endif

	for(int batch = 0; batch < nrBatches; batch++ ){
		prepare_write_queries(sqlQueries,&counter, nrTransactionsPerBatch);
#ifdef MRAMEN
    	startTime = HAL_GetTick();
#else
    	clock_gettime(CLOCK_REALTIME, &start);
#endif
		for(int t = 0; t < nrTransactionsPerBatch; t++){
			rc = sqlite3_exec(db, sqlQueries[t], 0, 0, &err_msg);
			check_error(rc,db);
		}
#ifdef MRAMEN
    elapsedTime += HAL_GetTick() - startTime;
#else
	clock_gettime(CLOCK_REALTIME, &end);
	timeSpent += (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
#endif
	}

	if(leftOverBatch){
		prepare_write_queries(sqlQueries,&counter, nrTransactionsLeftoverBatch);
#ifdef MRAMEN
    	startTime = HAL_GetTick();
#else
    	clock_gettime(CLOCK_REALTIME, &start);
#endif
		for(int t = 0; t < nrTransactionsLeftoverBatch; t++){
			rc = sqlite3_exec(db, sqlQueries[t], 0, 0, &err_msg);
			check_error(rc,db);
		}
#ifdef MRAMEN
    elapsedTime += HAL_GetTick() - startTime;
	printf("Took %u ms to insert %d records \n", elapsedTime,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS)
#else
	clock_gettime(CLOCK_REALTIME, &end);
	timeSpent += (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
	printf("Took %lf ms to insert %d records \n", timeSpent,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS);
#endif
	}

//========= READ THROUGHPUT ===========
	sqlite3_stmt *res;
#ifdef MRAMEN
	elapsedTime = 0;
#else
	timeSpent = 0;
#endif
	int batch = 0;
	for(batch = 0; batch < nrBatches; batch++ ){
		prepare_read_queries(sqlQueries, nrTransactionsPerBatch,  nrTransactionsPerBatch*batch);
#ifdef MRAMEN
    	startTime = HAL_GetTick();
#else
    	clock_gettime(CLOCK_REALTIME, &start);
#endif
		for(int t = 0; t < nrTransactionsPerBatch; t++){
//			printf("%s\n",sqlQueries[t]);
			rc = sqlite3_prepare_v2(db, sqlQueries[t], -1, &res, 0);
			check_error(rc,db);
			while((rc = sqlite3_step(res)) == SQLITE_ROW){}
			sqlite3_finalize(res);
			
		}
#if MRAMEN
    	elapsedTime += HAL_GetTick() - startTime;
#else
		clock_gettime(CLOCK_REALTIME, &end);
		timeSpent += (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
#endif
	}


	if(leftOverBatch){
		prepare_read_queries(sqlQueries, nrTransactionsLeftoverBatch, batch*nrTransactionsPerBatch);
#ifdef MRAMEN
    	startTime = HAL_GetTick();
#else
    	clock_gettime(CLOCK_REALTIME, &start);
#endif
		for(int t = 0; t < nrTransactionsLeftoverBatch; t++){
//			printf("%s\n",sqlQueries[t]);
			rc = sqlite3_prepare_v2(db, sqlQueries[t], -1, &res, 0);
			if (rc != SQLITE_OK ) {
			  printf("SQL error: %s\n", err_msg);

			  sqlite3_free(err_msg);
			  sqlite3_close(db);

			  exit(-1);
			} else{
				while((rc = sqlite3_step(res)) == SQLITE_ROW){

				}
				sqlite3_finalize(res);
			}
		}
#ifdef MRAMEN
    elapsedTime += HAL_GetTick() - startTime;
	printf("Took %u ms to read %d records \n", elapsedTime,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS)
#else
	clock_gettime(CLOCK_REALTIME, &end);
	timeSpent += (end.tv_sec - start.tv_sec) +
                                (end.tv_nsec - start.tv_nsec) / BILLION;
	printf("Took %lf ms to read %d records \n", timeSpent,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS);
#endif
	}

}

#ifdef MRAMEN
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
#ifdef MRAMEN
  uint32_t szBuf = 500400;
  void * pBuf = malloc(szBuf);

  mram_wipe();
  //wipe_flash();

  littlefs_vfs_init("helloworld7", 0);

  sqlite3 * db = sqlite_init(pBuf, szBuf);
  #endif
 sqlite3 * db = sqlite_init(NULL, 0);
  print_sqlite_version(db);
  create_table(db);

  measure_basic_throughput(db);





  //printf("%s\n", sqlQueries);
  char * select = "SELECT count(*) FROM Sensor;";
  rc = sqlite3_prepare_v2(db, select, -1, &res, 0);
  if (rc != SQLITE_OK ) {

	  fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
	  sqlite3_close(db);

	  return 1;
  } else{
	  rc = sqlite3_step(res);

	  if (rc == SQLITE_ROW) {
		printf("%s\n", sqlite3_column_text(res, 0));
	  } else{

		  printf("Rc error: %d\n",rc);

	  }
  }

  sqlite3_finalize(res);
  drop_table(db);
  sqlite3_close(db);
   //Check which files remain
#ifdef MRAMEN
  littlefs_vfs_ls("/");
  littlefs_vfs_close();
#endif
  //free(pBuf);
  return 0;
}

int main(int argc, char ** argv){
	test_sqlite3();
}