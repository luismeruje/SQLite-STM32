#include "lfs.h"
#include "littlefs_sqlite_vfs.h"
#include "stm32h7xx_hal.h"

#define MUTE_SHARED_MEMORY_WARNINGS

//Subclass of sqlite3_file for littlefs

typedef struct sqlite3_littlefs_file{
	__IO const struct sqlite3_io_methods *pMethods;
	__IO lfs_file_t file;
} sqlite3_littlefs_file;


//Subclass of sqlite3_vfs
//typedef struct sqlite3_littlefs_vfs{
//	int iVersion;            /* Structure version number (currently 3) */
//	  int szOsFile;            /* Size of subclassed sqlite3_file */
//	  int mxPathname;          /* Maximum file pathname length */
//	  sqlite3_vfs *pNext;      /* Next registered VFS */
//	  const char *zName;       /* Name of this virtual file system */
//	  void *pAppData;          /* Pointer to application-specific data */
//	  int (*xOpen)(sqlite3_vfs*, sqlite3_filename zName, sqlite3_file*,
//	               int flags, int *pOutFlags);
//	  int (*xDelete)(sqlite3_vfs*, const char *zName, int syncDir);
//	  int (*xAccess)(sqlite3_vfs*, const char *zName, int flags, int *pResOut);
//	  int (*xFullPathname)(sqlite3_vfs*, const char *zName, int nOut, char *zOut);
//	  void *(*xDlOpen)(sqlite3_vfs*, const char *zFilename);
//	  void (*xDlError)(sqlite3_vfs*, int nByte, char *zErrMsg);
//	  void (*(*xDlSym)(sqlite3_vfs*,void*, const char *zSymbol))(void);
//	  void (*xDlClose)(sqlite3_vfs*, void*);
//	  int (*xRandomness)(sqlite3_vfs*, int nByte, char *zOut);
//	  int (*xSleep)(sqlite3_vfs*, int microseconds);
//	  int (*xCurrentTime)(sqlite3_vfs*, double*);
//	  int (*xGetLastError)(sqlite3_vfs*, int, char *);
//	  /*
//	  ** The methods above are in version 1 of the sqlite_vfs object
//	  ** definition.  Those that follow are added in version 2 or later
//	  */
//	  int (*xCurrentTimeInt64)(sqlite3_vfs*, sqlite3_int64*);
//	  /*
//	  ** The methods above are in versions 1 and 2 of the sqlite_vfs object.
//	  ** Those below are for version 3 and greater.
//	  */
//	  int (*xSetSystemCall)(sqlite3_vfs*, const char *zName, sqlite3_syscall_ptr);
//	  sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char *zName);
//	  const char *(*xNextSystemCall)(sqlite3_vfs*, const char *zName);
//	  /*
//	  ** The methods above are in versions 1 through 3 of the sqlite_vfs object.
//	  ** New fields may be appended in future versions.  The iVersion
//	  ** value will increment whenever this happens.
//	  */
//
//	  const struct lfs_config * cfg;
//	  lfs_t * lfs;
//}sqlite3_littlefs_vfs;

#include "definitions.h"
#include "littlefs_driver.h"

/*
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
};*/

__IO static const struct lfs_config lfs_cfg = {
    // block device operations
    .read  = lfs_read_flash,
    .prog  = lfs_prog_flash,
    .erase = lfs_erase_sector_flash,
    .sync  = lfs_sync_flash,

    // block device configuration
    .read_size = 4,
    .prog_size = 4,
    .block_size = 128,
    .block_count = 4096,
    //.lookahead = 32
	.cache_size = 64,
    .lookahead_size = 32,
    .block_cycles = 10000,
};

static lfs_t lfs;

