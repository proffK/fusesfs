/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev, Edgar Kaziahmedov>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SFS_UNIT_
#define _SFS_UNIT_
#include <bdev/blockdev.h>
#include <sfs/io.h>
#include <sfs/defs.h>
 
typedef struct {
        blockdev* bdev;
        time_t time;
        off_t entry_start;
        off_t del_begin;
        off_t vol_ident;
} sfs_unit;

typedef struct {
        char* filename;
        size_t len;
        off_t cur_off;
        size_t size;
        uint64_t time;
        flag_t type;
} diriter;

typedef struct {
        uint64_t time;
        size_t size;
        off_t off;
        flag_t type;
} sfs_attr;

enum iter_types {
        FILE_ITER_TYPE = 1,
        DIR_ITER_TYPE
};

static inline void update(sfs_unit* fs)
{
        fs->time = get_time();
}

int sfs_init(sfs_unit* fs, blockdev* bdev);

off_t sfs_open(sfs_unit* fs, const char* filepath);

int sfs_release(sfs_unit* fs);

ssize_t sfs_read(sfs_unit* fs, off_t file, char* data, size_t size, off_t off);

int sfs_creat(sfs_unit* fs, const char* filepath);

off_t sfs_unlink(sfs_unit* fs, const char* filepath);

int sfs_delete(sfs_unit* fs, off_t file);

off_t sfs_rename(sfs_unit* fs, off_t file, const char* newpath);

int sfs_getattr(sfs_unit* fs, off_t file, sfs_attr* attr);

int sfs_mkdir(sfs_unit* fs, const char* dirpath);

int sfs_rmdir(sfs_unit* fs, const char* dirpath);

int sfs_readdir(sfs_unit* fs, diriter* iter);

ssize_t sfs_write(sfs_unit* fs, off_t file, 
                  char* data, size_t size, off_t off);

int sfs_truncate(sfs_unit* fs, off_t file, size_t new_size);
#endif
