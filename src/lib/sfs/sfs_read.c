#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))

ssize_t sfs_read(sfs_unit* fs, off_t file, 
                  char* data, size_t size, off_t off)
{
        entry entr;
        size_t file_size = 0;
        off_t file_start = 0;
        size_t read_size = 0;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
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
        SET_ERRNO(0);
        return read_data(fs->bdev, file_start * fs->bdev->block_size + off,
                         (uint8_t*) data, read_size);
}
