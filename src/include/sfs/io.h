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
#endif
