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

#include <sfs/callback.h>
#include <sfs/entry.h>
#include <sfs/defs.h>
#include <sfs/unit.h>
#include <sfs/utils.h>
#include <sfs/io.h>
#include <sfs/debug.h>

off_t entry_parse(sfs_unit* fs, 
                  entry* entr,
                  int (*callback) (sfs_unit* fs, entry* entr, off_t entry_off, void* data),
                  void* data)
{
        off_t start = fs->entry_start;
        off_t end = fs->vol_ident;

        SFS_TRACE("Callback start");
        while (start != end) {
                if (read_entry(fs->bdev, start, entr) == -1) {
                        SFS_TRACE("Read error");
                        SET_ERRNO(EIO);
                        return 0;
                }

                if ((callback(fs, entr, start, data)) != 0) 
                        return start;

                start += INDEX_ENTRY_SIZE;
        }

        return 0;
}
