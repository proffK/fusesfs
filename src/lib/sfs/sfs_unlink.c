#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))

off_t sfs_unlink(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t start = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect filename %s", filepath);
                SET_ERRNO(EINVAL);
                return 0;
        }
        
        if ((start = search_file(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("File %s not exist.", filepath);
                SET_ERRNO(ENOENT);
                return 0;
        }

        AS_FILE(&entr)->entry_type = DEL_FILE_ENTRY;
        write_entry(fs->bdev, start, &entr);

        update(fs);
        SET_ERRNO(0);
        return start;
}

#undef AS_FILE
