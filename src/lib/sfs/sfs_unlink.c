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
        off_t start_block = 0;
        off_t end_block = 0;
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
        start_block = AS_FILE(&entr)->start_block;
        end_block = AS_FILE(&entr)->end_block;
        AS_FILE(&entr)->entry_type = UNUSED_ENTRY;
        write_entry(fs->bdev, start, &entr);

        start += INDEX_ENTRY_SIZE;
        n--;
        free_entry(fs, &entr, start, n);

        del_file_list_add(fs, &entr, start_block, end_block);

        update(fs);
        return 0;
}

#undef AS_FILE
