#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#define FUSE_USE_VERSION 22
#include <fuse.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sfs/unit.h>
#include <sfs/debug.h>
#include <bdev/filedev.h>
#include <sfs/mbr.h>

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
#define ADD_OPER(num, sem, oper, flags) ({  \
    sem_ops[num].sem_num = (sem);           \
    sem_ops[num].sem_op = (oper);           \
    sem_ops[num].sem_flg = (flags);         \
})

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
        blockdev* bdev = malloc(sizeof(blockdev));
        filedev_data* fdev = malloc(sizeof(filedev_data));
        sfs_description = malloc(sizeof(sfs_unit));
        /*
         * Try to recognize block_size
         */
        int temp_fd = open(imagefile, O_RDONLY);
        lseek(temp_fd, offsetof(struct mbr_t, block_size), SEEK_SET);
        if (read(temp_fd, &bs_p2, 1) == 0)
                return NULL;
        bs = 128 << bs_p2;
        lseek(temp_fd, offsetof(struct mbr_t, total_size), SEEK_SET);
        if (read(temp_fd, &file_size, 1) == 0)
                return NULL;
        close(temp_fd);
        SFS_TRACE("BS: %lu, FS: %lu", bs, file_size);
        /*
         * Init fs
         */ 
        fdev->fd = -1;
        bdev = filedev_create(bdev, fdev, bs, file_size * bs);
        fdev->filename = imagefile;
        if (blockdev_init(bdev) != 0)
                return NULL;
        if (sfs_init(sfs_description, bdev) != 0)
                return NULL;
        SFS_TRACE("Init finished");
        return NULL;
}

static void fuse_sfs_destroy(void* param)
{
        SFS_TRACE("%s", "");
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
        sfs_attr attr;
        if (sfs_getattr(sfs_description, path, &attr) != 0)
                return -ENOENT;
        stbuf->st_dev  = getpid();
        stbuf->st_ino  = attr.off;
        stbuf->st_size = attr.size;
        stbuf->st_mtime = attr.time;
        if (attr.type == FILE_ITER_TYPE)
                stbuf->st_mode = S_IFREG;
        if (attr.type == DIR_ITER_TYPE)
                stbuf->st_mode = S_IFDIR;
        free(cpath);
        return 0;
}

static int fuse_sfs_open(const char* path, struct fuse_file_info* fi)
{
        SFS_TRACE("OPEN path %s", path);
        return -1;
}

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
                        //fprintf(stderr, "READDIR ERROR!!!!@@@@@@@@@@@@@@\n");
                        free(cpath);
                        return -1; 
                }
                if (iter.filename == NULL)
                        break;
                st_buf.st_dev = getpid();
                st_buf.st_ino = iter.cur_off;
                if (iter.type == FILE_ITER_TYPE)
                        st_buf.st_mode = S_IFREG;
                if (iter.type == DIR_ITER_TYPE)
                        st_buf.st_mode = S_IFDIR;
                st_buf.st_size = iter.size;
                st_buf.st_mtime = iter.time;
                //fprintf(stderr, "@@@@@@FILLER FILENAME %s len %d a %d\n", iter.filename + len + a, len, a);
                if (filler(buf, iter.filename + len + a, &st_buf, 0)) {
                        //fprintf(stderr, "I AM FILOSOPHER###############\n");
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
        if (sfs_unlink(sfs_description, cpath) == -1) {
                free(cpath);
                return -ENOENT;
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

//TODO
static int fuse_sfs_rename(const char* from, const char* to)
{
        SFS_TRACE("%s", "");
        return 0;
}

//TODO
static int fuse_sfs_read(const char* path, char *buf, size_t size, 
                         off_t offset, struct fuse_file_info* fi)
{
        SFS_TRACE("%s", "");
        return 0;
}

//TODO
static int fuse_sfs_write(const char* path, const char *buf, size_t size, 
                          off_t offset, struct fuse_file_info* fi)
{
        SFS_TRACE("%s", "");
        return -1;
}

static int fuse_sfs_mknod(const char* path, mode_t mode, dev_t rdev) 
{
        SFS_TRACE("MKNOOOOOOD path %s", path);
        if (!(mode & S_IFREG))
                return -ENOTSUP;
        path++;
        char* cpath = new_path(path);
        if (sfs_creat(sfs_description, cpath) == -1) {
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
        .utime          = fuse_sfs_utime
        //.rename         = fuse_sfs_rename, 
        //.read           = fuse_sfs_read,
        //.write          = fuse_sfs_write,
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



