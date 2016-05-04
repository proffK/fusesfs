#include <sfs/unit.h>
#include <bdev/blockdev.h>
#include <sfs/mbr.h>
#include <sfs/utils.h>
#include <sfs/debug.h>
#include <sfs/io.h>
#include <sfs/entry.h>

int sfs_init(sfs_unit* fs, blockdev* bdev)
{
        // TODO: check checksum
        // TODO: release list with deleted file
        // TODO: check timestamp
        uint64_t data_area_size = 0;
        uint64_t index_area_size = 0;
        uint8_t magic_number[3];
        uint64_t total_size = 0;
        uint64_t reserved_size = 0;
        uint8_t b_size_p2 = 0;
        uint64_t block_size = 128;
        //uint8_t checksum = 0;
        uint64_t new_timestamp = 0;

        SFS_TRACE("Read MBR fields from image");
        /* Read data_area_size */ 
        read_data(bdev, offsetof(struct mbr_t, data_area_size), 
                  (uint8_t*)(&data_area_size), sizeof(data_area_size));
        /* Read index area size */
        read_data(bdev, offsetof(struct mbr_t, index_area_size), 
                  (uint8_t*)(&index_area_size), sizeof(index_area_size));
        /* Read magic number */
        read_data(bdev, offsetof(struct mbr_t, magic_number),
                  (uint8_t*)(magic_number), 3);
        /* Read total size */
        read_data(bdev, offsetof(struct mbr_t, total_size), 
                  (uint8_t*)(&total_size), sizeof(total_size));
        /* Read block size */
        read_data(bdev, offsetof(struct mbr_t, block_size),
                  (uint8_t*)(&block_size), sizeof(b_size_p2));

        block_size <<= b_size_p2;
        reserved_size *= block_size;
        data_area_size *= block_size;
        fs->entry_start = reserved_size + data_area_size;
        fs->vol_ident = fs->entry_start + index_area_size - INDEX_ENTRY_SIZE;
        fs->del_begin = 0;

        /* Read timestamp */
        read_data(bdev, offsetof(vol_ident_entry, time_stamp) + fs->entry_start,
                  (uint8_t*)&(fs->time), sizeof(fs->time));
        /* Write zero to time stamp */
        write_data(bdev, offsetof(vol_ident_entry, time_stamp) + 
                   fs->entry_start,
                   (uint8_t*)&(new_timestamp), sizeof(new_timestamp));

        return 0;
}
