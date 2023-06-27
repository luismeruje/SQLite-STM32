/*
 * bench_sqlite3.c
 *
 *  Created on: Jun 20, 2023
 *      Author: luisferreira
 */

#include "bench_sqlite3.h"
#include <stdio.h>
#include "sqlite3.h"
#include "lfs.h"
#include "stm32h7xx_hal.h"
#include "flash_driver.h"
#include "littlefs_sqlite_vfs.h"
#include "MRAM_driver.h"

#define INSERTS_PER_TRANSACTION 2
//
#define NR_TRANSACTIONS 2500
//Individual query max size
#define TRANSACTION_STRING_SIZE (60 + (20*INSERTS_PER_TRANSACTION))


sqlite3 * sqlite_init(void * heap_buffer, uint32_t szBuf){
	//Using memsys5, zero-malloc mechanism. 475KBi heap

	 //Trying to set cache on separate memory zone (RAM_D2), however I do not know if SQLITE_CONFIG_HEAP overrides this configuration
	 sqlite3_config(SQLITE_CONFIG_PAGECACHE,0x20000000,32,4000);
	 sqlite3_config(SQLITE_CONFIG_HEAP, heap_buffer, szBuf, 2);
	 sqlite3_config(SQLITE_CONFIG_MMAP_SIZE,0,0);

	 //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);

	  //printf("Previous sqlite heap limit: %lu\n", sqlite3_hard_heap_limit64(450000)); //By setting the limit, SQLite will warn that it is out of memory, instead of causing a hardfault
	  sqlite3 *db;
	  sqlite3_open_v2("helloworld7", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "littlefs");
	  sqlite3_exec(db, "PRAGMA journal_mode=WAL", 0, 0, 0);

	  return db;
}

void create_table(sqlite3 *db){
	int rc;
	char *err_msg = 0;
	char * createTable = "CREATE TABLE Sensor(Timestamp INT, Device INT, Zone INT, Pressure INT);";
	rc = sqlite3_exec(db, createTable, 0, 0, &err_msg);

	  if (rc != SQLITE_OK ) {
		  fprintf(stderr, "SQL error: %s\n", err_msg);

		  sqlite3_free(err_msg);
		  sqlite3_close(db);

		  exit(-1);
	  } else{
		  printf("Good create\n");
	  }

}

void drop_table(sqlite3 *db){
	int rc;
	char *err_msg = 0;
	char * dropTableIfExists = "DROP TABLE IF EXISTS Sensor;";
	rc = sqlite3_exec(db, dropTableIfExists, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
        printf( "SQL error: %s\n", err_msg);
		printf("Msg: %s\n", sqlite3_errmsg(db));

		sqlite3_free(err_msg);
		sqlite3_close(db);

		exit(-1);
	} else{
		printf("Good drop\n");
	}
}

void print_sqlite_version(sqlite3 * db){
	sqlite3_stmt *res;
	int rc;
	rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);

	if (rc != SQLITE_OK) {

		printf("Failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		exit(-1);
	}

	rc = sqlite3_step(res);

	if (rc == SQLITE_ROW) {
		printf("SQLite version: %s\n", sqlite3_column_text(res, 0));
	}
}

void prepare_write_queries(char (* sqlQueries)[TRANSACTION_STRING_SIZE], uint32_t * value, uint32_t nrQueries){
	char aux[60];

	for(int t = 0; t < nrQueries; t++){
	  //Create queries
	  sprintf(sqlQueries[t], "INSERT INTO Sensor VALUES ");

	  for (int i = 0; i < INSERTS_PER_TRANSACTION - 1; i++, *value += 1){
		  sprintf(aux, "(%lu,%d,%d,%d),",*value, rand()%255,rand()%255,rand()%255);
		  strcat(sqlQueries[t], aux);
	  }
	  sprintf(aux, "(%lu,%d,%d,%d);",*value,rand()%255,rand()%255,rand()%255);
	  *value += 1;
	  strcat(sqlQueries[t], aux);
	}

	//printf("Example query: %s\n", sqlQueries[1]);
}

