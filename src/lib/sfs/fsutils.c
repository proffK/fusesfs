#include <sfs/fsutils.h>
#include <sfs/defs.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/debug.h>
#include <sfs/callback.h>

static int check_file_mask(sfs_unit* fs, entry* entr, 
                           off_t entry_off, void* data)
{
        char* curin = (char*) data;
        char* cur = NULL;
        int c = 0;

        if (entr->entry_type != FILE_ENTRY && entr->entry_type != DIR_ENTRY) 
                return 0;

        SFS_TRACE("Cheking mask %s", (char*) data);

        if (*curin == '\0') {
                return 1;
        }

        if (entr->entry_type == FILE_ENTRY) {
                cur = (char*) ((file_entry*) entr)->name;
                c = FIRST_FILE_NAME_SIZE;
                while (*curin != '\0' && c != 0) {
                        if (*cur != *curin) return 0;
                        c--;
                        cur++;
                        curin++;
                }
        }

        if (entr->entry_type == DIR_ENTRY) {
                cur = (char*) ((dir_entry*) entr)->dir_name;
                c = FIRST_DIR_NAME_SIZE;
                while (*curin != '\0' && c != 0) {
                        if (*cur != *curin) return 0;
                        c--;
                        cur++;
                        curin++;
                }
        }

        entry_off += INDEX_ENTRY_SIZE;
        read_entry(fs->bdev, entry_off, entr);
        c = INDEX_ENTRY_SIZE;
        cur = (char*) entr;
        while (*curin != '\0' && c != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
                if (c == 0) {
                        entry_off += INDEX_ENTRY_SIZE;
                        read_entry(fs->bdev, entry_off, entr);
                        c = INDEX_ENTRY_SIZE;
                        cur = (char*) entr;
                }

        }

        SFS_TRACE("Found mask %s", (char*) data);
        return 1;
}

static int read_file_entry(sfs_unit* fs, entry* entr, 
                           off_t entry_off, void* data)
{
        char* curin = (char*) data;
        char* cur = NULL;
        off_t start = entry_off;
        int ce = 0;
        int c = 0;

        if (entr->entry_type != FILE_ENTRY) 
                return 0;

        SFS_TRACE("Cheking file %s in entry %lu", (char*) data, entry_off);

        if (*curin == '\0') {
                return 1;
        }

        cur = (char*) ((file_entry*) entr)->name;
        ce = ((file_entry*) entr)->cont_entries;
        c = FIRST_FILE_NAME_SIZE;
        while (*curin != '\0' && c != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
        }

        if (*curin == '\0' && *cur == '\0') {
                return 1;
        }

        entry_off += INDEX_ENTRY_SIZE;
        if (read_entry(fs->bdev, entry_off, entr) == 0) {
                SFS_TRACE("End of index area");
                return 0;
        }
        c = INDEX_ENTRY_SIZE;
        cur = (char*) entr;
        while ((*cur != '\0' && *curin != '\0') && ce != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
                if (c == 0) {
                        entry_off += INDEX_ENTRY_SIZE;
                        if (read_entry(fs->bdev, entry_off, entr) == 0) {
                                SFS_TRACE("End of index area");
                                return 0;
                        }
                        c = INDEX_ENTRY_SIZE;
                        ce--;
                        cur = (char*) entr;
                }
        }
        if (*cur != *curin) return 0;

        SFS_TRACE("Found file %s", (char*) data);
        if (read_entry(fs->bdev, start, entr) == 0) {
                SFS_TRACE("Read error");
                return 0;
        }
        return 1;
}

