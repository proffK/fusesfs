#ifndef _BDEV_BLOCKDEV_
#define _BDEV_BLOCKDEV_

#include <bdev/defines.h>

struct block_dev_t {
        /* Specific data dri device */
        void* dev_data;
        size_t size;
        size_t block_size;
        size_t (*read) (struct block_dev_t* dev, buf_t* buf,
                        size_t buf_size, bnum_t block_num);
        size_t (*write) (struct block_dev_t* dev, buf_t* buf,
                         size_t buf_size, bnum_t block_num);
        int (*init) (struct block_dev_t* dev);
};

typedef struct block_dev_t blockdev;

inline static int blockdev_init(blockdev* bdev)
{
        if (bdev != NULL)
                return bdev->init(bdev);
        else
                return -1;
}

#endif
