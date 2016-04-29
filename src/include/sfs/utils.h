#ifndef _SFS_UTILS_
#define _SFS_UTILS_
#include <stddef.h>
#include <stdint.h>

#include <sfs/mbr.h>

uint8_t* memcpy(uint8_t* src, uint8_t* dst, size_t size);

inline static int is_correct_char(char c) 
{
        if (c < 0x20 || (c >= 0x80 && c <= 0x9F) ||
            c == '"'  || c == '*'  ||
            c == ':'  || c == '<'  ||
            c == '>'  || c == '?'  ||
            c == '\\' || c == 0x5C ||
            c == 0x7F || c == 0xA0) 
                return -1;
        return 0;
}

int is_correct_string(char* string, size_t length);

int is_correct_filepath(char* string);

int is_correct_dirpath(char* string); 

int is_correct_label(char* string);

uint8_t calc_checksum(struct mbr_t* mbr_section);        

#endif
