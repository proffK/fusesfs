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

#ifndef _ALLOC_SFS_
#define _ALLOC_SFS_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <sfs/unit.h>

off_t alloc_entry(sfs_unit* fs, entry* entr, int n);

int free_entry(sfs_unit* fs, entry* entr, off_t entr_off, int n);

off_t alloc_space(sfs_unit* fs, size_t size, entry* entr);

#endif 