void littlefs_vfs_init(char * db_file_to_delete, int format){
	// mount the filesystem
		if(format){
			lfs_format(&lfs, &lfs_cfg);
		}

	    int err = lfs_mount(&lfs, &lfs_cfg);

	    // reformat if we can't mount the filesystem
	    // this should only happen on the first boot
	    if (err) {
	        lfs_format(&lfs, &lfs_cfg);
	        lfs_mount(&lfs, &lfs_cfg);
	    }
	    littlefs_vfs_ls("/");
	    if(db_file_to_delete){
	    	char * wal_file_to_delete = (char *) malloc(strlen(db_file_to_delete) + 10);
	    	char * journal_file_to_delete = (char *) malloc(strlen(db_file_to_delete) + 20);

	    	strcpy(wal_file_to_delete, db_file_to_delete);
	    	strcat(wal_file_to_delete, "-wal");

	    	strcpy(journal_file_to_delete, db_file_to_delete);
	    	strcat(journal_file_to_delete, "-journal");

	    	printf("DB file %s remove result: %d\n", db_file_to_delete, lfs_remove(&lfs, db_file_to_delete));
	    	printf("DB wal %s remove result: %d\n", wal_file_to_delete, lfs_remove(&lfs, wal_file_to_delete));
	    	printf("DB journal %s remove result: %d\n", journal_file_to_delete, lfs_remove(&lfs, journal_file_to_delete));

	    	free(wal_file_to_delete);
	    	free(journal_file_to_delete);
	    }
}

//https://github.com/littlefs-project/littlefs/issues/2
int littlefs_vfs_ls (const char *path) {
    __IO lfs_dir_t dir;
    int err = lfs_dir_open(&lfs, &dir, path);
    if (err) {
        return err;
    }

    struct lfs_info info;
    while (true) {
        int res = lfs_dir_read(&lfs, &dir, &info);
        if (res < 0) {
            return res;
        }

        if (res == 0) {
            break;
        }

        switch (info.type) {
            case LFS_TYPE_REG: printf("reg "); break;
            case LFS_TYPE_DIR: printf("dir "); break;
            default:           printf("?   "); break;
        }

        static const char *prefixes[] = {"", "K", "M", "G"};
        for (int i = sizeof(prefixes)/sizeof(prefixes[0])-1; i >= 0; i--) {
            if (info.size >= (1 << 10*i)-1) {
                printf("%*lu%sB ", 4-(i != 0), info.size >> 10*i, prefixes[i]);
                break;
            }
        }

        printf("%s\n", info.name);
    }

    err = lfs_dir_close(&lfs, &dir);
    if (err) {
        return err;
    }

    return 0;
}

void littlefs_vfs_close(){
	// mount the filesystem
		lfs_unmount(&lfs);
}

static sqlite3_io_methods lfs_file_methods;

int littlefs_vfs_close_file (sqlite3_file* file){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;
	lfs_file_close(&lfs, &(full_file->file));
	return SQLITE_OK;
}

//Buffer: receives read data
//iAmt: num bytes to read
//iOfst: file offset
//WARNING: offset is 64bit, but little fs file seek offset is 32bits.



int littlefs_vfs_read_file(sqlite3_file* file, void* buffer, int iAmt, sqlite3_int64 iOfst){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	//TODO: check return offset matches expected
	lfs_file_seek(&lfs, &(full_file->file),
	        (int32_t) iOfst, LFS_SEEK_SET);

	lfs_file_read(&lfs, &(full_file->file), buffer, iAmt);
	return SQLITE_OK;
}


int littlefs_vfs_write_file(sqlite3_file* file, const void* buffer, int iAmt, sqlite3_int64 iOfst){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	//TODO: check return offset matches expected
	lfs_file_seek(&lfs, &(full_file->file),
			(int32_t) iOfst, LFS_SEEK_SET);
	lfs_file_write(&lfs, &(full_file->file), buffer, iAmt);
	return SQLITE_OK;
}


int littlefs_vfs_truncate_file(sqlite3_file* file, sqlite3_int64 size){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	//TODO: error check
	//WARNING: int64 to 32 conversion
	lfs_file_truncate(&lfs, &(full_file->file), (int32_t) size);
	return SQLITE_OK;
}


