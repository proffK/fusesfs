#ifndef _SFS_CALLBACK_
#define _SFS_CALLBACK_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

off_t entry_parse(sfs_unit* fs, 
                  entry* entr,
                  int (*callback) (sfs_unit* fs, entry* entr, 
                                   off_t entry_off, void* data),
                  void* data);
#endif
