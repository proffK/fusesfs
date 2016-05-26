/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
                SET_ERRNO(EINVAL);
                return -1;
        }
        
        if (check_dirs(fs, (char*) dirpath, &entr) != 0) {
                SFS_TRACE("File dirs %s exist. Offset: %lu", dirpath, start);
                SET_ERRNO(EEXIST);
                return -1;
        }
 
        if ((start = search_dir(fs, (char*) dirpath, &entr)) != 0) {
                SFS_TRACE("Dir %s exist. Offset: %lu", dirpath, start);
                SET_ERRNO(EEXIST);
                return -1;
        }

        len = strlen(dirpath) + 1; /* With '\0' */
        memset(&entr, 0, INDEX_ENTRY_SIZE);
        n = count_entry(len);

        if ((start = alloc_entry(fs, &entr, n)) == 0) {
                SFS_TRACE("Not enough space for dir %s", dirpath);
                SET_ERRNO(ENOSPC);
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

        update(fs);
        SET_ERRNO(0);
        return 0;
}

#undef AS_DIR
