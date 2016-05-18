#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))

ssize_t sfs_read(sfs_unit* fs, const char* filepath, 
                  char* data, size_t size, off_t off)
{
        entry entr;
        off_t start = 0;
        size_t file_size = 0;
        off_t file_start = 0;
        size_t read_size = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect filename %s", filepath);
                return -1;
        }
        
        if ((start = search_file(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("File not %s exist. Offset: %lu", filepath, start);
                return -1;
        }
        file_size = AS_FILE(&entr)->size;
        file_start = AS_FILE(&entr)->start_block;

        if (off > file_size) {
                SET_ERRNO(EINVAL);
                return -1;
        }
        
        if (off + size > file_size) {
                read_size = file_size - off;
        } else {
                read_size = size;
        }

        update(fs);
        return read_data(fs->bdev, file_start * fs->bdev->block_size + off,
                         (uint8_t*) data, read_size);
}
