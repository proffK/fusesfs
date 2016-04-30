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
        flag_t type;
} diriter;

int update(sfs_unit* fs);

int sfs_init(sfs_unit* fs, blockdev* bdev);

int sfs_release(sfs_unit* fs);

ssize_t sfs_read(sfs_unit* fs, const char* filepath, uint8_t* data, size_t size);

int sfs_creat(sfs_unit* fs, const char* filepath);

int sfs_unlink(sfs_unit* fs, const char* filepath);

int sfs_rename(sfs_unit* fs, const char* oldpath, const char* newpath);

time_t sfs_gettime(sfs_unit* fs, const char* filepath);

size_t sfs_getsize(sfs_unit* fs, const char* filepath);

int sfs_mkdir(sfs_unit* fs, const char* dirpath);

int sfs_rmdir(sfs_unit* fs, const char* dirpath);

int sfs_readdir(sfs_unit* fs, diriter* iter);

ssize_t sfs_write(sfs_unit* fs, const char* filepath, uint8_t* data, size_t size);
#endif
