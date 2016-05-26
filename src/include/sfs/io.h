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

#ifndef _SFS_IO_
#define _SFS_IO_
#include <sfs/defs.h>
#include <sfs/entry.h>
#include <bdev/blockdev.h>
#include <stdint.h>

size_t read_data(blockdev* dev, off_t offset, uint8_t* data, size_t size);

size_t write_data(blockdev* dev, off_t offset, uint8_t* data, size_t size);

size_t read_entry(blockdev* dev, off_t offset, entry* entry);

size_t write_entry(blockdev* dev, off_t offset, entry* entry);

int copy_block(blockdev* dev, off_t src, off_t dest, size_t size); 
#endif
