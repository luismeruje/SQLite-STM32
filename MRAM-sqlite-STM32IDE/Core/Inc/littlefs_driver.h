/*
 * littlefs_driver.h
 *
 *  Created on: 05/05/2023
 *      Author: luisferreira
 */

#ifndef INC_LITTLEFS_DRIVER_H_
#define INC_LITTLEFS_DRIVER_H_


int lfs_read_flash(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);

    // Program a region in a block. The block must have previously
    // been erased. Negative error codes are propagated to the user.
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_prog_flash(const struct lfs_config *c, lfs_block_t block,
		lfs_off_t off, const void *buffer, lfs_size_t size);

    // Erase a block. A block must be erased before being programmed.
    // The state of an erased block is undefined. Negative error codes
    // are propagated to the user.
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfs_erase_sector_flash(const struct lfs_config *c, lfs_block_t block);

    // Sync the state of the underlying block device. Negative error codes
    // are propagated to the user.
int lfs_sync_flash(const struct lfs_config *c);

#endif /* INC_LITTLEFS_DRIVER_H_ */
