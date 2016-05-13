#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

int sfs_getattr(sfs_unit* fs, const char* filepath, sfs_attr* attr)
{
        entry entr;
        struct diriter;
        off_t off = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect pathname %s", filepath);
                return -1;
        }
        off = search_dir(fs, (char*) filepath, &entr);

        if (off == 0) {
                off = search_file(fs, (char*) filepath, &entr);
        }
        if (off == 0) {
                return -1;
        }
        if (entr.entry_type == FILE_ENTRY) {
                SFS_TRACE("******FILE_ENTRY");
                attr->type = FILE_ITER_TYPE;
                attr->time = ((file_entry*) &entr)->time_stamp;
                attr->size = ((file_entry*) &entr)->size;
                attr->off = off;
        }
        if (entr.entry_type == DIR_ENTRY) {
                attr->type = DIR_ITER_TYPE;
                attr->time = ((dir_entry*) &entr)->time_stamp;
                attr->size = 0;
                attr->off = off;
        }

        return 0;
}
