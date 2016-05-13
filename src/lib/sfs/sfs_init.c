#include <sfs/unit.h>
#include <bdev/blockdev.h>
#include <sfs/mbr.h>
#include <sfs/utils.h>
#include <sfs/debug.h>
#include <sfs/io.h>
#include <sfs/entry.h>

int sfs_init(sfs_unit* fs, blockdev* bdev, time_t last_w_time)
{
        // TODO: check checksum
        // TODO: release list with deleted file
        // TODO: check timestamp
        uint64_t data_area_size = 0;
        uint64_t index_area_size = 0;
        uint8_t magic_number[3];
        uint8_t sfs_v;
        uint64_t total_size = 0;
        uint32_t reserved_size = 0;
        uint8_t b_size_p2 = 0;
        uint64_t block_size = 128;
        uint8_t checksum = 0;
        uint64_t new_timestamp = 0;
        uint8_t zero_folder_entry = 0;
        SFS_TRACE("Read MBR fields from image");
        /* Read data_area_size */ 
        if (read_data(bdev, offsetof(struct mbr_t, data_area_size), 
                  (uint8_t*)(&data_area_size), sizeof(data_area_size)) == -1)
                return -1;
        /* Read index area size */
        if (read_data(bdev, offsetof(struct mbr_t, index_area_size), 
                  (uint8_t*)(&index_area_size), sizeof(index_area_size)) == -1)
                return -1;
        /* Read reserved area size */
        if (read_data(bdev, offsetof(struct mbr_t, reserved_size), 
                  (uint8_t*)(&reserved_size), sizeof(uint32_t)) == -1)
                return -1;
        /* Read magic number */
        if (read_data(bdev, offsetof(struct mbr_t, magic_number),
                  (uint8_t*)(magic_number), 3) == -1)
                return -1;
        /* Read sfs_version */
        if (read_data(bdev, offsetof(struct mbr_t, sfs_version),
                  (uint8_t*)(&sfs_v), sizeof(sfs_v)) == -1)
                return -1;
        /* Read checksum */
        if (read_data(bdev, offsetof(struct mbr_t, checksum),
                  (uint8_t*)(&checksum), sizeof(checksum)) == -1)
                return -1;
        /* Read total size */
        if (read_data(bdev, offsetof(struct mbr_t, total_size), 
                  (uint8_t*)(&total_size), sizeof(total_size)) == -1)
                return -1;
        /* Read block size */
        if (read_data(bdev, offsetof(struct mbr_t, block_size),
                  (uint8_t*)(&b_size_p2), sizeof(b_size_p2)) == -1)
                return -1;
        /* Check checksum :) */
        /* Do you speak GNU/Linux or Linux? You should respect Stallman */
        if (checksum != calc_checksum(magic_number, &sfs_v, &total_size,
                                      &reserved_size, &b_size_p2))              
                return -1;
        SFS_TRACE("Calculate all sizes");
        block_size <<= b_size_p2;
        SFS_TRACE("Block size: %lu", block_size);
        SFS_TRACE("RESERVED size: %u", reserved_size); 
        reserved_size *= block_size;
        data_area_size *= block_size;
        fs->entry_start = reserved_size + data_area_size;
        fs->vol_ident = fs->entry_start + index_area_size - INDEX_ENTRY_SIZE;
        fs->del_begin = 0;
        fs->bdev = bdev;
        /* Try to recognize zero folder */
        if (read_data(bdev, fs->entry_start + 2*INDEX_ENTRY_SIZE, 
                      &zero_folder_entry, sizeof(zero_folder_entry)) == -1)
                return -1;
        if (zero_folder_entry != DIR_ENTRY)
                return -1;
        /* Read last modification time */ 
        if (read_data(bdev, fs->entry_start + 2*INDEX_ENTRY_SIZE +
                      offsetof(dir_entry, dir_name), &zero_folder_entry,
                      sizeof(zero_folder_entry)) == -1)
                return -1;
        if (zero_folder_entry != 0)
                return -1; 
        /* Read timestamp */
        if (read_data(bdev, offsetof(vol_ident_entry, time_stamp) + 
                            fs->entry_start, (uint8_t*)&(fs->time), 
                            sizeof(fs->time)) == -1)
                return -1;
        SFS_TRACE("YO l%lu ft%lu", last_w_time, fs->time);
        if (last_w_time - fs->time > 1)
                return 1;
        /* Write zero to time stamp */
        if (write_data(bdev, offsetof(vol_ident_entry, time_stamp) + 
                             fs->entry_start, (uint8_t*)&(new_timestamp), 
                             sizeof(new_timestamp)) == -1) {
                return -1;
        }
        return 0;
}