int littlefs_vfs_sync_file(sqlite3_file* file, int flags){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	//TODO: error check
	lfs_file_sync(&lfs, &(full_file->file));
	return SQLITE_OK;
}


int littlefs_vfs_size_file(sqlite3_file* file, sqlite3_int64 *pSize){
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	//TODO: error check
	*pSize = (sqlite3_int64) lfs_file_size(&lfs, &(full_file->file));
	return SQLITE_OK;
}


int littlefs_vfs_lock_file(sqlite3_file* file, int flag){
	//No-op
	return SQLITE_OK;
}


int littlefs_vfs_unlock_file(sqlite3_file* file, int flag){
	//No-op
	return SQLITE_OK;
}


int littlefs_vfs_check_reserved_lock_file(sqlite3_file* file, int *pResOut){
	//No-op
	return 0; //False means no lock exists
}

int littlefs_vfs_file_control_file(sqlite3_file* file, int op, void *pArg){
	//No-op
	return SQLITE_OK;
}

int littlefs_vfs_sector_size_file(sqlite3_file* file){
	return lfs_cfg.prog_size;
}

int littlefs_vfs_device_characteristics_file(sqlite3_file* file){
	int characteristics = 0;

	//WARNING: no option for 256bits
	//TODO: if called by SQLite, redo
	characteristics =  SQLITE_IOCAP_SEQUENTIAL;
	return characteristics;
}


int littlefs_vfs_shm_map_file(sqlite3_file* file, int iPg, int pgsz, int bExtend, void volatile** pp){
	//Assuming not used. See: https://www.sqlite.org/tempfiles.html
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: Shared memory map function was used\n");
#endif
	return SQLITE_OK;
}

int littlefs_vfs_shm_lock_file(sqlite3_file* file, int offset, int n, int flags){
	//No-op
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: Shared memory lock function was used\n");
#endif
	return SQLITE_OK;
}


void littlefs_vfs_shm_barrier_file(sqlite3_file* file){
	//No-op
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: Shared memory barrier function was used\n");
#endif
}

int littlefs_vfs_shm_unmap_file(sqlite3_file* file, int deleteFlag){
	//No-op
	return SQLITE_OK;
}


int littlefs_vfs_fetch_file(sqlite3_file* file, sqlite3_int64 iOfst, int iAmt, void **pp){
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: Fetch function was used\n");
#endif
	//No-op
	return SQLITE_OK;
}

int littlefs_vfs_unfetch_file(sqlite3_file* file, sqlite3_int64 iOfst, void *p){
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: Unfetch function was used\n");
#endif
	return SQLITE_OK;
}

static sqlite3_io_methods lfs_file_methods = {
		3,
		littlefs_vfs_close_file,
		littlefs_vfs_read_file,
		littlefs_vfs_write_file,
		littlefs_vfs_truncate_file,
		littlefs_vfs_sync_file,
		littlefs_vfs_size_file,
		littlefs_vfs_lock_file,
		littlefs_vfs_unlock_file,
		littlefs_vfs_check_reserved_lock_file,
		littlefs_vfs_file_control_file,
		littlefs_vfs_sector_size_file,
		littlefs_vfs_device_characteristics_file,
		littlefs_vfs_shm_map_file,
		littlefs_vfs_shm_lock_file,
		littlefs_vfs_shm_barrier_file,
		littlefs_vfs_shm_unmap_file,
		littlefs_vfs_fetch_file,
		littlefs_vfs_unfetch_file
};

int littlefs_vfs_open_file(sqlite3_vfs* vfs, sqlite3_filename zName, sqlite3_file* file,
               int flags, int *pOutFlags){
	//Uses field in vfs structure to know what space to previously allocate to file, so no need to worry about subclassing file
	__IO sqlite3_littlefs_file * full_file = (sqlite3_littlefs_file *) file;

	full_file->pMethods = &lfs_file_methods;

	//TODO: handle errors and flags
    lfs_file_open(&lfs, &(full_file->file),
	        zName, LFS_O_RDWR | LFS_O_CREAT);
    return SQLITE_OK;

}


