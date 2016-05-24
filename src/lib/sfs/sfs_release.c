#include <sfs/unit.h>
#include <bdev/blockdev.h>
#include <sfs/mbr.h>
#include <sfs/utils.h>
#include <sfs/debug.h>
#include <sfs/io.h>
#include <sfs/entry.h>

int sfs_release(sfs_unit* fs)
{      
        update(fs); 
        if (write_data(fs->bdev, fs->vol_ident + 
                                 offsetof(vol_ident_entry, time_stamp),
                       (uint8_t*)&(fs->time), sizeof(fs->time)) == -1)
                return -1;
        return 0;
}
