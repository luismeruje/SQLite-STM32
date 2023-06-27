/*
 * little-fs-test.c
 *
 *  Created on: May 4, 2023
 *      Author: luisferreira
 */


#include "lfs.h"
#include "flash_driver.h"
#include "little-fs-test.h"
#include "flash_driver.h"
#include "FLASH_SECTOR_H7.h"
#include "littlefs_driver.h"
#include "definitions.h"
#include <stdio.h>


// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;



// Configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
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
    //.lookahead=32
	.cache_size = 32,
    .lookahead_size = 16,
    .block_cycles = 500,
};


void erase_all_sectors(){
	for(int i = 0; i < 8; i++){
		erase_flash_sector(i, 2);
	}
}
// entry point
int test_lfs(void) {
	//When format is not working, or to reset.
	//erase_all_sectors();
	//lfs_format(&lfs, &cfg);
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count

    uint32_t boot_count = 0;
    err = lfs_mkdir(&lfs, "test");
    if (err) {
    	printf("Error making directory: %d\n", err);
    }
    err = lfs_file_open(&lfs, &file, "test/boot_count", LFS_O_RDWR | LFS_O_CREAT);
    if (err) {
		printf("Error opening file: %d\n", err);
	}
    err = lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
    if (err) {
		printf("Error reading file: %d\n", err);
	}


    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %ld\n", boot_count);
    return 0;
}