static int read_dir_entry(sfs_unit* fs, entry* entr, 
                          off_t entry_off, void* data)
{
        char* curin = (char*) data;
        off_t start = entry_off;
        char* cur = NULL;
        int c = 0;
        int ce = 0;

        if (entr->entry_type != DIR_ENTRY) 
                return 0;

        SFS_TRACE("Cheking dir %s", (char*) data);

        if (*curin == '\0') {
                return 1;
        }

        cur = (char*) ((dir_entry*) entr)->dir_name;
        ce = ((dir_entry*) entr)->cont_entries;
        c = FIRST_DIR_NAME_SIZE;
        while (*curin != '\0' && c != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
        }

        if (*curin == '\0' && *cur == '\0') {
                return 1;
        }

        entry_off += INDEX_ENTRY_SIZE;
        if (read_entry(fs->bdev, entry_off, entr) == 0) {
                SFS_TRACE("End of index area");
                return 0;
        }
        c = INDEX_ENTRY_SIZE;
        cur = (char*) entr;
        while ((*cur != '\0' && *curin != '\0') && ce != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
                if (c == 0) {
                        entry_off += INDEX_ENTRY_SIZE;
                        if (read_entry(fs->bdev, entry_off, entr) == 0) {
                                SFS_TRACE("End of index area");
                                return 0;
                        }
                        c = INDEX_ENTRY_SIZE;
                        cur = (char*) entr;
                        ce--;
                }
        }
        if (*cur != *curin) return 0;

        SFS_TRACE("Found dir %s", (char*) data);
        if (read_entry(fs->bdev, start, entr) == 0) {
                SFS_TRACE("Read error");
                return 0;
        }
         return 1;
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

                filepath += INDEX_ENTRY_SIZE;
                cur_off += INDEX_ENTRY_SIZE;
        }
        read_entry(fs->bdev, start_off, entr);

        return start_off;
}

off_t search_file_mask(sfs_unit* fs, char* filepath, 
                       entry* entr, off_t entr_off)
{
        SET_ERRNO(0);

        if (!(fs && filepath && entr)) {
                SET_ERRNO(EFAULT);
                return 0;
        }

        while (entr_off != fs->vol_ident) {
                read_entry(fs->bdev, entr_off, entr);
                //SFS_TRACE("Reading entry %lu", entr_off);
                if (entr->entry_type == FILE_ENTRY) {
                        if (check_file_mask(fs, entr, 
                                            entr_off, filepath) != 0) {
                                read_entry(fs->bdev, entr_off, entr);
                                return entr_off;
                        } else {
                                entr_off += INDEX_ENTRY_SIZE;
                        }
                } else if (entr->entry_type == DIR_ENTRY) {
                        if (check_file_mask(fs, entr, 
                                            entr_off, filepath) != 0) {
                                read_entry(fs->bdev, entr_off, entr);
                                return entr_off;
                        } else {
                                entr_off += INDEX_ENTRY_SIZE;
                        }
                } else {
                        entr_off += INDEX_ENTRY_SIZE;
                }
        }

        return 0;
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

                filepath += INDEX_ENTRY_SIZE;
                cur_off += INDEX_ENTRY_SIZE;
        }
        read_entry(fs->bdev, start_off, entr);

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
                                *cur = '/';
                                return -1;
                        }
                        *cur = '/';
                }
                cur++;
        }

        return 0;
}

int read_file_name(sfs_unit* fs, file_entry* entr, 
                   off_t file_off, char* str, size_t len)
{
        char* curin = (char*) entr->name;
        char* curout = str;
        int curpos = FIRST_FILE_NAME_SIZE;
        SFS_TRACE("Reading filename with len %lu", len);

        while (len > 1 && *curin != '\0') {
                *curout = *curin;
                curin++;
                curout++;
                len--;
                curpos--;
                if (curpos == 0) {
                        file_off += INDEX_ENTRY_SIZE;
                        read_entry(fs->bdev, file_off, (entry*) entr);
                        curin = (char*) entr;
                        curpos = INDEX_ENTRY_SIZE;
                }
        }

        if (len == 1 && curin == '\0') {
                *curout = '\0';
                return 0;
        }

        if (len == 1) {
                SET_ERRNO(ENOMEM);
                return -1;
        }

        *curout = '\0';
 
        SFS_TRACE("Read filename %s", str);
        return 0;
}

int read_dir_name(sfs_unit* fs, dir_entry* entr, 
                  off_t dir_off, char* str, size_t len)
{
        char* curin = (char*) entr->dir_name;
        char* curout = str;
        int curpos = FIRST_DIR_NAME_SIZE;
        SFS_TRACE("Reading dirname with len %lu", len);

        while (len > 1 && *curin != '\0') {
                *curout = *curin;
                curin++;
                curout++;
                len--;
                curpos--;
                if (curpos == 0) {
                        dir_off += INDEX_ENTRY_SIZE;
                        read_entry(fs->bdev, dir_off, (entry*) entr);
                        curin = (char*) entr;
                        curpos = INDEX_ENTRY_SIZE;
                }
        }

        if (len == 1 && curin == '\0') {
                *curout = '\0';
                return 0;
        }

        if (len == 1) {
                SET_ERRNO(ENOMEM);
                return -1;
        }

        *curout = '\0';
        SFS_TRACE("Succesfully read dirname %s", str);

        return 0;
}
