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

#ifndef _SFS_ENTRY_
#define _SFS_ENTRY_
#include <sfs/defs.h>

#define MAX_DIR_PATH            16374
#define MAX_FILE_PATH           16350
#define MAX_VOL_LABEL           52
#define INDEX_ENTRY_SIZE        64

#define FIRST_FILE_NAME_SIZE 30
#define FIRST_DIR_NAME_SIZE 54

enum entry_type {
        VOL_IDENT      = 0x01,
        START_ENTRY    = 0x02,
        UNUSED_ENTRY   = 0x10,
        DIR_ENTRY      = 0x11,
        FILE_ENTRY     = 0x12,
        UNUSABLE_ENTRY = 0x18,
        DEL_DIR_ENTRY  = 0x19,
        DEL_FILE_ENTRY = 0x1A
};

typedef struct {
        uint8_t entry_type;
        uint8_t data[INDEX_ENTRY_SIZE - 1];
} __attribute__((packed)) entry;

typedef struct {
        uint8_t entry_type; 
        uint8_t reserved[3]; 
        uint64_t time_stamp;
        uint8_t vol_label[52];
} __attribute__((packed)) vol_ident_entry;

typedef struct {
        uint8_t entry_type;
        uint64_t del_start;
        uint8_t reserved[55];
} __attribute__((packed)) start_entry;

typedef struct {
        uint8_t entry_type;
        uint8_t reserved[63];
} __attribute__((packed)) unused_entry;

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint8_t dir_name[54];
} __attribute__((packed)) dir_entry;

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint64_t start_block;
        uint64_t end_block;
        uint64_t size;
        uint8_t name[30];
} __attribute__((packed)) file_entry;


typedef struct {
        uint8_t entry_type;
        uint8_t unused[9];
        uint64_t start_block;
        uint64_t end_block;
        uint8_t reserved[38];
} __attribute__((packed)) unusable_entry;

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint8_t dir_name[54];
} __attribute__((packed)) del_dir_entry;

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint64_t start_block;
        uint64_t end_block;
        uint64_t size;
        uint8_t name[30];
} __attribute__((packed)) del_file_entry;

typedef struct {
        uint8_t name[64];
} __attribute__((packed)) cont_entry;

#endif 
