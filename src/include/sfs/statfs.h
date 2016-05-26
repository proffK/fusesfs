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

#ifndef __STATFS__
#define __STATFS__
#include <sfs/defs.h>
#include <sfs/unit.h>

size_t scan_del_file_list(sfs_unit* fs, entry* entr);

size_t scan_used_space(sfs_unit* fs, entry* entr);

size_t scan_free_inode(sfs_unit* fs, entry* entr);
#endif
