#ifndef INODE_H
#define INODE_H
#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef uint64_t vino_t;
typedef uint64_t pino_t;

#define INODE_MAP_DEFAULT_SIZE 1024
#define INODE_DEBUG

typedef struct {
        /* Physical inode */
        pino_t pino;
        int dirty;
        int openbit;
        pthread_rwlock_t lock;
} file_t;

typedef struct  {
        /* Max current virtual inode */
        vino_t max_vino;
        size_t c_size;
        pthread_rwlock_t lock;
        file_t* inode_table;
} inode_map_t;

extern inode_map_t* inode_map;

/*
 * Create inode_map
 */
int inode_map_create(void);

/*
 * Delete inode_map
 */
int inode_map_delete(void);

/*
 * Add inode to inode_map
 */
vino_t pino_add(pino_t pino);

/*
 * Set dirty flag
 */
static inline int set_dirty(vino_t vino) 
{
        if (vino >= inode_map->max_vino) 
                return -1;
        inode_map->inode_table[vino].dirty = 1;
        return 0;
}

/*
 * Get dirty flag
 */
static inline int get_dirty(vino_t vino) 
{
        if (vino >= inode_map->max_vino)
                return -1;
        return inode_map->inode_table[vino].dirty;
}

/*
 * Get virtual inode
 */
vino_t get_vino(pino_t pino);

/*
 * Get physical inode
 */
static inline pino_t get_pino(vino_t vino) 
{
        if (vino >= inode_map->max_vino) 
                return 0;
        return inode_map->inode_table[vino].pino;
}

/*
 * Set physical inode
 */
static inline int set_pino(vino_t vino, pino_t new_pino) 
{
        if (vino >= inode_map->max_vino) 
                return -1;
        inode_map->inode_table[vino].pino = new_pino;
        return 0; 
}

static inline int set_openbit(vino_t vino)
{
        inode_map->inode_table[vino].openbit = 1;
        return 0;
}

static inline int clr_openbit(vino_t vino)
{
        inode_map->inode_table[vino].openbit = 0;
        return 0;
}

static inline int get_openbit(vino_t vino) {
        return inode_map->inode_table[vino].openbit;
}
/*
 * Print vino info
 */
int vino_dump(vino_t vino);

/* 
 * Print inode map 
 */
int inode_map_dump(void);

/*
 * RW lock for locking inode map
 */
static inline void inode_map_rdlock() 
{
        pthread_rwlock_rdlock(&(inode_map->lock));
}

static inline void inode_map_wrlock() 
{
        pthread_rwlock_wrlock(&(inode_map->lock));
}

static inline void inode_map_unlock() 
{
        pthread_rwlock_unlock(&(inode_map->lock));
}
/*
 * RW lock for locking inode
 */
static inline void inode_rdlock(vino_t vino) 
{
        pthread_rwlock_rdlock(&(inode_map->inode_table[vino].lock));
}

static inline void inode_wrlock(vino_t vino) 
{
        pthread_rwlock_wrlock(&(inode_map->inode_table[vino].lock));
}

static inline void inode_unlock(vino_t vino) 
{
        pthread_rwlock_unlock(&(inode_map->inode_table[vino].lock));
}

#endif
