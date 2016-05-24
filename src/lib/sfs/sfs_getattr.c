#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

int sfs_getattr(sfs_unit* fs, off_t file, sfs_attr* attr)
{
        entry entr;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return -1;
        }

        if (entr.entry_type == FILE_ENTRY) {
                attr->type = FILE_ITER_TYPE;
                attr->time = ((file_entry*) &entr)->time_stamp;
                attr->size = ((file_entry*) &entr)->size;
                attr->off = file;
        }
        if (entr.entry_type == DIR_ENTRY) {
                attr->type = DIR_ITER_TYPE;
                attr->time = ((dir_entry*) &entr)->time_stamp;
                attr->size = 0;
                attr->off = file;
        }
        SET_ERRNO(0);
        return 0;
}
