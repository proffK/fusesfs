#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))

static inline uint8_t count_entry(size_t len)
{
        if (len <= FIRST_FILE_NAME_SIZE) return 1;

        return 1 + (((len - FIRST_FILE_NAME_SIZE) / (INDEX_ENTRY_SIZE)) + 
               !!(len - FIRST_FILE_NAME_SIZE) % (INDEX_ENTRY_SIZE));
}

int sfs_creat(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t start = 0;
        size_t len = 0;
        uint8_t n = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect filename %s", filepath);
                return -1;
        }
        
        if (check_dirs(fs, (char*) filepath, &entr) != 0) {
                SFS_TRACE("File dirs %s exist. Offset: %lu", filepath, start);
                return -1;
        }

        if ((start = search_dir(fs, (char*) filepath, &entr)) != 0) {
                SFS_TRACE("Dir %s exist. Offset: %lu", filepath, start);
                return -1;
        }

        if ((start = search_file(fs, (char*) filepath, &entr)) != 0) {
                SFS_TRACE("File %s exist. Offset: %lu", filepath, start);
                return -1;
        }

        len = strlen(filepath) + 1; /* With '\0' */
        memset(&entr, 0, INDEX_ENTRY_SIZE);
        n = count_entry(len);

        if ((start = alloc_entry(fs, &entr, n)) == 0) {
                SFS_TRACE("Not enough space for file %s %d", filepath, n);
                return -1;
        }
        SFS_TRACE("Start: %lX %s %d", start, filepath, n);

        if (n == 1) {
                strcpy((char*) AS_FILE(&entr)->name, filepath);
        } else {
                memcpy(AS_FILE(&entr)->name, (uint8_t*) filepath, 
                       FIRST_FILE_NAME_SIZE);
                len -= FIRST_FILE_NAME_SIZE;
                filepath += FIRST_FILE_NAME_SIZE;
        }
        n--;

        AS_FILE(&entr)->time_stamp = get_time();
        AS_FILE(&entr)->entry_type = FILE_ENTRY;
        AS_FILE(&entr)->cont_entries = n;
        write_entry(fs->bdev, start, &entr);
        start += INDEX_ENTRY_SIZE;

        while (n--) {
                SFS_TRACE("Write cont entry %s", filepath);
                strncpy((char*) &entr, filepath, INDEX_ENTRY_SIZE);
                len -= INDEX_ENTRY_SIZE;
                filepath += INDEX_ENTRY_SIZE;
                write_entry(fs->bdev, start, &entr);
                start += INDEX_ENTRY_SIZE;
        }

        return 0;
}