void prepare_read_queries(char (* sqlQueries)[TRANSACTION_STRING_SIZE], uint32_t numQueries, uint32_t queryOffset){

	for(int t = 0; t < numQueries; t++){
	  //Create queries
	  sprintf(sqlQueries[t], "SELECT Pressure from Sensor WHERE %lu <= Timestamp < %lu;", INSERTS_PER_TRANSACTION * (queryOffset+t), INSERTS_PER_TRANSACTION * (1+queryOffset+t));
	}

	//printf("Example query: %s\n", sqlQueries[1]);
}

void measure_basic_throughput(sqlite3 *db){
	char (* sqlQueries)[TRANSACTION_STRING_SIZE] = (void *) 0x38000000;


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
			"Number transactions per batch: %lu\n"
			"Nr batches: %lu\nLeftoverBatch? %s\n"
			"Nr transactions leftover batch: %lu\n",
				INSERTS_PER_TRANSACTION,
				NR_TRANSACTIONS,
				nrTransactionsPerBatch,
				nrBatches,
				leftOverBatch ? "Yes" : "No",
				NR_TRANSACTIONS % nrTransactionsPerBatch);

	//Execute and time the insertions

	int startTime = 0;
	uint32_t elapsedTime = 0;
	for(int batch = 0; batch < nrBatches; batch++ ){
		prepare_write_queries(sqlQueries,&counter, nrTransactionsPerBatch);
		startTime = HAL_GetTick();
		for(int t = 0; t < nrTransactionsPerBatch; t++){
			rc = sqlite3_exec(db, sqlQueries[t], 0, 0, &err_msg);
			if (rc != SQLITE_OK ) {
			  printf("SQL error: %s\n", err_msg);

			  sqlite3_free(err_msg);
			  sqlite3_close(db);

			  exit(-1);
			}
		}
		elapsedTime += HAL_GetTick() - startTime;
	}

	if(leftOverBatch){
		prepare_write_queries(sqlQueries,&counter, nrTransactionsLeftoverBatch);
		startTime = HAL_GetTick();
		for(int t = 0; t < nrTransactionsLeftoverBatch; t++){
			rc = sqlite3_exec(db, sqlQueries[t], 0, 0, &err_msg);
			if (rc != SQLITE_OK ) {
			  printf("SQL error: %s\n", err_msg);

			  sqlite3_free(err_msg);
			  sqlite3_close(db);

			  exit(-1);
			}
		}
		elapsedTime += HAL_GetTick() - startTime;
	}

	printf("Took %lu ms to insert %d records \n", elapsedTime,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS);

	//Read evaluation
	sqlite3_stmt *res;
	elapsedTime = 0;
	int batch = 0;
	for(batch = 0; batch < nrBatches; batch++ ){
		prepare_read_queries(sqlQueries, nrTransactionsPerBatch,  nrTransactionsPerBatch*batch);
		startTime = HAL_GetTick();
		for(int t = 0; t < nrTransactionsPerBatch; t++){
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
		elapsedTime += HAL_GetTick() - startTime;
	}


	if(leftOverBatch){
		prepare_read_queries(sqlQueries, nrTransactionsLeftoverBatch, batch*nrTransactionsPerBatch);
		startTime = HAL_GetTick();
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
		elapsedTime += HAL_GetTick() - startTime;
	}

	printf("Took %lu ms to read %d records \n", elapsedTime,INSERTS_PER_TRANSACTION * NR_TRANSACTIONS);
}


void wipe_flash(){
	//FIrst six sectors are occupied with code
	erase_flash_sector(6, 1);
	erase_flash_sector(7, 1);
	for(int i = 0; i < 8; i++){
		erase_flash_sector(i, 2);
	}
}

int test_sqlite3(){
  sqlite3_stmt *res;
  int rc;
  uint32_t szBuf = 500400;
  void * pBuf = malloc(szBuf);
  mram_wipe();
  //wipe_flash();

  littlefs_vfs_init("helloworld7", 0);

  sqlite3 * db = sqlite_init(pBuf, szBuf);
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
  littlefs_vfs_ls("/");

  sqlite3_finalize(res);
  drop_table(db);
  sqlite3_close(db);
   //Check which files remain
  littlefs_vfs_ls("/");
  littlefs_vfs_close();
  //free(pBuf);
  return 0;
}
