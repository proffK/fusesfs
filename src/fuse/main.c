#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#define FUSE_USE_VERSION 25
#include <fuse.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <sfs/unit.h>
#include <sfs/debug.h>
#include <sfs/statfs.h>
#include <sfs/fixfs.h>
#include <bdev/filedev.h>
#include <sfs/mbr.h>
#include <fuse/inode.h>

enum err_code {
        SFS_INVAL = 1,
        SFS_ENOMEM
};

extern int sfs_errno;
extern inode_map_t* inode_map;
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++ Semaphores ++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#define LOCK_IMAGE_SEM 0
#define SEM_NUM 1
#define ACCESS_FLAGS 0666
#define MAX_OPER_NUM 2
/*
 * Semaphores functions
 */
#define ADD_OPER(num, sem, oper, flags) do {  \
    sem_ops[num].sem_num = (sem);           \
    sem_ops[num].sem_op = (oper);           \
    sem_ops[num].sem_flg = (flags);         \
} while(0)

#define DO_OPER(num)                        \
    (semop(semaphores, sem_ops, (num))) 

/*
 * Operations under semaphores
 */
#define V   +1
#define P   -1
#define ZW   0

/*
* Create array of operations
*/ 
struct sembuf sem_ops[MAX_OPER_NUM];

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++ End of semaphores +++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#define NUM_OF_FUSE_OPTIONS 4
static char* imagefile = NULL;
sfs_unit* sfs_description = NULL;
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++ Functions +++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
static void usage(unsigned status) 
{
        if (status != EXIT_SUCCESS) 
                fprintf(stderr, "Try 'fusesfs -h' for more information.\n");
        else {
                printf("Usage: fusesfs [OPTIONS] IMAGE DIRECTORY\n");
                printf("\n"
                       "-h,        Print this screen            \n");
        }
        exit(status);
}

