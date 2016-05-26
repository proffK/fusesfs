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

#include <sfs/fixfs.h>
#include <sfs/fsutils.h>
#include <sfs/io.h>
#include <sfs/defs.h>
#include <sfs/unit.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/callback.h>
#include <sfs/debug.h>

#define AS_DFILE(entr) ((del_file_entry*) (entr))

static int fix_del_file(sfs_unit* fs, entry* entr, off_t entry_off, void* data)
{
        if (entr->entry_type == DEL_FILE_ENTRY) {
                if (strcmp((const char*) AS_DFILE(entr)->name, "free") != 0 && 
                    strcmp((const char*) AS_DFILE(entr)->name, "*free") != 0) {
                        entr->entry_type = FILE_ENTRY;
                        write_entry(fs->bdev, entry_off, entr);
                }
        }
        return 0;
}

int fix_non_del_file(sfs_unit* fs, entry* entr)
{
        entry_parse(fs, entr, fix_del_file, NULL);
        return 0;
}
