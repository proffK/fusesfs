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

#include <sfs/statfs.h>
#include <sfs/fsutils.h>
#include <sfs/io.h>
#include <sfs/defs.h>
#include <sfs/unit.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/callback.h>
#include <sfs/debug.h>

size_t scan_del_file_list(sfs_unit* fs, entry* entr)
{
        off_t off = fs->del_begin;
        size_t cur_size = 0;
        uint64_t watchdog = 1 + (fs->vol_ident - fs->entry_start) 
                                / INDEX_ENTRY_SIZE;

        while (off != 0 && watchdog--) {
                if (read_entry(fs->bdev, off, entr) == 0) {
                        SFS_TRACE("Can't read");
                        SET_ERRNO(EIO);
                        return -1;
                }
                cur_size += get_size(fs, entr);
                off = del_next(fs, entr);
                if (off == -1) {
                        SET_ERRNO(EFAULT);
                        return -1;
                }
        }
        if (watchdog == 0) {
                SET_ERRNO(ETIMEDOUT);
                return -1;
        }
        SET_ERRNO(0);
        return cur_size;
}

static int count_space(sfs_unit* fs, entry* entr, off_t entry_off, void* data)
{
        if (entr->entry_type != FILE_ENTRY) return 0;

        *((size_t*) data) += get_size(fs, entr);
        return 0;
}

size_t scan_used_space(sfs_unit* fs, entry* entr)
{
        size_t size = 0;
        entry_parse(fs, entr, count_space, &size);
        return size;
}

static int count_unused_entry(sfs_unit* fs, entry* entr, 
                              off_t entry_off, void* data)
{
        if (entr->entry_type == UNUSED_ENTRY) 
                (*((size_t*) data))++;
        return 0;
}

size_t scan_free_inode(sfs_unit* fs, entry* entr)
{
        size_t size = 0;
        entry_parse(fs, entr, count_unused_entry, &size);
        return size;
}
