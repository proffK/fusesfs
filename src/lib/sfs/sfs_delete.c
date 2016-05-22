#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/alloc.h>

#define AS_DFILE(entr) ((del_file_entry*) (entr))

int sfs_delete(sfs_unit* fs, off_t file)
{
        entry entr;
        off_t start_block = 0;
        off_t end_block = 0;
        uint8_t n = 0;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        n = AS_DFILE(&entr)->cont_entries;
        start_block = AS_DFILE(&entr)->start_block;
        end_block = AS_DFILE(&entr)->end_block;
        AS_DFILE(&entr)->entry_type = UNUSED_ENTRY;
        write_entry(fs->bdev, file, &entr);

        file += INDEX_ENTRY_SIZE;
        n--;
        free_entry(fs, &entr, file, n);

        del_file_list_add(fs, &entr, start_block, end_block);

        update(fs);
        return 0;
}

#undef AS_DFILE
