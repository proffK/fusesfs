#ifndef _SFS_FSUTILS_
#define _SFS_FSUTILS_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

off_t search_file(sfs_unit* fs, char* filepath, entry* entr);

off_t search_dir(sfs_unit* fs, char* filepath, entry* entr);

int check_dirs(sfs_unit* fs, char* filepath, entry* entr); 

#endif
