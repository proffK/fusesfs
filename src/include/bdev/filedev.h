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

#ifndef _BDEV_FILEDEV_
#define _BDEV_FILEDEV_

#include <bdev/blockdev.h>

typedef struct filedev_data_t {
        char* filename;
        /* Must be -1 for initializing */
        int fd;                 
} filedev_data;

blockdev* filedev_create(blockdev* bdev, filedev_data* fdev,
                         size_t block_size, size_t size);

#ifdef FILEDEV_DEBUG
int filedev_dump(blockdev* fdev, int fd);
#endif

#endif
