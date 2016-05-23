#include <sfs/fsutils.h>
#include <sfs/defs.h>
#include <sfs/utils.h>
#include <sfs/entry.h>
#include <sfs/alloc.h>
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

        //SFS_TRACE("Cheking file %s in entry %lu", (char*) data, entry_off);
        cur = (char*) ((file_entry*) entr)->name;

        if (*curin == '\0') { 
                if (*cur != '\0') 
                        return 0;
                return 1;
        }

        ce = ((file_entry*) entr)->cont_entries;
        c = FIRST_FILE_NAME_SIZE;
        while (*curin != '\0' && c != 0) {
                if (*cur != *curin) return 0;
                c--;
                cur++;
                curin++;
        }

        if (*curin == '\0' && c != 0) {
                if (*cur == '\0')
                        return 1;
                return 0;
        }

        if (*curin == '\0' && ce == 0) { 
                SFS_TRACE("########## %d off %lu", *cur, entry_off);
                if (*cur != '\0') 
                        return 0;
                return 1;
        }

        if (ce == 0)
                return 0;

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

        cur = (char*) ((dir_entry*) entr)->dir_name;
        ce = ((dir_entry*) entr)->cont_entries;
        c = FIRST_DIR_NAME_SIZE;

        if (*curin == '\0') { 
                if (*cur != '\0') 
                        return 0;
                return 1;
        }
        //SFS_TRACE("Cheking dir %s off %lu curin %d cur %d", (char*) data, entry_off, *curin, *cur);
        while (*curin != '\0' && c != 0) {
                if (*curin != *cur) return 0;
                c--;
                cur++;
                curin++;
        }

        if (*curin == '\0' && c != 0) {
                if (*cur == '\0')
                        return 1;
                return 0;
        }

        if (*curin == '\0' && ce == 0) { 
                SFS_TRACE("########## %d off %lu", *cur, entry_off);
                if (*cur != '\0') 
                        return 0;
                return 1;
        }

        if (ce == 0) 
                return 0;

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
#define AS_FILE(entr) ((file_entry*) entr)
#define AS_DFILE(entr) ((del_file_entry*) entr)

static off_t del_next(sfs_unit* fs, entry* entr)
{
        off_t ret = NEXT_DEL(entr);
        if (ret == 0) {
                return 0;
        }
        if (read_entry(fs->bdev, ret, entr) == -1) {
                return (off_t) -1;
        }
        return ret;
}

static int del_file_list_remove(sfs_unit* fs, off_t off, entry* entr)
{
        off_t next = 0;
        off_t prev = 0;
        read_entry(fs->bdev, off, entr);
        prev = PREV_DEL(entr);
        next = NEXT_DEL(entr);

        if (prev != 0) {
                read_entry(fs->bdev, prev, entr);
                NEXT_DEL(entr) = next;
                write_entry(fs->bdev, prev, entr);
        } else {
                fs->del_begin = next;
        }
        if (next != 0) {
                read_entry(fs->bdev, next, entr);
                PREV_DEL(entr) = prev;
                write_entry(fs->bdev, next, entr);
        } 
        free_entry(fs, entr, off, 1);

        return 0;
}

int del_file_list_add(sfs_unit* fs, entry* entr, uint64_t start, uint64_t end)
{
        off_t next = 0;
        off_t prev = 0;
        off_t off = 0;
        uint64_t watchdog = 1 + (fs->vol_ident - fs->entry_start) 
                                / INDEX_ENTRY_SIZE;

        if (start == 0 && end == 0) {
                return 0;
        }

        SFS_TRACE("Add %lu %lu in deleted file list", start, end);
        read_entry(fs->bdev, fs->del_begin, entr);
        off = fs->del_begin;

        while (off != 0 && watchdog--) {
                SFS_TRACE("Del list elem:"
                                "Offset:  %lu\n"
                                "Start:   %lu\n"
                                "End:     %lu\n"
                                "Prev:    %lu\n"
                                "Next:    %lu\n",
                                off, 
                                AS_DFILE(entr)->start_block,
                                AS_DFILE(entr)->end_block,
                                PREV_DEL(entr),
                                NEXT_DEL(entr));

                if (AS_DFILE(entr)->end_block + 1 == start) {
                        SFS_TRACE("Prev %lu was found", off);
                        prev = off;
                        off = del_next(fs, entr);
                        continue;
                }
                if (AS_DFILE(entr)->start_block - 1 == end) {
                        SFS_TRACE("Next %lu was found", off);
                        next = off;
                }
                if ((off = del_next(fs, entr)) == (off_t) -1) {
                        return -1;
                }
                if (prev != 0 && next != 0)
                        break;
        }
        if (watchdog == 0) return -1;

        if (next != 0) {
                read_entry(fs->bdev, next, entr);
                AS_DFILE(entr)->start_block = start;
                if (prev != 0) {
                        off_t prev_start = 0;
                        read_entry(fs->bdev, prev, entr);
                        prev_start = AS_DFILE(entr)->start_block;
                        del_file_list_remove(fs, prev, entr);
                        read_entry(fs->bdev, next, entr);
                        AS_DFILE(entr)->start_block = prev_start;
                        write_entry(fs->bdev, next, entr);
                        return 0;
                } else {
                        write_entry(fs->bdev, fs->del_begin, entr);
                }
                return 0;
        } else if (prev != 0) {
                read_entry(fs->bdev, prev, entr);
                AS_DFILE(entr)->end_block = end;
                write_entry(fs->bdev, prev, entr);
                return 0;
        } else {
                off_t new = alloc_entry(fs, entr, 1);
                if (new == 0) {
                        SET_ERRNO(ENOMEM);
                        return -1;
                }
                AS_DFILE(entr)->entry_type = DEL_FILE_ENTRY;
                AS_DFILE(entr)->cont_entries = 0;
                PREV_DEL(entr) = 0;
                NEXT_DEL(entr) = fs->del_begin;
                AS_DFILE(entr)->start_block = start;
                AS_DFILE(entr)->end_block = end;
                strcpy((char*) AS_DFILE(entr)->name, "free"); 
                // TODO: start and end block in free name;
                write_entry(fs->bdev, new, entr);
                SFS_TRACE("Allocated entry:"
                                "Offset:  %lu\n"
                                "Start:   %lu\n"
                                "End:     %lu\n"
                                "Prev:    %lu\n"
                                "Next:    %lu\n",
                                new, 
                                AS_DFILE(entr)->start_block,
                                AS_DFILE(entr)->end_block,
                                PREV_DEL(entr),
                                NEXT_DEL(entr));

                if (off != 0) {
                        read_entry(fs->bdev, fs->del_begin, entr);
                        PREV_DEL(entr) = new;
                        write_entry(fs->bdev, fs->del_begin, entr);
                }
                fs->del_begin = new;
        }
        return 0;
}

