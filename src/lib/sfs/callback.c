#include <sfs/callback.h>
#include <sfs/entry.h>
#include <sfs/io.h>
#include <sfs/defs.h>
#include <sfs/debug.h>

off_t entry_parse(sfs_unit* fs, 
                  entry* entr,
                  int (*callback) (sfs_unit* fs, entry* entr, off_t entry_off, void* data),
                  void* data)
{
        off_t start = fs->entry_start;
        off_t end = fs->vol_indent;

        SFS_TRACE("Callback start");
        while (start != end) {
                if (read_entry(fs->bdev, start, entr) == -1) {
                        SFS_TRACE("Read error");
                        return 0;
                }

                if ((callback(fs, entr, start, data)) != 0) 
                        return start;

                start += INDEX_ENTRY_SIZE;
        }

        return 0;
}
