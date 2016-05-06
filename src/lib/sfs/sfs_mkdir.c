#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_DIR(entr) ((dir_entry*) (entr))

static inline uint8_t count_entry(size_t len)
{
        if (len <= FIRST_DIR_NAME_SIZE) return 1;

        return 1 + (((len - FIRST_DIR_NAME_SIZE) / (INDEX_ENTRY_SIZE)) + 
               !!(len - FIRST_DIR_NAME_SIZE) % (INDEX_ENTRY_SIZE));
}

int sfs_mkdir(sfs_unit* fs, const char* dirpath)
{
        entry entr;
        off_t start = 0;
        size_t len = 0;
        uint8_t n = 0;

        if (is_correct_dirpath(dirpath) != 0) {
                SFS_TRACE("Incorrect dirname %s", dirpath);
                return -1;
        }
        
        if (check_dirs(fs, (char*) dirpath, &entr) != 0) {
                SFS_TRACE("File dirs %s exist. Offset: %lu", dirpath, start);
                return -1;
        }
 
        if ((start = search_dir(fs, (char*) dirpath, &entr)) != 0) {
                SFS_TRACE("Dir %s exist. Offset: %lu", dirpath, start);
                return -1;
        }

        len = strlen(dirpath) + 1; /* With '\0' */
        memset(&entr, 0, INDEX_ENTRY_SIZE);
        n = count_entry(len);

        if ((start = alloc_entry(fs, &entr, n)) == 0) {
                SFS_TRACE("Not enough space for dir %s", dirpath);
                return -1;
        }

        if (n == 1) {
                strcpy((char*) AS_DIR(&entr)->dir_name, dirpath);
        } else {
                memcpy(AS_DIR(&entr)->dir_name, (char*) dirpath,
                       FIRST_DIR_NAME_SIZE);
                len -= FIRST_DIR_NAME_SIZE;
                dirpath += FIRST_DIR_NAME_SIZE;
        }

        n--;
        AS_DIR(&entr)->time_stamp = get_time();
        AS_DIR(&entr)->entry_type = DIR_ENTRY;
        AS_DIR(&entr)->cont_entries = n;
        write_entry(fs->bdev, start, &entr);
        start += INDEX_ENTRY_SIZE;

        while (n--) {
                strncpy((char*) &entr, dirpath, INDEX_ENTRY_SIZE);
                len -= INDEX_ENTRY_SIZE;
                dirpath += INDEX_ENTRY_SIZE;
                write_entry(fs->bdev, start, &entr);
                start += INDEX_ENTRY_SIZE;
        }

        return 0;
}

#undef AS_DIR
