#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

static int is_leaf(char* path) {
        int i = 0;

        SFS_TRACE("Check leaf %s", path);
        while (path[i] != '\0') {
                if (path[i] == '/') return 0;
                i++;
        }
        SFS_TRACE("Checked leaf %s", path);
        return 1;
}

int sfs_readdir(sfs_unit* fs, diriter* iter)
{
        entry entr;
        size_t len = iter->len;
        size_t start_len = 0;
        char* name = iter->filename;
        off_t off = iter->cur_off;

        if (is_correct_filepath(name) != 0) {
                SFS_TRACE("Incorrect dirname %s", name);
                return -1;
        }

        if (search_dir(fs, name, &entr) == 0) {
                SFS_TRACE("Dir %s not exist.", name);
                return -1;
        }

        if (iter->cur_off == 0) 
                off = fs->entry_start;

        start_len = strnlen(iter->filename, len);

RESTART:
        name[start_len + 1] = '\0';
        SFS_TRACE("Try to search file mask %s", name);
        off = search_file_mask(fs, name, &entr, off);

        if (off == 0) {
                iter->filename = NULL;
                return 0;
        }

        if (entr.entry_type == FILE_ENTRY) {
                if (read_file_name(fs, (file_entry*) &entr, 
                                   off, name, len) == -1)
                        return -1;
                if (strlen(name) == start_len) {
                        off += INDEX_ENTRY_SIZE;
                        goto RESTART;
                }
                name += (start_len + 1);
                if (is_leaf(name) == 0) {
                        off += INDEX_ENTRY_SIZE;
                        name -= (start_len + 1);
                        goto RESTART;
                }
                name -= (start_len + 1);
                iter->type = FILE_ITER_TYPE;
        }

        if (entr.entry_type == DIR_ENTRY) {
                if (read_dir_name(fs, (dir_entry*) &entr, 
                                   off, name, len) == -1)
                        return -1;
                if (strlen(name) == start_len) {
                        off += INDEX_ENTRY_SIZE;
                        goto RESTART;
                }
                name += (start_len + 1);
                if (is_leaf(name) == 0) {
                        off += INDEX_ENTRY_SIZE;
                        name -= (start_len + 1);
                        goto RESTART;
                }
                name -= (start_len + 1);
                iter->type = DIR_ITER_TYPE;
        }
        iter->cur_off = off + INDEX_ENTRY_SIZE;

        SFS_TRACE("searched file mask %s", name);
        return 0;
}
