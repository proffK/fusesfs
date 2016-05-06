#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((del_file_entry*) (entr))

int sfs_unlink(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t start = 0;
        uint8_t n = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect filename %s", filepath);
                return -1;
        }
        
        if ((start = search_file(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("File %s not exist.", filepath);
                return -1;
        }

        n = AS_FILE(&entr)->cont_entries;
        AS_FILE(&entr)->cont_entries = 0;
        AS_FILE(&entr)->entry_type = DEL_FILE_ENTRY;
        write_entry(fs->bdev, start, &entr);

        start += INDEX_ENTRY_SIZE;
        n--;
        free_entry(fs, &entr, start, n);

        update(fs);
        return 0;
}

#undef AS_FILE
