/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Edgar Kaziahmedov>

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

#ifndef MKSFS_H
#define MKSFS_H

#include <sfs/defs.h>
#include <sfs/entry.h>

#define VOLUME_NAME_SIZE (size_t)52
#define DEFAULT_BLOCK_SIZE (size_t)512
#define DEFAULT_MIN_BLOCK (size_t)256
#define DEFAULT_INDEX_PERCENT 5UL
#define MBR_SIZE (size_t)512
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
        time_t time_stamp;
        size_t data_size;
        size_t index_size;
        size_t total_block;
        size_t reserved_size;
        size_t block_size;
        char label[VOLUME_NAME_SIZE];
        char* file_name;
};

int image_create(struct sfs_options sfs_opts);

#endif
