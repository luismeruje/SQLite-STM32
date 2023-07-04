/*
Copyright 2023 INESC TEC
This software is authored by:
Luís Manuel Meruje Ferreira (INESC TEC) *
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at *
http://www.apache.org/licenses/LICENSE-2.0 */

#ifndef INC_LITTLEFS_SQLITE_VFS_H_
#define INC_LITTLEFS_SQLITE_VFS_H_
#include "sqlite3.h"

sqlite3_vfs * get_littlefs_vfs();
void littlefs_vfs_init(char * db_file_to_delete, int format);
int littlefs_vfs_ls (const char *path);
void littlefs_vfs_close();

#endif /* INC_LITTLEFS_SQLITE_VFS_H_ */
