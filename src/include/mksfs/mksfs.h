#ifndef MKSFS_H
#define MKSFS_H

#define VOLUME_NAME_SIZE (ssize_t)52
#define DEFAULT_BLOCK_SIZE (ssize_t)512
#define DEFAULT_MIN_BLOCK (ssize_t)256
#define DEFAULT_METADATA_PERCENT 5UL
#define FS_ENTRY_SIZE (ssize_t)64
#define MBR_SIZE (ssize_t)512
#define MIN_NUM_ENTRY 2
#define METADATA_MIN_SIZE MIN_NUM_ENTRY*FS_ENTRY_SIZE 

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
        unsigned md_size;
        unsigned block_size;
        char label[VOLUME_NAME_SIZE];
        char* file_name;
};


#endif
