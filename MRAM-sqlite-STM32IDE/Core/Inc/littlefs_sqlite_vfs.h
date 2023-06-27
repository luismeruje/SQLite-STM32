/*
 * littlefs_sqlite_vfs.h
 *
 *  Created on: Jun 20, 2023
 *      Author: luisferreira
 */

#ifndef INC_LITTLEFS_SQLITE_VFS_H_
#define INC_LITTLEFS_SQLITE_VFS_H_
#include "sqlite3.h"

sqlite3_vfs * get_littlefs_vfs();
void littlefs_vfs_init(char * db_file_to_delete, int format);
int littlefs_vfs_ls (const char *path);
void littlefs_vfs_close();

#endif /* INC_LITTLEFS_SQLITE_VFS_H_ */
