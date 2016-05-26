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

off_t sfs_open(sfs_unit* fs, const char* filepath)
{
        entry entr;
        off_t file = 0;

        if ((file = search_file(fs, (char*) filepath, &entr)) == 0) {
                if ((file = search_dir(fs, (char*) filepath, &entr)) == 0) {
                SFS_TRACE("Entry not %s exist. Offset: %lu", filepath, file);
                SET_ERRNO(ENOENT);
                }
        }
        
        return file;
}