int littlefs_vfs_delete_file(sqlite3_vfs* vfs, const char *zName,
		int syncDir) //TODO: Fsync dir after delete. Is this needed for littlefs?
{

	lfs_remove(&lfs, zName);
	return SQLITE_OK;
}


int _lfs_file_exists(lfs_t * lfs, const char *zName){
	int result = 0;
	__IO lfs_file_t file;
	if(lfs_file_open(lfs, &file, zName, LFS_O_RDWR) >= 0){
		result = 1;
		lfs_file_close(lfs, &file);
	}
	return result;
}

int littlefs_vfs_access_file(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut){
	int result = 0;
	if(flags==SQLITE_ACCESS_EXISTS){
		result = _lfs_file_exists(&lfs, zName);
	} else if(flags == SQLITE_ACCESS_READWRITE){
		//TODO
		result = 1;
	}

	return result;
}

//TODO: Not implemented
int littlefs_vfs_full_path_name(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut){
	strncpy(zOut, zName, nOut);
	printf("WARNING: Full path name function called\n");
	return SQLITE_OK;
}

void *littlefs_vfs_dl_open(sqlite3_vfs* vfs, const char *zFilename){
	//NO-op
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: DL open function called\n");
#endif
	return NULL;
}


void littlefs_vfs_dl_error(sqlite3_vfs* vfs, int nByte, char *zErrMsg){
	strncpy(zErrMsg,"DL error not implemented\n",nByte);
}

// void (*(*xDlSym)(sqlite3_vfs*,void*, const char *zSymbol))(void); //Just set to NULL
void littlefs_vfs_dl_close(sqlite3_vfs* vfs, void* pHandle){
#ifndef MUTE_SHARED_MEMORY_WARNINGS
	printf("WARNING: DL close function called\n");
#endif
	//No-op
}


int littlefs_vfs_randomness(sqlite3_vfs* vfs, int nByte, char *zOut){
	//TODO: use crypto model instead, or similar
	for(int i = 0; i < nByte; i++){
		zOut[i] = (char) (rand() % 255);
	}
	return SQLITE_OK;
}



#include "stm32h7xx_hal.h"

int littlefs_vfs_sleep (sqlite3_vfs* vfs, int microseconds){
	HAL_Delay((microseconds/1000) + 1);
	return SQLITE_OK;
}


int littlefs_vfs_current_time (sqlite3_vfs* vfs, double* piOut){
	 *piOut = HAL_GetTick() * (sqlite3_int64)8640000;

	 return SQLITE_OK;
}

int littlefs_vfs_last_error(sqlite3_vfs* vfs, int notUsed, char * notUsed2){

	printf("Errors not being tracked\n");
	//No-op
	return SQLITE_OK;
}


int littlefs_vfs_current_time_int64(sqlite3_vfs* vfs, sqlite3_int64* piOut){
	*piOut = HAL_GetTick() * (sqlite3_int64)8640000;

	return SQLITE_OK;
}
//

__IO static sqlite3_vfs littleFsVFS = {
	3,
	sizeof(sqlite3_littlefs_file),
	1024,
	NULL,
	"littlefs",
	0,
	littlefs_vfs_open_file,
	littlefs_vfs_delete_file,
	littlefs_vfs_access_file,
	littlefs_vfs_full_path_name,
	littlefs_vfs_dl_open,
	NULL,
	NULL,
	NULL,
	littlefs_vfs_randomness,
	littlefs_vfs_sleep,
	littlefs_vfs_current_time,
	0,
	littlefs_vfs_current_time_int64
};

sqlite3_vfs * get_littlefs_vfs(){
	return &littleFsVFS;
}
//Set all as NULL
//	  int (*xSetSystemCall)(sqlite3_vfs*, const char *zName, sqlite3_syscall_ptr);
//	  sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char *zName);
//	  const char *(*xNextSystemCall)(sqlite3_vfs*, const char *zName);


/************** End of stm32_little_fs_vfs.c ***********************************************/
