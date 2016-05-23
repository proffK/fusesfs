#ifndef _SFS_UTILS_
#define _SFS_UTILS_
#include <sfs/defs.h>

#include <sfs/mbr.h>

#define SFS_ERRNO

#ifndef UTILS_NON_CONFLICT
uint8_t* memcpy(void* dst, void* src, size_t size);

void* memset(void* dst, int c, size_t size);
#endif

inline static int is_correct_char(char c) 
{
        if (c < 0x20  || (c >= 0x80 && c <= 0x9F) ||
            c == '"'  || c == '*'  ||
            c == ':'  || c == '<'  ||
            c == '>'  || c == '?'  ||
            c == '\\' || c == 0x5C ||
            c == 0x7F || c == 0xA0) 
                return -1;
        return 0;
}

int strcmp(const char* s1, const char* s2);

int strncmp(const char* s1, const char* s2, size_t n);

char* strcpy(char* dest, const char* src);

char* strncpy(char* dest, const char* src, size_t n);

size_t strlen(const char* s);

size_t strnlen(const char* s, size_t n);

int is_correct_string(const char* string, size_t length);

int is_correct_filepath(const char* string);

int is_correct_dirpath(const char* string); 

int is_correct_label(const char* string);

uint8_t calc_checksum(uint8_t* mag_num, uint8_t* sfs_v, uint64_t* tot_size, 
                      uint32_t* res_size, uint8_t* b_size);

#ifdef SFS_ERRNO
extern int sfs_errno;

#define EPERM        1  /* Operation not permitted */
#define ENOENT       2  /* No such file or directory */
#define ESRCH        3  /* No such process */
#define EINTR        4  /* Interrupted system call */
#define EIO      5  /* I/O error */
#define ENXIO        6  /* No such device or address */
#define E2BIG        7  /* Argument list too long */
#define ENOEXEC      8  /* Exec format error */
#define EBADF        9  /* Bad file number */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Try again */
#define ENOMEM      12  /* Out of memory */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
#define ENOTBLK     15  /* Block device required */
#define EBUSY       16  /* Device or resource busy */
#define EEXIST      17  /* File exists */
#define EXDEV       18  /* Cross-device link */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define ENFILE      23  /* File table overflow */
#define EMFILE      24  /* Too many open files */
#define ENOTTY      25  /* Not a typewriter */
#define ETXTBSY     26  /* Text file busy */
#define EFBIG       27  /* File too large */
#define ENOSPC      28  /* No space left on device */
#define ESPIPE      29  /* Illegal seek */
#define EROFS       30  /* Read-only file system */
#define EMLINK      31  /* Too many links */
#define EPIPE       32  /* Broken pipe */
#define EDOM        33  /* Math argument out of domain of func */
#define ERANGE      34  /* Math result not representable */
#define ENOTEMPTY   39  

#define SET_ERRNO(err_code) sfs_errno = err_code
#define GET_ERRNO sfs_errno

#endif
#ifndef SFS_ERRNO

#define SET_ERRNO(err_code) 
#define GET_ERRNO 0

#endif

#endif
