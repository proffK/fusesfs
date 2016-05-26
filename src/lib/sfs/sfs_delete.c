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
#include <sfs/entry.h>
#include <sfs/alloc.h>

#define AS_DFILE(entr) ((del_file_entry*) (entr))

int sfs_delete(sfs_unit* fs, off_t file)
{
        entry entr;
        off_t start_block = 0;
        off_t end_block = 0;
        uint8_t n = 0;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        n = AS_DFILE(&entr)->cont_entries;
        start_block = AS_DFILE(&entr)->start_block;
        end_block = AS_DFILE(&entr)->end_block;
        AS_DFILE(&entr)->entry_type = UNUSED_ENTRY;
        write_entry(fs->bdev, file, &entr);

        file += INDEX_ENTRY_SIZE;
        free_entry(fs, &entr, file, n);

        del_file_list_add(fs, &entr, start_block, end_block);

        update(fs);
        SET_ERRNO(0);
        return 0;
}

#undef AS_DFILE
