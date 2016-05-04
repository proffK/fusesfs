#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <mksfs/mksfs.h>
#include <sfs/mbr.h>
#include <sfs/entry.h>
#include <bdev/filedev.h>
#include <sfs/io.h>
#include <sfs/debug.h>

int image_create(struct sfs_options sfs_opts) 
{
        size_t block_size = pow(2, sfs_opts.block_size + BEGIN_POWER_OF_BS);
        /*
         * Print volume info 
         */
        fprintf(stdout, "\nTime stamp: %s"
                        "Data size: %lu blocks\n"
                        "Index size: %lu bytes\n"
                        "Total blocks: %lu blocks\n"
                        "Block size: %lu bytes\n"
                        "Label: %s\n"
                        "File name: %s\n", 
                        ctime(&(sfs_opts.time_stamp)), sfs_opts.data_size,
                        sfs_opts.index_size, sfs_opts.total_block,
                        block_size, sfs_opts.label,
                        sfs_opts.file_name); 
                              
        /*
         * Init file and device
         */
        uint8_t buffer = 0;
        uint64_t counter = 0;
        uint64_t end = 0;

        size_t i = 0;
        filedev_data fdev;
        blockdev bdev;
        struct mbr_t mbr_block;
        vol_ident_entry vol_entry;
        start_entry st_entry;
        unused_entry remain_area;
        del_file_entry del_entry;
        SFS_TRACE("Calculating aligned index area position.");
        size_t g_offset_start = sfs_opts.total_block * block_size - 
                                sfs_opts.index_size; 
        size_t g_offset_vol =   sfs_opts.total_block * block_size -
                                INDEX_ENTRY_SIZE;
        
        fdev.fd = -1;
        SFS_TRACE("Create file device.");
        if (filedev_create(&bdev, &fdev, block_size, 
                           sfs_opts.total_block * block_size) != &bdev)
                return -1;
        SFS_TRACE("Init block device.");
        fdev.filename = sfs_opts.file_name;
        if (bdev.init(&bdev) != 0)
                return -1;
        /*
         * Fill MBR block
         */
        SFS_TRACE("Filling MBR section. MBR size: %lu.", sizeof(struct mbr_t));
        memset(&mbr_block, 0, MBR_SIZE);
        mbr_block.time_stamp = sfs_opts.time_stamp;
        mbr_block.data_area_size = sfs_opts.data_size;
        mbr_block.index_area_size = sfs_opts.index_size;
        SFS_TRACE("INDEX size(bytes): %lu", sfs_opts.index_size);
        mbr_block.magic_number[0] = 'S';
        mbr_block.magic_number[1] = 'F';
        mbr_block.magic_number[2] = 'S';
        mbr_block.total_size = sfs_opts.total_block;
        mbr_block.reserved_size = sfs_opts.reserved_size;
        mbr_block.block_size = sfs_opts.block_size;
        counter = (uint64_t)&mbr_block.magic_number;
        end = (uint64_t)&mbr_block.checksum;
        for (; counter < end; counter++) 
                buffer += *(uint8_t*)counter;
        mbr_block.checksum = buffer; 
                
        if (write_data(&bdev, 0, (uint8_t*)(&mbr_block), MBR_SIZE) == -1)
                return -1;
        /*
         * Fill basic info in index area
         */
        SFS_TRACE("Filling INDEX area.");
        /* Fill Volume Identifier */
        memset(&vol_entry, 0, INDEX_ENTRY_SIZE);
        vol_entry.entry_type = VOL_IDENT;
        vol_entry.time_stamp = sfs_opts.time_stamp;
        memcpy(vol_entry.vol_label, sfs_opts.label, strlen(sfs_opts.label));
        if (write_data(&bdev, g_offset_vol, (uint8_t*)(&vol_entry), 
                       INDEX_ENTRY_SIZE) == -1) 
                return -1;
        /* Fill starting marker entry */
        memset(&st_entry, 0, INDEX_ENTRY_SIZE);
        st_entry.entry_type = START_ENTRY;
        if (write_data(&bdev, g_offset_start, (uint8_t*)(&st_entry),
                       INDEX_ENTRY_SIZE) == -1)
                return -1;
        /* Fill deleted file */
        memset(&del_entry, 0, INDEX_ENTRY_SIZE); 
        del_entry.entry_type = DEL_FILE_ENTRY;
        del_entry.cont_entries = 0;
        del_entry.time_stamp = (uint64_t)NULL;
        del_entry.start_block = mbr_block.reserved_size; 
        del_entry.end_block = mbr_block.reserved_size + 
                              mbr_block.block_size - 1;
        del_entry.size = (uint64_t)NULL;
        strncpy((char*)del_entry.name, "*free", 29);
        g_offset_start += INDEX_ENTRY_SIZE;
        if (write_data(&bdev, g_offset_start, (uint8_t*)(&del_entry),
                       INDEX_ENTRY_SIZE) == -1)
                return -1;
        /* Fill remainig area of unused entries */
        SFS_TRACE("Filling remain INDEX area with unused entries.");
        memset(&remain_area, 0, INDEX_ENTRY_SIZE);
        remain_area.entry_type = UNUSED_ENTRY;
        for (i = g_offset_start + INDEX_ENTRY_SIZE; 
             i < g_offset_vol; i += INDEX_ENTRY_SIZE) 
                if (write_data(&bdev, i, (uint8_t*)(&remain_area), 
                               INDEX_ENTRY_SIZE) == -1) 
                        return -1;
        bdev.release(&bdev); 
        return 0;
}
