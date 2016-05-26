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

#include <bdev/filedev.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define FDEV ((filedev_data*) bdev->dev_data)
static int filedev_init(blockdev* bdev) 
{
        errno = 0;

        if (FDEV->filename == NULL) {
                errno = EINVAL;
                return -1;
        }
        
        if (FDEV->fd == -1) 
                FDEV->fd = open(FDEV->filename, O_RDWR);

        if (FDEV->fd == -1) 
                return -1;

        if ((bdev->buf = (buf_t*) malloc (bdev->block_size)) == NULL) {
                errno = ENOMEM;
                return -1;
        }

        bdev->buf_num = (bnum_t) -1;

        return 0;
}

static int filedev_release(blockdev* bdev) 
{
        errno = 0;

        if (FDEV->filename == NULL) {
                errno = EINVAL;
                return -1;
        }
        
        if (FDEV->fd != -1) {
                close(FDEV->fd);
                FDEV->fd = -1;
        }

        free(bdev->buf);

        return 0;
}

static int filedev_sync(blockdev* bdev) 
{
        return fsync(FDEV->fd);
}

static size_t filedev_read(blockdev* bdev, buf_t* buf, 
                           size_t buf_size, bnum_t block_num)
{
        ssize_t size = 0;
        off_t start_pos = 0;
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (FDEV->fd == -1) {
                errno = EINVAL;
                return -1;
        }
        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return -1;
        }

        start_pos = block_num * bdev->block_size;
        if (start_pos + buf_size > bdev->size) {
                errno = EINVAL;
                return -1;
        }

        start_pos = lseek(FDEV->fd, start_pos, SEEK_SET);

        if (start_pos == -1) 
                return -1;

        size = read(FDEV->fd, buf, buf_size);

        if (size != buf_size)
                return -1;

        return (size_t) size;
}

static size_t filedev_write(blockdev* bdev, buf_t* buf, 
                            size_t buf_size, bnum_t block_num)
{
        ssize_t size = 0;
        off_t start_pos = 0;
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return -1;
        }

        if (FDEV->fd == -1) {
                errno = EINVAL;
                return -1;
        }

        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return -1;
        }

        start_pos = block_num * bdev->block_size;
        start_pos = lseek(FDEV->fd, start_pos, SEEK_SET);

        if (start_pos == -1) 
                return -1;

        size = write(FDEV->fd, buf, buf_size);

        if (size != buf_size)
                return -1;

        return (size_t) size;
}
#undef FDEV

#ifdef FILEDEV_DEBUG
#define DEBUG_MSG_SIZE 1024
int filedev_dump(blockdev* fdev, int fd)
{
        char debug_msg[DEBUG_MSG_SIZE];
        int size = snprintf(debug_msg, DEBUG_MSG_SIZE - 1,
                        "File block device located in %p\n"
                        "Device data located in %p\n"
                        "Filename:   %s\n"
                        "File desc:  %d\n"
                        "File size:  %lu\n"
                        "Block size: %lu\n"
                        "Read func:  %p\n"
                        "Write func: %p\n"
                        "Init func:  %p\n",
                        fdev,
                        fdev->dev_data,
                        fdev->((filedev_data*) dev_data)->filename,
                        fdev->((filedev_data*) dev_data)->fd,
                        fdev->size,
                        fdev->block_size,
                        fdev->read,
                        fdev->write,
                        fdev->init);

        if (size < 0) 
                return -1;

        debug_msg[size] = '\0';

        return 0;
}
#undef DEBUG_MSG_SIZE
#endif

blockdev* filedev_create(blockdev* bdev, filedev_data* fdev, 
                         size_t block_size, size_t size)
{
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return NULL;
        }

        if (size % block_size) {
                errno = EINVAL;
                return NULL;
        }

        bdev->size = size;
        bdev->block_size = block_size;
        bdev->dev_data = fdev;
        bdev->init = filedev_init;
        bdev->read = filedev_read;
        bdev->write = filedev_write;
        bdev->release = filedev_release;
        bdev->sync = filedev_sync;

        return bdev;
}

uint64_t get_time() {
        return (uint64_t) time(NULL);
}
