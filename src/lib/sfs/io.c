#include <sfs/io.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/entry.h>
#include <bdev/blockdev.h>
#include <sfs/utils.h>

size_t read_data(blockdev* dev, off_t offset, uint8_t* data, size_t size)
{
        size_t bsize = dev->block_size;
        buf_t* buf = dev->buf;
        bnum_t bnum = offset / bsize;
        size_t cur_pos = offset % bsize;
        size_t ret_size = size;

        IO_TRACE("Reading data: \n"
                 "offset: %lu\n"
                 "data:   %p\n"
                 "size:   %lu\n", offset, data, size);

        if (dev->buf_num != bnum) {
                IO_TRACE("Read block: %lu\n", bnum);
                if (dev->read(dev, buf, bsize, bnum) == -1) {
                        IO_TRACE("Reading failed\n");
                        return -1;
                }

                dev->buf_num = bnum;
        }

        if (cur_pos + size <= bsize) {
                memcpy(data, buf + cur_pos, size);
                return ret_size;
        }

        memcpy(data, buf + cur_pos, bsize - cur_pos);
        size -= bsize - cur_pos;
        data += bsize + cur_pos;
        bnum++;

        while (size >= bsize) {
                IO_TRACE("Read block: %lu\n", bnum);
                if (dev->read(dev, buf, bsize, bnum) == -1) {
                        IO_TRACE("Reading failed\n");
                        return -1;
                }

                memcpy(data, buf, bsize);
                bnum++;
                size -= bsize;
                data += bsize;
        }
        if (size == 0) return ret_size;

        dev->buf_num = bnum;
        IO_TRACE("Read block: %lu", bnum);
        if (dev->read(dev, buf, bsize, bnum) == -1) {
                IO_TRACE("Reading failed\n");
                return -1;
        }

        memcpy(data, buf, size);
        return ret_size;
}
//#define dev->write(x,y,z,t) 0
size_t write_data(blockdev* dev, off_t offset, uint8_t* data, size_t size)
{
        size_t bsize = dev->block_size;
        buf_t* buf = dev->buf;
        bnum_t bnum = offset / bsize;
        size_t cur_pos = offset % bsize;
        size_t ret_size = size;

        IO_TRACE("Writing data: \n"
                 "offset: %lu\n"
                 "data:   %p\n"
                 "size:   %lu\n", offset, data, size);

        IO_TRACE("Read block: %lu", bnum);
        if (dev->read(dev, buf, bsize, bnum) == -1) {
                IO_TRACE("Reading failed\n");
                return -1;
        }
        dev->buf_num = bnum;

        if (cur_pos + size <= bsize) {
                memcpy(buf + cur_pos, data, size);
                IO_TRACE("Write block: %lu", bnum);
                if (dev->write(dev, buf, bsize, bnum) == -1) {
                        IO_TRACE("Writing failed\n");   
                        return -1;
                }
                dev->buf_num = -1;
                return ret_size;
        }

        memcpy(buf + cur_pos, data, bsize - cur_pos);
        IO_TRACE("Write block: %lu", bnum);
        if (dev->write(dev, buf, bsize, bnum) == -1) {
                IO_TRACE("Writing failed\n");   
                return -1;
        }
        size -= bsize - cur_pos;
        data += bsize - cur_pos;
        bnum++;

        while (size >= bsize) {
                memcpy(buf, data, bsize);
                IO_TRACE("Write block: %lu", bnum);
                if (dev->write(dev, buf, bsize, bnum) == -1) {
                        IO_TRACE("Writing failed\n");   
                        return -1;
                }
                bnum++;
                size -= bsize;
                data += bsize;
                dev->buf_num = -1;
        }

        if (size == 0) return ret_size;

        dev->buf_num = bnum;
        IO_TRACE("Read block: %lu", bnum);
        if (dev->read(dev, buf, bsize, bnum) == -1) {
                IO_TRACE("Reading failed\n");   
                return -1;
        }
        memcpy(buf, data, size);
        if (dev->write(dev, buf, bsize, bnum) == -1) {
                IO_TRACE("Writing failed\n");   
                return -1;
        }

        return ret_size;
}

size_t read_entry(blockdev* dev, off_t offset, entry* entry)
{
        IO_TRACE("Reading entry: \n"
                 "offset: %lu\n"
                 "data:   %p\n" , offset, entry);

        if (offset % INDEX_ENTRY_SIZE) {
                IO_TRACE("Offset isn't aligned\n");
                return -1;
        }

        return read_data(dev, offset, (uint8_t*)entry, INDEX_ENTRY_SIZE);
}

size_t write_entry(blockdev* dev, off_t offset, entry* entry)
{
        IO_TRACE("Writing entry: \n"
                 "offset: %lu\n"
                 "data:   %p\n" , offset, entry);

        if (offset % INDEX_ENTRY_SIZE) {
                IO_TRACE("Offset isn't aligned\n");
                return -1;
        }

        return write_data(dev, offset, (uint8_t*)entry, INDEX_ENTRY_SIZE);
}

