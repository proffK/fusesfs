#include <bdev/filedev.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>

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

        return 0;
}

static size_t filedev_read(blockdev* bdev, buf_t* buf, 
                           size_t buf_size, bnum_t block_num)
{
        ssize_t size = 0;
        off_t start_pos = 0;
        errno = 0;

        if (bdev == NULL) {
                errno = EFAULT;
                return 0;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return 0;
        }

        if (FDEV->fd == -1) {
                errno = EINVAL;
                return 0;
        }

        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return 0;
        }

        start_pos = block_num * bdev->block_size;

        if (start_pos + buf_size > bdev->size) {
                errno = EINVAL;
                return 0;
        }

        start_pos = lseek(FDEV->fd, start_pos, SEEK_SET);

        if (start_pos == -1) 
                return 0;

        size = read(FDEV->fd, buf, buf_size);

        if (size != buf_size)
                return 0;

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
                return 0;
        }

        if (buf == NULL) {
                errno = EFAULT;
                return 0;
        }

        if (FDEV->fd == -1) {
                errno = EINVAL;
                return 0;
        }

        if (buf_size % bdev->block_size) {
                errno = EINVAL;
                return 0;
        }

        start_pos = block_num * bdev->block_size;
        start_pos = lseek(FDEV->fd, start_pos, SEEK_SET);

        if (start_pos == -1) 
                return 0;

        size = write(FDEV->fd, buf, buf_size);

        if (size != buf_size)
                return 0;

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

        return bdev;
}

       

