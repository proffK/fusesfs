#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

off_t sfs_open(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t file = 0;

        if ((file = search_file(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("File not %s exist. Offset: %lu", filepath, file);
        }

        return file;
}
