#ifndef _SFS_IO_
#define _SFS_IO_
#include <stddef.h>
#include <stdint.h>
#include <bdev/blockdev.h>

size_t read_data(blockdev* dev, uint64_t offset, uint8_t* data, size_t size);

size_t write_data(blockdev* dev, uint64_t offset, uint8_t* data, size_t size);

size_t read_entry(blockdev* dev, uint64_t offset, uint8_t* entry);

size_t write_entry(blockdev* dev, uint64_t offset, uint8_t* entry);
#endif
