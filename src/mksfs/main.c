#define _GNU_SOURCE
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include <mksfs/mksfs.h>

static void usage(unsigned status);
static ssize_t convert_size(char* parameter, off_t file_s);

int main(int argc, char* argv[]) {
        int opt = 0;
        int flag = 0;
        struct sfs_options sfs_opts;
        /* Local copies fields of sfs_options */
        char* md_size_s = NULL;
        ssize_t md_size = 0;
        char* block_size_s = NULL;
        ssize_t block_size = 0;
        char* label = NULL;
        char* file_name = NULL;
        off_t file_size = 0;
        /* Struct for getopt_long */
        static struct option const long_options[] = {
                {"help", no_argument, NULL, 'h'},
                {"metadata", required_argument, NULL, 'm'},
                {"block-size", required_argument, NULL, 'b'},
                {"label", required_argument, NULL, 'l'},
                {"version", no_argument, NULL, 'v'},
                {NULL, 0, NULL, 0}
        };
        if (argc == 1) {
                fprintf(stderr, "No parameters.\n");
                usage(EXIT_FAILURE);
        }
        /* Get user options */
        while ((opt = getopt_long(argc, argv, "hm:b:l:", 
                long_options, NULL)) != -1) {
                switch (opt) {    
                case 'h':
                        usage(EXIT_SUCCESS);
                        break;
                case 'm':
                        md_size_s = optarg; 
                        break;
                case 'b':
                        block_size_s = optarg;
                        break;
                case 'l':
                        label = optarg;
                        break;
                case 'v':
                        usage(EXIT_SUCCESS);
                        break; 
                default:
                        usage(EXIT_FAILURE);
                }                     
        }
        /* 
         * Try to open file 
         */
        int fd = open(argv[argc - 1], O_RDWR);
        if (fd == -1) {
                fprintf(stderr, "Invalid input file/device.\n");
                usage(EXIT_INPFILE);
        }
        /*
         * File size calculate and check it
         */
        file_size = lseek(fd, 0, SEEK_END);
        close(fd);
        if (file_size < (MBR_SIZE + METADATA_MIN_SIZE)) {
                fprintf(stderr, "Too small file size.\n");
                fprintf(stderr, "%s %luB\n", "The smallest file size is", 
                                             MBR_SIZE + METADATA_MIN_SIZE);
                exit(EXIT_FILELRG);
        }
        /* 
         * Handler metadata size 
         */
        if (md_size_s == NULL) {
                double buf = DEFAULT_METADATA_PERCENT * file_size / 100L;
                md_size = (ssize_t)round(buf);
        } else {
                md_size = convert_size(md_size_s, file_size);
                if (md_size <= 0) {
                        fprintf(stderr, "Invalid metadata size.\n");
                        usage(EXIT_NOMD);
                }
                if (md_size > (file_size - MBR_SIZE)) {
                        fprintf(stderr, "Metadata part size more than file"
                                        " size.\n");
                        usage(EXIT_MDLRG);
                }
                if (md_size < METADATA_MIN_SIZE) {
                        fprintf(stderr, "Metadata part size too small.\n");
                        usage(EXIT_MDSML);
                }
        }
        /* Align to FS_ENTRY_POINT (down) */
        md_size -= md_size % FS_ENTRY_SIZE;
        /* 
         * Handler blocksize data 
         */ 
        if (block_size_s == NULL) 
                block_size = DEFAULT_BLOCK_SIZE;
        else {
                block_size = convert_size(block_size_s, 0);
                if (block_size <= DEFAULT_BLOCK_SIZE/2 ) {
                        fprintf(stderr, "Invalid block size.\n");
                        usage(EXIT_NOBS);
                }
                long int divisor = DEFAULT_MIN_BLOCK;
                /* Check on the power of two */
                while (divisor > 0 && (divisor != block_size)) 
                        divisor <<= 1;

                if (divisor < 0) {
                        fprintf(stderr, "Block size isn't the power of 2.\n");
                        usage(EXIT_BSDGR2);
                }
        }
        if (block_size > (file_size - MBR_SIZE - md_size)) {
                fprintf(stderr, "Block size more than file size.\n");
                usage(EXIT_BSLRG);
        }
        if ((file_size - MBR_SIZE - md_size) % block_size != 0) {
                fprintf(stderr, "Block size isn't a divisor of data size.\n");
                usage(EXIT_BSDIV);
        }
        /*
         * Handler label name
         */
        if (label != NULL && strlen(label) >= VOLUME_NAME_SIZE) {
                fprintf(stderr, "%s %ld %s", "Label shouldn't be longer"
                                " than ", VOLUME_NAME_SIZE - 1,
                                " symbols.\n");
                usage(EXIT_LBL);
        }
        /*
         * Start to flll fields of options struct
         */
        sfs_opts.md_size = md_size;
        sfs_opts.block_size = block_size;
        if (label != NULL)
                strcpy(sfs_opts.label, label);
        else 
                sfs_opts.label[0] = '\0';
        sfs_opts.file_name = argv[argc - 1];
        /*
         * Create empty SFS image 
         */

        return EXIT_SUCCESS;
}

/*
 * Print help and extended help
 */
static void usage(unsigned status) 
{
        if (status != EXIT_SUCCESS) 
                fprintf(stderr, "Try 'mksfs --help' for more information.\n");
        else {
                printf("Usage: mksfs [OPTIONS] FILE\n");
                fputs("\n"
                      "-m, --metadata      Size of metadata part             \n"
                      "                    Default 5%% of file size.         \n"
                      "-b, --block-size    Number of bytes in each block.    \n"
                      "                    It should be more than 128B and   \n"
                      "                    block_size = 2^N, N is integer.   \n"
                      "                    Default is 512B.                  \n"
                      "-l, --label         Volume name in UTF-8, it shouldn't\n"
                      "                    more than 51 symbols.\n",
                       stdout); 
        }
        exit(status);
}     

/*
 * Convert other form of size(B, K, M, G)
 * Number without postfis is handlers as B
 */
static ssize_t convert_size(char* parameter, off_t file_s) 
{
        ssize_t number = 0;
        char err_c = 0;
        char last_sym = 0;
        int ret_code = 0;
        /* Try to recognize a number */
        ret_code = sscanf(parameter, "%lu%c%c", &number, &last_sym, &err_c);
        if (err_c != '\0' || ret_code == 0)
                return -1;
        /* Try to recognize a unit of size */
        switch (last_sym) {
        case '%':
                if (file_s != 0)
                        return (ssize_t)(file_s * number / 100L);
                break;
        case 'B':
        case '\0':
                return number;
        case 'K':
                return 1024 * number;
        case 'M':
                return 1024 * 1024 * number;
        case 'G':
                return 1024 * 1024 * 1024 * number;
        default:
                return -1;
        }
        return -1;
}