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

uint8_t calc_checksum(struct mbr_t* mbr_section);        

#ifdef SFS_ERRNO
extern int sfs_errno;

enum err_code {
        EINVAL = 1,
        ENOMEM,
        EIO,
        EFAULT
};

#define SET_ERRNO(err_code) sfs_errno = err_code
#define GET_ERRNO sfs_errno

#endif
#ifndef SFS_ERRNO

#define SET_ERRNO(err_code) 
#define GET_ERRNO 0

#endif

#endif
