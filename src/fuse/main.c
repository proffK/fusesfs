#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fuse.h>
#include <unistd.h>

static char* imagefile = NULL;
static char* mount_point = NULL;

static void usage(unsigned status);
int check_mount_point(); 
char* normalize_name(const char* fname);

int main(int argc, char* argv[]) {

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
        /*
         * Hanlder input image and dir
         */
        if ((argc - optind) < 2) {
               usage(EXIT_FAILURE);
          
}

static void usage(unsigned status) 
{
        if (status != EXIT_SUCCESS) 
                fprintf(stderr, "Try 'fusesfs -h' for more information.\n");
        else {
                printf("Usage: fusesfs [OPTIONS] IMAGE DIRECTORY\n");
                printf("\n"
                       "-m, --metadata    Print this screen            \n");
        }
        exit(status);
}

int check_mount_point() 
{
        struct stat st;
        int rc = lstat(mount_point, &st);
        if (rc == -1 && errno == ENOENT) {
                // directory does not exists, createcontext
                rc = mkdir(mount_point, 0777); 
                if(rc != 0) {
                        perror("Can`t create mount point");
                        return -EIO;
                };
        } else if (rc == -1) {
                perror("Can`t check mount point");
                return -1;
        };
        return 0;
};

char* normalize_name(const char* fname)
{
    char* abs_fname = (char *) malloc(PATH_MAX);
    realpath(fname, abs_fname);
    // ignore errors from realpath()
    return abs_fname;
};
