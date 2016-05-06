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

#endif
