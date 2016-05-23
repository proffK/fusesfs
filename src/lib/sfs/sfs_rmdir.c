#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/alloc.h>

#define AS_DIR(entr) ((del_dir_entry*) (entr))

int sfs_rmdir(sfs_unit* fs, const char* dirpath)
{
        entry entr;
        off_t off = fs->entry_start;
        off_t start = 0;
        uint8_t n = 0;
        //diriter iter;

        if (is_correct_filepath(dirpath) != 0) {
                SFS_TRACE("Incorrect dirname %s", dirpath);
                SET_ERRNO(EINVAL);
                return -1;
        }
        
        if ((start = search_dir(fs, (char*) dirpath, &entr)) == 0) {
                SFS_TRACE("Dir %s not exist.", dirpath);
                SET_ERRNO(EINVAL);
                return -1;
        }

        off = search_file_mask(fs, (char*) dirpath, &entr, off);

        if (off != start) {
                SFS_TRACE("Dir isn't empty");
                SET_ERRNO(EINVAL);
        }

        off += INDEX_ENTRY_SIZE;

        off = search_file_mask(fs, (char*) dirpath, &entr, off);

        if (off != 0) {
                SFS_TRACE("Dir isn't empty");
                SET_ERRNO(EINVAL);
        }
        
        start = search_dir(fs, (char*) dirpath, &entr);

        n = AS_DIR(&entr)->cont_entries;
        AS_DIR(&entr)->cont_entries = 0;
        AS_DIR(&entr)->entry_type = UNUSED_ENTRY;
        write_entry(fs->bdev, start, &entr);

        start += INDEX_ENTRY_SIZE;
        free_entry(fs, &entr, start, n);

        update(fs);
        return 0;
}

#undef AS_DIR
