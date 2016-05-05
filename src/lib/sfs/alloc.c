#include <sfs/alloc.h>
#include <sfs/debug.h>
#include <sfs/entry.h>
#include <sfs/callback.h>

struct pair {
        int start;
        int cur;
};

#define CUR ((struct pair*) count)->cur
#define START ((struct pair*) count)->start

static int search_free_entry(sfs_unit* fs, entry* entr, off_t entry_off, void* count) 
{
        if (entr->entry_type != UNUSED_ENTRY) {
                if (CUR != START) 
                        CUR = START;
                return 0;
        }

        CUR--;
        if (CUR == 0) 
                return START;

        return 0;
}

#undef CUR
#undef START

off_t alloc_entry(sfs_unit* fs, entry* entr, int n)
{
        struct pair p;
        off_t ret = 0;

        if (n <= 0) {
                SFS_TRACE("Incorrect n: %d", n);
                return 0;
        }

        p.start = n;
        p.cur = n;

        if ((ret = entry_parse(fs, entr, search_free_entry, &p)) == 0) 
                return 0;

        ret -= INDEX_ENTRY_SIZE * (n - 1); 

        return ret;
}

static int free_one_entry(sfs_unit* fs, entry* entr, off_t entr_off)
{
        if (read_entry(fs->bdev, entr_off, entr) == -1) {
                return -1;
        }
        entr->entry_type = UNUSED_ENTRY;
        return 0;
}

int free_entry(sfs_unit* fs, entry* entr, off_t entr_off, int n)
{
        int i = 0;

        for (i = 0; i < n; ++i) {
                if (free_one_entry(fs, entr, entr_off) == -1)
                        return -1;
                entr_off += INDEX_ENTRY_SIZE;
        }
        return 0;
}

off_t alloc_space(sfs_unit* fs, size_t size)
{
        return 0; // TODO:
}

int free_space(sfs_unit* fs, off_t space)
{
        return 0; // TODO:
}
