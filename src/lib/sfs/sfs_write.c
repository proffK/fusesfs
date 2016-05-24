#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))
#define AS_DFILE(entr) ((del_file_entry*) (entr))

ssize_t sfs_write(sfs_unit* fs, off_t file, 
                  char* data, size_t size, off_t off)
{
        entry entr;
        size_t old_size = 0;
        size_t new_size = 0;
        off_t old_start = 0;
        off_t old_end = 0;
        off_t new_start = 0;
        off_t new_end = 0;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        old_size = AS_FILE(&entr)->size;
        old_start = AS_FILE(&entr)->start_block;
        old_end = AS_FILE(&entr)->end_block;
        SFS_TRACE("Try write to file:\n"
                        "Size:  %lu\n"
                        "Start: %lu\n"
                        "End:   %lu", old_size,
                        old_start, old_end);
 
        if (off + size > old_size) {
                new_size = off + size;
        } else {
                new_size = old_size;
        }

        new_start = old_start;
        new_end = old_end;
        SFS_TRACE("New size %lu", new_size);

        if (get_real_size(fs, new_size) > get_real_size(fs, old_size)) {
                if (new_start != 0) {
                        if (try_expand(fs, file, new_size, &entr) == 0) {
                                new_end = new_start + 
                                          (get_real_size(fs, new_size) 
                                               / fs->bdev->block_size) - 1;
                                goto END;
                        }
                } 
                if ((new_start = del_file_list_alloc(fs, &entr, new_size))
                                        == 0) {
                        SFS_TRACE("Can't alloc space");
                        SET_ERRNO(ENOSPC);
                        return -1;
                }
                new_end = new_start + (get_real_size(fs, new_size) 
                                       / fs->bdev->block_size) - 1;
                
                if (old_start != 0) {
                        del_file_list_add(fs, &entr, old_start, old_end);
                        if (copy_block(fs->bdev, old_start, new_start, 
                                       old_end - old_start + 1) != 0) {
                                SFS_TRACE("Can't copy blocks");
                                SET_ERRNO(EIO);
                                return -1;
                        }
                }
        }
END:
        read_entry(fs->bdev, file, &entr);
        AS_FILE(&entr)->start_block = new_start;
        AS_FILE(&entr)->end_block = new_end;
        AS_FILE(&entr)->size = new_size;
        AS_FILE(&entr)->time_stamp = get_time();
        write_entry(fs->bdev, file, &entr);
 
        update(fs);
        SET_ERRNO(0);
        return write_data(fs->bdev, new_start * fs->bdev->block_size + off,
                          (uint8_t*) data, size);
}
