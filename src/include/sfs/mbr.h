#ifndef _SFS_MBR_
#define _SFS_MBR_
#include <stdint.h>

struct mbr_t {
        uint8_t  reserved_boot_code[404];
        uint64_t time_stamp;              /* Seconds since 1 Jan 1970 */
        uint64_t data_area_size;          /* In blocks */
        uint64_t index_area_size;         /* In bytes */
        uint8_t  magic_number[3];         /* 0x534653 (SFS in ASCII) */
        uint8_t  sfs_version;             
        uint64_t total_size;              /* In blocks */
        uint32_t reserved_size;           /* In blocks */
        uint8_t  block_size;              /* Bytes per block = 
                                             2 ^ (block_size + 7) */
        uint8_t  reserved_part_table[66];
};

#endif
