#include <sfs/fsutils.h>
#include <sfs/defs.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/debug.h>
#include <sfs/callback.h>

static int read_file_entry(sfs_unit* fs, entry* entr, off_t entry_off, void* data)
{
        if (entr->entry_type != FILE_ENTRY) return 0;

        if (strncmp(data, (char*) ((file_entry*) entr)->name, 
                          FIRST_FILE_NAME_SIZE) == 0) {
                SFS_TRACE("Found file with offset %lu", entry_off);
                return 1;
        }

        return 0;

}

static int read_dir_entry(sfs_unit* fs, entry* entr, off_t entry_off, void* data)
{
        if (entr->entry_type != DIR_ENTRY) return 0;

        if (strncmp(data, (char*) ((dir_entry*) entr)->dir_name, 
                          FIRST_DIR_NAME_SIZE) == 0) {
                SFS_TRACE("Found directory with offset %lu", entry_off);
                return 1;
        }

        return 0;

}

off_t search_file(sfs_unit* fs, char* filepath, entry* entr)
{
        off_t start_off = 0;
        off_t cur_off = 0;
        uint8_t n = 0;

        SET_ERRNO(0);

        if (!(fs && filepath && entr)) {
                SET_ERRNO(EFAULT);
                return 0;
        }

        if ((start_off = entry_parse(fs, entr, 
                                    read_file_entry, filepath)) == 0) {
                SFS_TRACE("Didn't found file %s", filepath);
                return 0;
        }

        if ((n = ((file_entry*) entr)->cont_entries) == 0) {
                SFS_TRACE("Founded file %s", filepath);
                return start_off;
        }

        cur_off = start_off + INDEX_ENTRY_SIZE;
        filepath += FIRST_FILE_NAME_SIZE;

        while (n--) {
                if (read_entry(fs->bdev, cur_off, entr) == -1) {
                        SET_ERRNO(EIO);
                        return 0;
                }
                if (strncmp((char*) entr, filepath, INDEX_ENTRY_SIZE) != 0) {
                        SFS_TRACE("Didn't found file %s", filepath);
                        return 0;
                }

                filepath += FIRST_FILE_NAME_SIZE;
                cur_off += INDEX_ENTRY_SIZE;
        }

        return start_off;
}

off_t search_dir(sfs_unit* fs, char* filepath, entry* entr)
{
        off_t start_off = 0;
        off_t cur_off = 0;
        uint8_t n = 0;

        SET_ERRNO(0);

        if (!(fs && filepath && entr)) {
                SET_ERRNO(EFAULT);
                return 0;
        }

        if ((start_off = entry_parse(fs, entr, 
                                    read_dir_entry, filepath)) == 0) {
                SFS_TRACE("Didn't found directory %s", filepath);
                return 0;
        }


        if ((n = ((dir_entry*) entr)->cont_entries) == 0) {
                SFS_TRACE("Founded directory %s", filepath);
                return start_off;
        }

        cur_off = start_off + INDEX_ENTRY_SIZE;
        filepath += FIRST_DIR_NAME_SIZE;

        while (n--) {
                if (read_entry(fs->bdev, cur_off, entr) == -1) {
                        SET_ERRNO(EIO);
                        return 0;
                }
                if (strncmp((char*) entr, filepath, INDEX_ENTRY_SIZE) != 0) {
                        SFS_TRACE("Didn't found directory %s", filepath);
                        return 0;
                }

                filepath += FIRST_DIR_NAME_SIZE;
                cur_off += INDEX_ENTRY_SIZE;
        }

        return start_off;
}

int check_dirs(sfs_unit* fs, char* filepath, entry* entr)
{
        char* cur = filepath;
        size_t len = strlen(filepath) + 1;
        char* end = filepath + len;
        int ret = 0;

        SFS_TRACE("Check path %s", filepath);
        while (cur != end) {
                if (*cur == '/') {
                        *cur = '\0';
                        ret = search_dir(fs, filepath, entr);
                        if (ret == 0) {
                                SFS_TRACE("Directory in path %s not exist", 
                                                filepath);
                                return -1;
                        }
                        *cur = '/';
                }
                cur++;
        }

        return 0;
}
