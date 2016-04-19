#ifndef _SFS_IO_
#define _SFS_IO_
#include <stddef.h>
#include <bdev/blockdev.h>

ssize_t read_data(blockdev* dev, uint64_t offset, void* data, size_t size);

ssize_t write_data(blockdev* dev, uint64_t offset, void* data, size_t size);

ssize_t read_entry(blockdev* dev, uint64_t offset, void* entry);

ssize_t write_entry(blockdev* dev, uint64_t offset, void* entry);
#endif
