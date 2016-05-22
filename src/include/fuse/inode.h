#ifndef INODE_H
#define INODE_H
#include <stdint.h>
#include <stddef.h>

typedef uint64_t vino_t;
typedef uint64_t pino_t;

#define INODE_MAP_DEFAULT_SIZE 1024
#define INODE_DEBUG

typedef struct {
        /* Physical inode */
        pino_t pino;
        int dirty;
} file_t;

typedef struct  {
        /* Max current virtual inode */
        vino_t max_vino;
        size_t c_size;
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
/*
 * Print vino info
 */
int vino_dump(vino_t vino);

/* 
 * Print inode map 
 */
int inode_map_dump(void);

#endif
