#include <sfs/fixfs.h>
#include <sfs/fsutils.h>
#include <sfs/io.h>
#include <sfs/defs.h>
#include <sfs/unit.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/callback.h>
#include <sfs/debug.h>

#define AS_DFILE(entr) ((del_file_entry*) (entr))

static int fix_del_file(sfs_unit* fs, entry* entr, off_t entry_off, void* data)
{
        if (entr->entry_type == DEL_FILE_ENTRY) {
                if (strcmp((const char*) AS_DFILE(entr)->name, "free") != 0 && 
                    strcmp((const char*) AS_DFILE(entr)->name, "*free") != 0) {
                        entr->entry_type = FILE_ENTRY;
                        write_entry(fs->bdev, entry_off, entr);
                }
        }
        return 0;
}

int fix_non_del_file(sfs_unit* fs, entry* entr)
{
        entry_parse(fs, entr, fix_del_file, NULL);
        return 0;
}
