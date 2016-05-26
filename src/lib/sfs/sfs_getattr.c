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

#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

int sfs_getattr(sfs_unit* fs, off_t file, sfs_attr* attr)
{
        entry entr;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return -1;
        }

        if (entr.entry_type == FILE_ENTRY) {
                attr->type = FILE_ITER_TYPE;
                attr->time = ((file_entry*) &entr)->time_stamp;
                attr->size = ((file_entry*) &entr)->size;
                attr->off = file;
        }
        if (entr.entry_type == DIR_ENTRY) {
                attr->type = DIR_ITER_TYPE;
                attr->time = ((dir_entry*) &entr)->time_stamp;
                attr->size = 0;
                attr->off = file;
        }
        SET_ERRNO(0);
        return 0;
}