static int try_to_lock_image(char* path) 
{ 
        key_t key = ftok(path, 0);
        int semaphores = semget(key, SEM_NUM, IPC_CREAT | ACCESS_FLAGS); 

        /*
         * V(LOCK_IMAGE) if no zero the out
         */
        ADD_OPER(0, LOCK_IMAGE_SEM, ZW, IPC_NOWAIT);
        ADD_OPER(1, LOCK_IMAGE_SEM,  V, SEM_UNDO);

        if (DO_OPER(2) == -1)
               return -1; 
        return 0;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++ End of functions ++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
static inline char* new_path(const char* path) 
{
        char* cpath = (char*) calloc(PATH_MAX, sizeof(char));
        if (cpath == NULL) {
                perror("");
                return NULL;
        }
        strncpy(cpath, path, PATH_MAX - 1);
        cpath[PATH_MAX - 1] = '\0';
        return cpath;
}
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++ FUSE OPERATION STRUCTURE ++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
static void* fuse_sfs_init()
{
        SFS_TRACE("INIT");
        uint8_t bs_p2 = 0;
        uint64_t bs = 0;
        uint64_t file_size = 0;
        struct stat dstat;
        struct mbr_t mbr;
        entry entr;
        char Answer = '\0';
        size_t data_size = 0;
        size_t free_size = 0;
        size_t used_size = 0;
        blockdev* bdev = malloc(sizeof(blockdev));
        filedev_data* fdev = malloc(sizeof(filedev_data));
        sfs_description = malloc(sizeof(sfs_unit));
        /*
         * Try to recognize block_size
         */
        int temp_fd = open(imagefile, O_RDONLY);
        fstat(temp_fd, &dstat); 
        lseek(temp_fd, offsetof(struct mbr_t, block_size), SEEK_SET);
        if (read(temp_fd, &bs_p2, 1) == 0) {
                perror("");
                close(temp_fd);
                free(bdev->dev_data);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }
        bs = 128 << bs_p2;
        if (bs <= 128) {
                fprintf(stderr, "Block size isn't valid!\n");
                close(temp_fd);
                free(fdev);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }
        lseek(temp_fd, offsetof(struct mbr_t, total_size), SEEK_SET);
        if (read(temp_fd, &file_size, 8) == 0) {
                perror("");
                close(temp_fd);
                free(fdev);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }
        close(temp_fd);
        SFS_TRACE("BS: %lu, FS: %lu", bs, file_size);
        /*
         * Init fs
         */ 
        fdev->fd = -1;
        bdev = filedev_create(bdev, fdev, bs, file_size * bs);
        fdev->filename = imagefile;
        if (blockdev_init(bdev) != 0) {
                perror("");
                free(fdev);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }

        if (sfs_init(sfs_description, bdev) < 0) {
                fprintf(stderr, "The image was corrupted.\n");
                bdev->release(bdev);
                free(bdev->dev_data);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }

        if (dstat.st_mtime - sfs_description->time <= 1) {
                fprintf(stderr, "Modification time and timestamp are differ\n"
                                "WE DON'T GIVE ANY WARRANTY!\n");
                while (Answer != 'Y' && Answer != 'n') {
                        fprintf(stderr, "Do you want to continue?\n"
                                        "Press [y/N]\n"
                                        "WE DON'T GIVE ANY WARRANTY!\n");
                        Answer = getchar();
                }
                if (Answer == 'n') {
                        bdev->release(bdev);
                        free(bdev->dev_data);
                        free(bdev);
                        free(sfs_description);
                        exit(EXIT_FAILURE);
                }
        }
        else
                fprintf(stderr, "Modification time and timestamp "
                                "are equal\n");

        read_data(sfs_description->bdev, 0, (uint8_t*) &mbr, 
                  sizeof(struct mbr_t));

        data_size = mbr.data_area_size * sfs_description->bdev->block_size;
        free_size = scan_del_file_list(sfs_description, &entr);
        if (free_size == -1) {
                fprintf(stderr, "Allocation of free space was completly broken\n"
                                "Filesystem cannot be mounted\n");
                bdev->release(bdev);
                free(bdev->dev_data);
                free(bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }
 
        used_size = scan_used_space(sfs_description, &entr);
        
        if (data_size != free_size + used_size) {
                fprintf(stderr, "Allocation of free space was broken\n"
                                "free size = %lu\n"
                                "used size = %lu\n"
                                "data size = %lu\n", free_size, used_size,
                                                     data_size);
                while (Answer != 'y' && Answer != 'N') {
                        fprintf(stderr, "Do you want to continue?\n"
                                        "Press [y/N]\n"
                                        "WE DON'T GIVE ANY WARRANTY!\n");
                        Answer = getchar();
                }
                if (Answer == 'N') {
                        bdev->release(bdev);
                        free(bdev->dev_data);
                        free(bdev);
                        free(sfs_description);
                        exit(EXIT_FAILURE);
                }
        }
        fix_non_del_file(sfs_description, &entr);
 
        /* 
         * Create inode container
         */
        if (inode_map_create() == -1) {
                sfs_release(sfs_description);
                sfs_description->bdev->release(sfs_description->bdev);
                free(sfs_description->bdev->dev_data);
                free(sfs_description->bdev);
                free(sfs_description);
                exit(EXIT_FAILURE);
        }

        SFS_TRACE("Init finished");
        return NULL;
}

static void fuse_sfs_destroy(void* param)
{
        SFS_TRACE("%s", "");
        inode_map_delete();
        sfs_release(sfs_description);
        sfs_description->bdev->release(sfs_description->bdev);
        free(sfs_description->bdev->dev_data);
        free(sfs_description->bdev);
        free(sfs_description);
        
}

static int fuse_sfs_getattr(const char* path, struct stat *stbuf) 
{
        SFS_TRACE("GETATTR path %s", path);
        path++;
        char* cpath = new_path(path);
        vino_t vino;
        sfs_attr attr;
        pino_t pino;
        if ((pino = sfs_open(sfs_description, cpath)) == 0)
                return -sfs_errno;
        if ((vino = get_vino(pino)) == 0) {
                if ((vino = pino_add(pino)) == 0) {
                        free(cpath);
                        return -ENOMEM;
                }
        }
        if (sfs_getattr(sfs_description, pino, &attr) != 0)
                return -ENOENT;
        stbuf->st_dev  = getpid();
        stbuf->st_ino  = vino;
        stbuf->st_size = attr.size;
        stbuf->st_mtime = attr.time;
        if (attr.type == FILE_ITER_TYPE)
                stbuf->st_mode = S_IFREG;
        if (attr.type == DIR_ITER_TYPE)
                stbuf->st_mode = S_IFDIR;
        stbuf->st_mode |= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                          S_IROTH;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        free(cpath);
        return 0;
}

static int fuse_sfs_open(const char* path, struct fuse_file_info* fi)
{
        SFS_TRACE("OPEN path %s", path);
        path++;
        char* cpath = new_path(path);
        vino_t vino = 0;
        pino_t pino = sfs_open(sfs_description, cpath);
        if (pino == 0)
                return -sfs_errno;

        if ((vino = get_vino(pino)) == 0) {
                if ((vino = pino_add(pino)) == 0) {
                        free(cpath);
                        return -ENOMEM;
                }
        }
        set_openbit(vino);
        free(cpath);
        fi->fh = vino;
        vino_dump(vino);
        return 0;
}


static int fuse_sfs_release(const char* path, struct fuse_file_info* fi)
{
        SFS_TRACE("RELEASE path %s", path);
        vino_t vino = fi->fh;
        if (get_dirty(vino) == 1) {
                if (sfs_delete(sfs_description, get_pino(vino)) == -1)
                        return -sfs_errno;
                set_pino(vino, 0);
        }
        clr_openbit(vino);
        return 0;                
};        

static int fuse_sfs_flush(const char *path, struct fuse_file_info* fi)
{
        SFS_TRACE("FLUSH path %s", path);
        return 0;
}

static int fuse_sfs_readdir(const char* path, void* buf, 
                            fuse_fill_dir_t filler, off_t offset,
                            struct fuse_file_info* fi)
{
        SFS_TRACE("READDIR path %s", path);
        (void) offset;
        (void) fi;
        diriter iter;

        path++;
        char* cpath = new_path(path);

        int a = 1;
        int len = strlen(cpath);
        vino_t vino = 0;
        if (len == 0)
                a = 0;
        (void) a;
        iter.filename = cpath;
        iter.len = PATH_MAX;
        iter.cur_off = 0;

        while (iter.filename != NULL) {
                struct stat st_buf;
                memset(&st_buf, 0, sizeof(struct stat));
                if (sfs_readdir(sfs_description, &iter) != 0) {
                        free(cpath);
                        return -1; 
                }
                if (iter.filename == NULL)
                        break;
                st_buf.st_dev = getpid();
                if ((vino = get_vino(iter.cur_off)) == 0) {
                        if ((vino = pino_add(iter.cur_off)) == 0) {
                                free(cpath);
                                return -ENOMEM;
                        }
                }
                st_buf.st_ino = vino;
                if (iter.type == FILE_ITER_TYPE)
                        st_buf.st_mode = S_IFREG;
                if (iter.type == DIR_ITER_TYPE)
                        st_buf.st_mode = S_IFDIR;
                st_buf.st_size = iter.size;
                st_buf.st_mtime = iter.time;
                if (filler(buf, iter.filename + len + a, &st_buf, 0)) {
                        break;
                }
                iter.filename[len] = '\0';
        }
        free(cpath);
        return 0;
}

static int fuse_sfs_mkdir(const char* path, mode_t mode)
{
        SFS_TRACE("%s", "");
        path++;
        char* cpath = new_path(path);
        if (sfs_mkdir(sfs_description, cpath) == -1) {
                free(cpath);
                return -EEXIST;
        }
        free(cpath);
        return 0;
}

static int fuse_sfs_unlink(const char* path)
{
        SFS_TRACE("UNLINK path%s", path);
        path++;
        char* cpath = new_path(path);
        pino_t pino = 0;
        vino_t vino = 0;
     
        if ((pino = sfs_unlink(sfs_description, cpath)) == 0) {
                free(cpath);
                return -sfs_errno;
        }

        if ((vino = get_vino(pino)) == 0) {
                free(cpath);
                if (sfs_delete(sfs_description, pino) == -1)
                        return -sfs_errno;
                return 0;
        }
        if (get_openbit(vino) == 1)
                set_dirty(vino);
        else {
                if (sfs_delete(sfs_description, pino) == -1)
                        return -sfs_errno;
                set_pino(vino, 0);
        }  

        free(cpath);
        return 0;
}

static int fuse_sfs_rmdir(const char* path)
{
        SFS_TRACE("%s", "");
        path++;
        char* cpath = new_path(path);
        if (sfs_rmdir(sfs_description, cpath) == -1) {
                free(cpath);
                return -1;
        }
        free(cpath);
        return 0;
}

static int fuse_sfs_rename(const char* from, const char* to)
{
        SFS_TRACE("RENAME from %s to %s", from, to);
        from++;
        to++;
        char* cpath_from = new_path(from);
        char* cpath_to = new_path(to);
        pino_t old_pino = 0;
        pino_t pino = 0;
        vino_t vino = 0;
        sfs_attr attr; 
        if ((old_pino = sfs_open(sfs_description, cpath_from)) == 0) {
                free(cpath_from);
                free(cpath_to);
                return -sfs_errno;
        }

        if (sfs_getattr(sfs_description, old_pino, &attr) == -1) {
                return -sfs_errno;
        }

        if (attr.type == DIR_ITER_TYPE) {
                if (sfs_rmdir(sfs_description, cpath_from) == -1) {
                        free(cpath_from);
                        free(cpath_to);
                        return -sfs_errno;
                }
                if (sfs_mkdir(sfs_description, cpath_to) == -1) {
                        free(cpath_from);
                        free(cpath_to);
                        return -sfs_errno;
                }
                if ((pino = sfs_open(sfs_description, cpath_to)) == 0) {
                        free(cpath_from);
                        free(cpath_to);
                        return -sfs_errno;
                }
        } else {

                if ((pino = sfs_rename(sfs_description, old_pino, cpath_to)) == 0) {
                        free(cpath_from);
                        free(cpath_to);
                        return -sfs_errno;
                }
        }

        if (old_pino == pino) {
                free(cpath_from);
                free(cpath_to);
                return 0;
        }
        if ((vino = get_vino(old_pino)) == 0) {
                if ((vino = pino_add(pino)) == 0) {
                        free(cpath_from);
                        free(cpath_to);
                        return -ENOMEM;
                }
                free(cpath_from);
                free(cpath_to);
                return 0;
        }
        set_pino(vino, pino);
        free(cpath_from);
        free(cpath_to);
        return 0;
}

static int fuse_sfs_read(const char* path, char *buf, size_t size, 
                         off_t offset, struct fuse_file_info* fi)
{
        SFS_TRACE("READ: Path: %s Virtual inode %lu Physical inode %lu",
                  path, fi->fh, get_pino(fi->fh));
        int num_byte = 0;
        if ((num_byte = sfs_read(sfs_description, get_pino(fi->fh), 
                                 buf, size, offset)) == 0) {
               return -sfs_errno;
        } 
        return num_byte;
}

static int fuse_sfs_write(const char* path, const char *buf, size_t size, 
                          off_t offset, struct fuse_file_info* fi)
{ 
        SFS_TRACE("WRITE: Path: %s Virtual inode %lu Physical inode %lu",
                  path, fi->fh, get_pino(fi->fh));
        int num_byte = 0;
        if ((num_byte = sfs_write(sfs_description, get_pino(fi->fh),
                             (char*)buf, size, offset)) == 0) {
                return -sfs_errno;
        }
        return num_byte;
}

static int fuse_sfs_mknod(const char* path, mode_t mode, dev_t rdev) 
{
        SFS_TRACE("MKNOOOOOOD path %s", path);
        if (!(mode & S_IFREG))
                return -ENOTSUP;
        path++;
        char* cpath = new_path(path);

        if (mode & S_IFREG && sfs_creat(sfs_description, cpath) == -1) {
                free(cpath);
                return -ENOMEM;
        }
        if (mode & S_IFDIR && sfs_creat(sfs_description, cpath) == -1) {
                free(cpath);
                return -ENOMEM;
        }
        free(cpath);
        return 0;
}

static int fuse_sfs_chmod(const char* path, mode_t mode)
{
        return 0;
}

static int fuse_sfs_chown(const char* path, uid_t uid, gid_t gid)
{
        return 0;
}

static int fuse_sfs_utime(const char* path, struct utimbuf* buf)
{
        return 0;
}

static int fuse_sfs_truncate(const char* path, off_t length) 
{
        path++;
        char* cpath = new_path(path);
        pino_t pino = sfs_open(sfs_description, cpath);
        SFS_TRACE("TRUNCATE path %s Physical inode %lu", path, pino);
        if (pino == 0)
                return -sfs_errno;
        if (sfs_truncate(sfs_description, pino, length) != 0)
                return -sfs_errno;
        free(cpath);
        return 0;
}

static int fuse_sfs_ftruncate(const char* path, off_t length, 
                              struct fuse_file_info* fi) 
{
        SFS_TRACE("FTRUNCATE path %s Virtual inode %lu", 
                   path, get_pino(fi->fh));
        if (sfs_truncate(sfs_description, get_pino(fi->fh), length) != 0)
                return -sfs_errno;
        return 0;
}

static int fuse_sfs_opendir(const char* path, struct fuse_file_info* fi)
{
        SFS_TRACE("OPENDIR path %s", path);
        path++;
        char* cpath = new_path(path);
        vino_t vino;
        sfs_attr attr;

        pino_t pino = sfs_open(sfs_description, cpath);
        fprintf(stderr, "PINOOOOOO %lu\n", pino);
        if (pino == 0)
                return -sfs_errno;

        if (sfs_getattr(sfs_description, pino, &attr) != 0)
                return -sfs_errno;

        if (attr.type == FILE_ITER_TYPE) {
                fprintf(stderr, "$$$$$OGO\n");
                return -ENOTDIR;
        }

        if ((vino = get_vino(pino)) == 0) {
                if ((vino = pino_add(pino)) == 0) {
                        free(cpath);
                        return -ENOMEM;
                }
        }
        fi->fh  = vino;
        free(cpath);
        return 0;
}

static int fuse_sfs_releasedir(const char* path, struct fuse_file_info* fi)
{
        SFS_TRACE("RELEASEDIR path %s virtual inode %lu", path, fi->fh);
        path++;
        char* cpath = new_path(path);
        vino_t vino = fi->fh;
        if (get_dirty(vino) == 1) {
                if (sfs_rmdir(sfs_description, cpath) == -1)
                        return -sfs_errno;
                set_pino(vino, 0);
        }
        return 0;                
       
}

static int fuse_sfs_statfs(const char* path, struct statvfs* stfs)
{
        SFS_TRACE("STATFS path %s", path);
        entry entr;
        struct mbr_t mbr;
        read_data(sfs_description->bdev, 0, (uint8_t*) &mbr, sizeof(struct mbr_t));

        stfs->f_bsize = sfs_description->bdev->block_size;
        stfs->f_frsize = stfs->f_bsize;
        stfs->f_blocks = mbr.data_area_size;
        stfs->f_bfree = scan_del_file_list(sfs_description, &entr)
                        / stfs->f_bsize;
        stfs->f_bavail = stfs->f_bfree;
        stfs->f_files = mbr.index_area_size / INDEX_ENTRY_SIZE;
        stfs->f_ffree = scan_free_inode(sfs_description, &entr);
        stfs->f_favail = stfs->f_ffree;
        stfs->f_fsid = *(mbr.magic_number);
        stfs->f_flag = 0;
        stfs->f_namemax = PATH_MAX;
        return 0;
}

static struct fuse_operations sfs_oper = {
        .init           = fuse_sfs_init,   
        .destroy        = fuse_sfs_destroy,
        .getattr        = fuse_sfs_getattr,
        .flush          = fuse_sfs_flush,   
        .readdir        = fuse_sfs_readdir,
        .mkdir          = fuse_sfs_mkdir,
        .unlink         = fuse_sfs_unlink, 
        .rmdir          = fuse_sfs_rmdir,
        .mknod          = fuse_sfs_mknod,
        .chmod          = fuse_sfs_chmod,
        .chown          = fuse_sfs_chown,
        .utime          = fuse_sfs_utime,
        .rename         = fuse_sfs_rename, 
        .read           = fuse_sfs_read,
        .write          = fuse_sfs_write,
        .truncate       = fuse_sfs_truncate,
        .open           = fuse_sfs_open,
        .release        = fuse_sfs_release,
        .ftruncate      = fuse_sfs_ftruncate,
        .opendir        = fuse_sfs_opendir,
        .releasedir     = fuse_sfs_releasedir,
        .statfs         = fuse_sfs_statfs
};

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++++++++ END FUSE OPERATION STRUCTURE +++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int main(int argc, char* argv[]) {
        int image_fd = 0;
        struct stat st;
        /*
         * Read option 
         */
        char opt = 0;
        while ((opt = getopt(argc, argv, "+h")) > 0) {
                switch(opt) {
                case 'h':
                        usage(EXIT_SUCCESS);
                        break;
                case ':':
                case '?':
                        usage(EXIT_FAILURE);
                        break;
                };
        }
        /*
         * Check on exist remaining args
         */
        if ((argc - optind) < 2) 
               usage(EXIT_FAILURE);
        /*
         * Check on exist image
         */
        image_fd = open(argv[optind], O_RDWR);
        if (image_fd == -1) {
                fprintf(stderr, "Supplied image file name: \"%s\"\n", 
                                argv[optind]);
                perror("Can`t open image file");
                exit(EXIT_FAILURE);
        };
        close(image_fd);
        /*
         * Check on exist directory
         */
        int rc = lstat(argv[optind + 1], &st);
        if (rc == -1 && errno == ENOENT) {
                perror("Can't found mount directory");
                exit(EXIT_FAILURE);
        };
        /* 
         * Check on lock file
         */
        if (try_to_lock_image(argv[optind]) == -1) {
                fprintf(stderr, "Image already uses\n");
                exit(EXIT_FAILURE);
        };
        /*
         * Expand options for sending to FUSE
         */
        char** nargv = (char**) malloc((NUM_OF_FUSE_OPTIONS) * sizeof(char*));
        int nargc = NUM_OF_FUSE_OPTIONS;
        nargv[0] = argv[0];
        nargv[1] = argv[optind + 1];
        /* Disabling multi-threads operation helps to avoid race condition */
        nargv[2] = "-s";
        /* Enable foreground mode for saving value of semaphores */
        nargv[3] = "-d";
        imagefile = argv[optind];
        /*
         * Call FUSE
         */
        int ret = fuse_main(nargc, nargv, &sfs_oper);        
        free(nargv);
        return ret;
}



