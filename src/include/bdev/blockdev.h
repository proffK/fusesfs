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

#ifndef _BDEV_BLOCKDEV_
#define _BDEV_BLOCKDEV_

#include <bdev/defines.h>
#include <sfs/defs.h>

struct block_dev_t {
        void* dev_data;        /* Specific data dri device */
        size_t size;
        size_t block_size;
        buf_t* buf;
        bnum_t buf_num;        /* Number of block in buffer */
        size_t (*read) (struct block_dev_t* dev, buf_t* buf,
                        size_t buf_size, bnum_t block_num);
        size_t (*write) (struct block_dev_t* dev, buf_t* buf,
                         size_t buf_size, bnum_t block_num);
        int (*init) (struct block_dev_t* dev);
        int (*release) (struct block_dev_t* dev);
};

typedef struct block_dev_t blockdev;

inline static int blockdev_init(blockdev* bdev)
{
        if (bdev != NULL)
                return bdev->init(bdev);
        else
                return -1;
}

uint64_t get_time();

#endif
