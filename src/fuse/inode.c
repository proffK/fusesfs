#include <fuse/inode.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>

inode_map_t* inode_map = NULL;

static int inode_lock_init(vino_t vino) 
{
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        int ret = pthread_rwlock_init(&(inode_map->inode_table[vino].lock)
                                      , &attr); 
        return ret;
}

static int inode_lock_destroy(vino_t vino) 
{
        return pthread_rwlock_destroy(&(inode_map->inode_table[vino].lock)); 
}

static int inode_map_lock_init() 
{
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        int ret = pthread_rwlock_init(&(inode_map->lock), &attr); 
        return ret;
}

static int inode_map_lock_destroy() 
{
        return pthread_rwlock_destroy(&(inode_map->lock)); 
}

static int inode_map_resize(void) 
{ 
        file_t* buffer = inode_map->inode_table;
        inode_map->inode_table = mmap(NULL, 
                         (inode_map->c_size) * 2 * sizeof(file_t), 
                         PROT_READ|PROT_WRITE,
                         MAP_ANONYMOUS|MAP_SHARED|MAP_LOCKED, 0, 0);
        if (inode_map->inode_table == NULL) {
                inode_map->inode_table = buffer;
                return -1;
        }
        memcpy(inode_map->inode_table, buffer, inode_map->c_size);
        inode_map->c_size *= 2;
        return 0;                
}

int inode_map_create(void) 
{
        inode_map = mmap(NULL, sizeof(inode_map_t), PROT_READ|PROT_WRITE,
                         MAP_ANONYMOUS|MAP_SHARED|MAP_LOCKED, 0, 0);
        if (!(inode_map))
                return -1;
        inode_map->max_vino = 1;
        inode_map->inode_table = mmap(NULL, 
                         (sizeof(file_t) * INODE_MAP_DEFAULT_SIZE), 
                         PROT_READ|PROT_WRITE,
                         MAP_ANONYMOUS|MAP_SHARED, 0, 0);
        perror("");
        fprintf(stderr, "inode_map %p\n", inode_map);
        fprintf(stderr, "inode_map_table %p\n", inode_map->inode_table);
        if (inode_map->inode_table == (file_t*) -1) {
                munmap(inode_map, sizeof(inode_map_t));
                return -1;
        }
        inode_map->c_size = INODE_MAP_DEFAULT_SIZE;

        if (inode_map_lock_init() == -1) {
                munmap(inode_map->inode_table, 
                        sizeof(file_t) * inode_map->c_size);
                munmap(inode_map, sizeof(inode_map_t));
                return -1;
        }

        return 0;
}

int inode_map_delete(void) 
{
        long i = 0;
        for (i = 1; i < inode_map->max_vino; i++) {
                inode_lock_destroy(i);
        }
        inode_map_lock_destroy();
        munmap(inode_map->inode_table, 
                sizeof(file_t) * inode_map->c_size);
        munmap(inode_map, sizeof(inode_map_t));
        return 0;
}

vino_t pino_add(pino_t pino) 
{
        if (inode_map->max_vino == inode_map->c_size)
                if (inode_map_resize() != 0)
                        return 0;
        inode_map->inode_table[inode_map->max_vino].pino = pino;
        inode_map->inode_table[inode_map->max_vino].dirty = 0; 
        inode_map->inode_table[inode_map->max_vino].openbit = 0;
        if (inode_lock_init(inode_map->max_vino) == -1)
                return -1;

        (inode_map->max_vino)++;
        return inode_map->max_vino - 1;
}
       
vino_t get_vino(pino_t pino) 
{
        uint64_t i = 1;
        for (i = 1; i < inode_map->max_vino; i++) {
                if (inode_map->inode_table[i].pino == pino)
                        return i;
        }
        return 0;
}

int vino_dump(vino_t vino) 
{
#ifndef INODE_DEBUG
        if (vino >= inode_map->max_vino) 
                return -1;
#endif
#ifdef INODE_DEBUG
        if (vino >= inode_map->max_vino) {
                fprintf(stderr, "Virtual  inode: So big\n");
                return -1;
        } else
                fprintf(stderr, "Virtual  inode: %lu\n", vino);
        fprintf(stderr, "Physical inode: %lu\n", 
                        inode_map->inode_table[vino].pino);
        fprintf(stderr, "Dirty flag: %d\n\n",
                        inode_map->inode_table[vino].dirty);
#endif
        return 0;
}

int inode_map_dump(void) 
{
#ifdef INODE_DEBUG
        fprintf(stderr, "Maximal virtual inode: %lu\n",
                        inode_map->max_vino);
        fprintf(stderr, "Current inode map size: %lu\n",
                        inode_map->c_size);
        fprintf(stderr, "Inode table pointer: %p\n\n",
                        inode_map->inode_table);
#endif
        return 0;
}
