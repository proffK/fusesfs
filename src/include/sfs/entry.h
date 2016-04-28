#ifndef _SFS_ENTRY_
#define _SFS_ENTRY_
#include <sfs/defs.h>

#define INDEX_ENTRY_SIZE 64

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
} entry __attribute__((packed));

typedef struct {
        uint8_t entry_type; 
        uint8_t reserved[3]; 
        uint64_t time_stamp;
        uint8_t vol_label[52];
} vol_ident_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint64_t del_start;
        uint8_t reserved[55];
} start_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint8_t reserved[63];
} unused_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint8_t dir_name[54];
} dir_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint64_t start_block;
        uint64_t end_block;
        uint64_t size;
        uint8_t name[30];
} file_entry __attribute__((packed));


typedef struct {
        uint8_t entry_type;
        uint8_t unused[9];
        uint64_t start_block;
        uint64_t end_block;
        uint8_t reserved[38];
} unusable_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint8_t dir_name[54];
} del_dir_entry __attribute__((packed));

typedef struct {
        uint8_t entry_type;
        uint8_t cont_entries;
        uint64_t time_stamp;
        uint64_t start_block;
        uint64_t end_block;
        uint64_t size;
        uint8_t name[30];
} del_file_entry __attribute__((packed));

typedef struct {
        uint8_t name[64];
} cont_entry __attribute__((packed));

#endif 
