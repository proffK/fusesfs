#ifndef _SFS_FSUTILS_
#define _SFS_FSUTILS_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

#define AS_DFILE(entr) ((del_file_entry*) (entr))
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

static inline size_t get_size(sfs_unit* fs, entry* entr) 
{
        return fs->bdev->block_size * ((AS_DFILE(entr)->end_block) - 
                AS_DFILE(entr)->start_block + 1);
}

#define NEXT_DEL(entr) AS_DFILE(entr)->size
#define PREV_DEL(entr) AS_DFILE(entr)->time_stamp

static inline off_t del_next(sfs_unit* fs, entry* entr)
{
        off_t ret = NEXT_DEL(entr);
        if (ret == 0) {
                return 0;
        }
        if (read_entry(fs->bdev, ret, entr) == -1) {
                return (off_t) -1;
        }
        return ret;
}

#undef AS_DFILE
#endif
