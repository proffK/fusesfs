#ifndef _ALLOC_SFS_
#define _ALLOC_SFS_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

off_t alloc_entry(sfs_unit* fs, entry* entr, int n);

int free_entry(sfs_unit* fs, entry* entr, off_t entr_off, int n);

off_t alloc_space(sfs_unit* fs, size_t size);

int free_space(sfs_unit* fs, off_t space);

#endif 
