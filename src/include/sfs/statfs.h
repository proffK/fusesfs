#ifndef __STATFS__
#define __STATFS__
#include <sfs/defs.h>
#include <sfs/unit.h>

size_t scan_del_file_list(sfs_unit* fs, entry* entr);

size_t scan_used_space(sfs_unit* fs, entry* entr);

size_t scan_free_inode(sfs_unit* fs, entry* entr);
#endif
