/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev, Edgar Kaziahmedov>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SFS_MBR_
#define _SFS_MBR_
#include <stdint.h>

#define start_checksum 0x01AC
#define end_checksum   0x01BC
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
        uint8_t  checksum;
        uint8_t  reserved_part_table[66];
} __attribute__((packed));

#endif
