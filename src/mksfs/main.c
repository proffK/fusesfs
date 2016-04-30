#define _GNU_SOURCE
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
static size_t convert_size(char* parameter, off_t file_s);

int main(int argc, char* argv[]) {
        int opt = 0;
        struct sfs_options sfs_opts;
        /* Service variables */
        size_t rsrvd_size       = MBR_SIZE;
        size_t index_size       = 0;
        size_t block_size       = 0;
        size_t total_blocks     = 0;
        size_t index_sz_perblk  = 0;
        off_t  file_size        = 0;
        char*  index_size_s     = NULL;
        char*  block_size_s     = NULL;
        char*  label            = NULL;
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
        while ((opt = getopt_long(argc - 1, argv, "hm:b:l:", 
                long_options, NULL)) != -1) {
                switch (opt) {    
                case 'h':
                        usage(EXIT_SUCCESS);
                        break;
                case 'm':
                        index_size_s = optarg; 
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
        if (fd == -1 && argc == 2) 
                usage(EXIT_SUCCESS);

        if (fd == -1) {
                fprintf(stderr, "Invalid input file/device.\n");
                usage(EXIT_INPFILE);
        }
        /*
         * File size calculate and check it
         */
        file_size = lseek(fd, 0, SEEK_END);
        close(fd);
        if (file_size < (MBR_SIZE + INDEX_MIN_SIZE)) {
                fprintf(stderr, "Too small file size.\n");
                fprintf(stderr, "%s %luB\n", "The smallest file size is", 
                                             MBR_SIZE + INDEX_MIN_SIZE);
                exit(EXIT_FILELRG);
        }
        /* 
         * Handler blocksize data 
         */ 
        if (block_size_s == NULL) 
                block_size = DEFAULT_BLOCK_SIZE;
        else {
                block_size = convert_size(block_size_s, 0);
                /* block size must be greater than 128B */
                if (block_size <= DEFAULT_MIN_BLOCK/2 || errno == EINVAL) {
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
        if (block_size > file_size) {
                fprintf(stderr, "Block size more than file size.\n");
                usage(EXIT_BSLRG);
        }
        if (file_size % block_size != 0) {
                fprintf(stderr, "Block size isn't a divisor of data size.\n");
                usage(EXIT_BSDIV);
        }
        /*
         * Calculate reserved area size in bytes
         */
        if (block_size <= MBR_SIZE)
                rsrvd_size = MBR_SIZE;
        else
                rsrvd_size = block_size;
        /* 
         * Handler index size 
         */
        if (index_size_s == NULL) {
                double buf = DEFAULT_INDEX_PERCENT * file_size / 100L;
                index_size = (size_t)round(buf);

        } else {
                index_size = convert_size(index_size_s, file_size);
                if (index_size == 0 || errno == EINVAL) {
                        fprintf(stderr, "Invalid metadata size.\n");
                        usage(EXIT_NOMD);
                }
                
        }
        /* Auto align to BLOCK_SIZE (up) */
        index_size += block_size - index_size % block_size; 
        /* Check index size(maybe file size too small) */
        if (index_size > (file_size - rsrvd_size)) {
                fprintf(stderr, "Index area size more or equal than file"
                                " size.\n");
                usage(EXIT_MDLRG);
        }
        /* Check size of index area */
        if (index_size < INDEX_MIN_SIZE) {
                fprintf(stderr, "Index part size too small.\n");
                usage(EXIT_MDSML);
        } 
        /*
         * Handler label name
         */
        unsigned length = 0;
        int i = 0;
        if (label != NULL && (length = strlen(label)) >= VOLUME_NAME_SIZE) {
                fprintf(stderr, "%s %ld %s", "Label shouldn't be longer"
                                " than ", VOLUME_NAME_SIZE - 1,
                                " symbols.\n");
                usage(EXIT_LBL);
        }
        /* Check on unsupported symbols */ 
        for (i = 0; i < length; i++)
                if (label[i] < 0x20   || 
                   (label[i] >= 0x80  && label[i] <= 0x9F) ||
                    label[i] == '"'   || label[i] == '*'   ||
                    label[i] == ':'   || label[i] == '<'   ||
                    label[i] == '>'   || label[i] == '?'   ||
                    label[i] == '\\'  || label[i] == 0x5C  ||
                    label[i] == 0x7F  || label[i] == 0xA0) {
                        fprintf(stderr, "Unsupported symbol \'%c\' in volume name.\n",
                                label[i]);
                        usage(EXIT_LBL);
                }
        /*
         * Start to flll fields of options struct
         */
        total_blocks = file_size / block_size;
        /* Convert reserved area size to size in blocks */
        rsrvd_size /= block_size;
        /* Convert index area size to align size per block size */
        if (index_size % block_size == 0) 
                index_sz_perblk = index_size / block_size;
        else
                index_sz_perblk = index_size / block_size + 1;
        /* Fill struct */
        sfs_opts.time_stamp = time(NULL);
        sfs_opts.data_size = total_blocks - rsrvd_size - index_sz_perblk; 
        sfs_opts.index_size = index_size;
        sfs_opts.total_block = total_blocks;
        sfs_opts.reserved_size = rsrvd_size;
        sfs_opts.block_size = (size_t)log2(block_size) - BEGIN_POWER_OF_BS;
        if (label != NULL)
                strcpy(sfs_opts.label, label);
        else 
                sfs_opts.label[0] = '\0';
        sfs_opts.file_name = argv[argc - 1];
        /*
         * Create empty SFS image 
         */
        if (image_create(sfs_opts) != 0)
                return EXIT_FAILURE;
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
                printf("\n"
                       "-m, --metadata    Size of metadata part              \n"
                       "                  Default 5%% of file size.          \n"
                       "-b, --block-size  Number of bytes in each block.     \n"
                       "                  It should be more than 128B and    \n"
                       "                  block_size = 2^N, N is integer.    \n"
                       "                  Default is 512B.                   \n"
                       "-l, --label       Volume name in UTF-8, it shouldn't \n"
                       "                  more than 51 symbols.\n"); 
        }
        exit(status);
}     

/*
 * Convert other form of size(B, K, M, G)
 * Number without postfis is handlers as B
 */
static size_t convert_size(char* parameter, off_t file_s) 
{
        ssize_t number = 0;
        char err_c = 0;
        char last_sym = 0;
        int ret_code = 0;
        errno = 0;
        /* Try to recognize a number */
        ret_code = sscanf(parameter, "%lu%c%c", &number, &last_sym, &err_c);
        if (err_c != '\0' || ret_code == 0 || number < 0) {
                errno = EINVAL;
                return (size_t)(-1);
        }
        /* Try to recognize a unit of size */
        switch (last_sym) {
        case '%':
                if (file_s != 0)
                        return (size_t)(file_s * number / 100L);
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
                errno = EINVAL;
                return (size_t)(-1);
        }
        errno = EINVAL;
        return (size_t)(-1);
}