static inline size_t get_size(sfs_unit* fs, entry* entr) 
{
        return fs->bdev->block_size * ((AS_DFILE(entr)->end_block) - 
                AS_DFILE(entr)->start_block + 1);
}

off_t del_file_list_alloc(sfs_unit* fs, entry* entr, size_t size)
{
        size_t cur_max = 0;
        off_t off = fs->del_begin;
        off_t cur_max_entr = off;
        uint64_t watchdog = 1 + (fs->vol_ident - fs->entry_start) 
                                / INDEX_ENTRY_SIZE;
        size_t real_size = get_real_size(fs, size);

        while (off != 0 && watchdog--) {
                size_t cur_size = 0;
                if (read_entry(fs->bdev, off, entr) == 0) {
                        SFS_TRACE("Can't read");
                        return 0;
                }
                if ((cur_size = get_size(fs, entr)) > cur_max) {
                        cur_max_entr = off;
                        cur_max = cur_size;
                }
                off = del_next(fs, entr);
        }
        if (watchdog == 0) return 0;

        SFS_TRACE("Real size %lu CURMAX %lu", real_size, cur_max);
        if (real_size > cur_max) {
                SET_ERRNO(ENOSPC);
                return 0;
        } else if (real_size == cur_max) {
                off_t ret = 0;
                read_entry(fs->bdev, cur_max_entr, entr);
                ret = AS_DFILE(entr)->start_block;
                del_file_list_remove(fs, cur_max_entr, entr);
                return ret;
        } else {
                off_t ret = 0;
                read_entry(fs->bdev, cur_max_entr, entr);
                ret = AS_DFILE(entr)->start_block;
                AS_DFILE(entr)->start_block += 
                                (real_size / fs->bdev->block_size);
                SFS_TRACE("New start block %lu in entry %lu", 
                                AS_DFILE(entr)->start_block,
                                cur_max_entr);
                write_entry(fs->bdev, cur_max_entr, entr);
                return ret;
        }
                
        return 0;
}

int try_expand(sfs_unit* fs, off_t off, size_t new_size, entry* entr)
{
        size_t delta_block = 0;
        off_t end = 0;
        read_entry(fs->bdev, off, entr);
        end = AS_FILE(entr)->end_block;
        uint64_t watchdog = 1 + (fs->vol_ident - fs->entry_start) 
                                / INDEX_ENTRY_SIZE;
        delta_block = get_real_size(fs, new_size) - 
                      get_real_size(fs, AS_FILE(entr)->size);
        off = fs->del_begin;
        SFS_TRACE("TRY EXPAND####################");

        while (off != 0 && watchdog--) {
                if (read_entry(fs->bdev, off, entr) == 0) {
                        SFS_TRACE("Can't read");
                        return -1;
                }
                if ((AS_DFILE(entr)->start_block == (end + 1)) &&
                    (get_size(fs, entr) >= delta_block)) {
                        SFS_TRACE("Donor FOUUUUUND %lu", off);
                        break;
                }
                off = del_next(fs, entr);
        }
        if (watchdog == 0) return -1;
        if (off == 0) return 1;

        if (get_size(fs, entr) == delta_block) {
                del_file_list_remove(fs, off, entr);
                return 0;
        }
       
        if (get_size(fs, entr) > delta_block) {
                AS_DFILE(entr)->start_block += (delta_block 
                                                / fs->bdev->block_size);
                SFS_TRACE("!!!!!!!!!!! %lu", AS_DFILE(entr)->start_block);
                write_entry(fs->bdev, off, entr);
                return 0;
        }
        return 0;
}

int file_shrink(sfs_unit* fs, off_t off, size_t new_size, entry* entr)
{
        size_t delta_block = 0;
        off_t end = 0;
        read_entry(fs->bdev, off, entr);
        end = AS_FILE(entr)->end_block;
        delta_block = get_real_size(fs, AS_FILE(entr)->size) - 
                      get_real_size(fs, new_size);
        
        if (delta_block == 0) 
                return 0;

        del_file_list_add(fs, entr, end - delta_block + 1, end); 
        return 0;
}
