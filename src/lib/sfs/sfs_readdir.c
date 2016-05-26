/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev>

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
        int add = 1; 
        size_t len = iter->len;
        size_t start_len = 0;
        char* name = iter->filename;
        off_t off = iter->cur_off;

        if (is_correct_filepath(name) != 0) {
                SFS_TRACE("Incorrect dirname %s", name);
                SET_ERRNO(EINVAL);
                return -1;
        }

        if (search_dir(fs, name, &entr) == 0) {
                SFS_TRACE("Dir %s not exist.", name);
                SET_ERRNO(ENOENT);
                return -1;
        }

        if (iter->cur_off == 0) 
                off = fs->entry_start;

        start_len = strnlen(iter->filename, len);
        if (start_len == 0) add = 0;

RESTART:
        if (*name != '\0') {
                name[start_len] = '/';
                name[start_len + add] = '\0';
        }
        SFS_TRACE("Try to search file mask %s", name);
        off = search_file_mask(fs, name, &entr, off);

        if (off == 0) {
                iter->filename = NULL;
                return 0;
        }

        if (entr.entry_type == FILE_ENTRY) {
                iter->size = ((file_entry*) &entr)->size;
                iter->time = ((file_entry*) &entr)->time_stamp;
                if (read_file_name(fs, (file_entry*) &entr, 
                                   off, name, len) == -1)
                        return -1;
                if (strlen(name) == start_len) {
                        off += INDEX_ENTRY_SIZE;
                        goto RESTART;
                }
                name += (start_len + add);
                if (is_leaf(name) == 0) {
                        off += INDEX_ENTRY_SIZE;
                        name -= (start_len + add);
                        goto RESTART;
                }
                name -= (start_len + add);
                iter->type = FILE_ITER_TYPE;
        }

        if (entr.entry_type == DIR_ENTRY) {
                iter->size = 0;
                iter->time = ((dir_entry*) &entr)->time_stamp;
                if (read_dir_name(fs, (dir_entry*) &entr, 
                                   off, name, len) == -1)
                        return -1;
                if (strlen(name) == start_len) {
                        off += INDEX_ENTRY_SIZE;
                        goto RESTART;
                }
                name += (start_len + add);
                if (is_leaf(name) == 0) {
                        off += INDEX_ENTRY_SIZE;
                        name -= (start_len + add);
                        goto RESTART;
                }
                name -= (start_len + add);
                iter->type = DIR_ITER_TYPE;
        }
        iter->cur_off = off + INDEX_ENTRY_SIZE;

        if (is_correct_filepath(name) != 0) {
                SFS_TRACE("Incorrect name of file was found: %s", name);
                off += INDEX_ENTRY_SIZE;
                goto RESTART;
        }
        
        SFS_TRACE("searched file mask %s", name);
        SET_ERRNO(0);
        return 0;
}
