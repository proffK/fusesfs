#ifndef _SFS_FSUTILS_
#define _SFS_FSUTILS_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

off_t search_file(sfs_unit* fs, char* filepath, entry* entr);

off_t search_dir(sfs_unit* fs, char* filepath, entry* entr);

int check_dirs(sfs_unit* fs, char* filepath, entry* entr); 

int read_dir_name(sfs_unit* fs, dir_entry* entr, off_t dir_off, 
                  char* str, size_t len);

int read_file_name(sfs_unit* fs, file_entry* entr, off_t file_off, 
                   char* str, size_t len);

off_t search_file_mask(sfs_unit* fs, char* filepath, 
                       entry* entr, off_t entr_off);

off_t del_file_list_alloc(sfs_unit* fs, entry* entr, size_t size);

int del_file_list_add(sfs_unit* fs, entry* entr, uint64_t start, uint64_t end);

int try_expand(sfs_unit* fs, off_t off, size_t new_size, entry* entr);

int file_shrink(sfs_unit* fs, off_t off, size_t new_size, entry* entr);

static inline size_t get_real_size(sfs_unit* fs, size_t size)
{
        return fs->bdev->block_size * ((size / fs->bdev->block_size) +
                        !!(size % fs->bdev->block_size));
}

#define NEXT_DEL(entr) AS_DFILE(entr)->size
#define PREV_DEL(entr) AS_DFILE(entr)->time_stamp

#endif
