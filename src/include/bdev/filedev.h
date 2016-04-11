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
