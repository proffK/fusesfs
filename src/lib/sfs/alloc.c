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

#include <sfs/alloc.h>
#include <sfs/debug.h>
#include <sfs/entry.h>
#include <sfs/callback.h>

struct pair {
        int start;
        int cur;
};

#define CUR ((struct pair*) count)->cur
#define START ((struct pair*) count)->start

static int search_free_entry(sfs_unit* fs, entry* entr, 
                             off_t entry_off, void* count) 
{
        if (entr->entry_type != UNUSED_ENTRY) {
                if (CUR != START) 
                        CUR = START;
                return 0;
        }

        CUR--;
        if (CUR == 0) 
                return START;

        return 0;
}

#undef CUR
#undef START

static int search_del_space(sfs_unit* fs, entry* entr, 
                             off_t entry_off, void* bsize)
{
        if (entr->entry_type != DEL_FILE_ENTRY) 
                return 0;

        if (((del_file_entry*) entr)->end_block -
            ((del_file_entry*) entr)->start_block + 1 
              >= *((size_t*) bsize))
                return 1;

        return 0;
}

off_t alloc_entry(sfs_unit* fs, entry* entr, int n)
{
        struct pair p;
        off_t ret = 0;

        if (n <= 0) {
                SFS_TRACE("Incorrect n: %d", n);
                return 0;
        }

        p.start = n;
        p.cur = n;

        if ((ret = entry_parse(fs, entr, search_free_entry, &p)) == 0) 
                return 0;

        ret -= INDEX_ENTRY_SIZE * (n - 1); 

        return ret;
}

static int free_one_entry(sfs_unit* fs, entry* entr, off_t entr_off)
{
        if (read_entry(fs->bdev, entr_off, entr) == -1) {
                return -1;
        }
        entr->entry_type = UNUSED_ENTRY;
        write_entry(fs->bdev, entr_off, entr);
        return 0;
}

int free_entry(sfs_unit* fs, entry* entr, off_t entr_off, int n)
{
        int i = 0;

        for (i = 0; i < n; ++i) {
                if (free_one_entry(fs, entr, entr_off) == -1)
                        return -1;
                entr_off += INDEX_ENTRY_SIZE;
        }
        return 0;
}

off_t alloc_space(sfs_unit* fs, size_t size, entry* entr)
{
        size_t block_size = fs->bdev->block_size;
        size_t bsize = ((size / block_size) +
                        !!(size % block_size));
        off_t ret = 0;

        ret = entry_parse(fs, entr, search_del_space, &bsize);

        return ret;
}
