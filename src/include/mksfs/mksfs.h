#ifndef MKSFS_H
#define MKSFS_H

#define VOLUME_NAME_SIZE (ssize_t)52
#define DEFAULT_BLOCK_SIZE (ssize_t)512
#define DEFAULT_MIN_BLOCK (ssize_t)256
#define DEFAULT_INDEX_PERCENT 5UL
#define INDEX_ENTRY_SIZE (ssize_t)64
#define MBR_SIZE (ssize_t)512
#define MIN_NUM_ENTRY 2
#define BEGIN_POWER_OF_BS 7
#define INDEX_MIN_SIZE MIN_NUM_ENTRY*INDEX_ENTRY_SIZE 

enum exit_codes {
        EXIT_INPFILE = EXIT_FAILURE + 1,
        EXIT_NOMD = EXIT_FAILURE + 2,
        EXIT_MDLRG = EXIT_FAILURE + 3,
        EXIT_FILELRG = EXIT_FAILURE + 4,
        EXIT_MDSML = EXIT_FAILURE + 5,
        EXIT_NOBS = EXIT_FAILURE + 6,
        EXIT_BSLRG = EXIT_FAILURE + 7,
        EXIT_BSDIV = EXIT_FAILURE + 8,
        EXIT_BSDGR2 = EXIT_FAILURE + 9,
        EXIT_LBL = EXIT_FAILURE + 10
};

/*
 * Main struct, that describe options for creation 
 * a SFS file system
 */
struct sfs_options {
        size_t time_stamp;
        size_t data_size;
        size_t index_size;
        size_t total_block;
        size_t block_size;
        char label[VOLUME_NAME_SIZE];
        char* file_name;
};

int mksfs_image_create(struct sfs_options sfs_opts);

#endif
