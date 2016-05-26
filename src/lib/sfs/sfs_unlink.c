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

#define AS_FILE(entr) ((file_entry*) (entr))

off_t sfs_unlink(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t start = 0;

        if (is_correct_filepath(filepath) != 0) {
                SFS_TRACE("Incorrect filename %s", filepath);
                SET_ERRNO(EINVAL);
                return 0;
        }
        
        if ((start = search_file(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("File %s not exist.", filepath);
                SET_ERRNO(ENOENT);
                return 0;
        }

        AS_FILE(&entr)->entry_type = DEL_FILE_ENTRY;
        write_entry(fs->bdev, start, &entr);

        update(fs);
        SET_ERRNO(0);
        return start;
}

#undef AS_FILE
