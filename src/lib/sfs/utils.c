/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev, Edgar Kaziahmedov>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sfs/utils.h>
#include <sfs/entry.h>
#include <stdio.h>

#define D_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint32_t)))
#define W_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint16_t)))

#ifdef SFS_ERRNO
int sfs_errno = 0;
#endif

#ifndef UTILS_NON_CONFLICT
uint8_t* memcpy(void* dst, void* src, size_t size)
{
        uint8_t* old_dst = dst;

        if (src == NULL || dst == NULL) {
                return NULL;
        }

        if (D_ALIGNED(src) && D_ALIGNED(dst) && D_ALIGNED(size)) {
                while(size) {
                        *((uint32_t*) dst) = *((uint32_t*) src);
                        dst += sizeof(uint32_t);
                        src += sizeof(uint32_t);
                        size -= sizeof(uint32_t);
                }

                return old_dst;
        }

        if (W_ALIGNED(src) && W_ALIGNED(dst) && W_ALIGNED(size)) {
                while(size) {
                        *((uint16_t*) dst) = *((uint16_t*) src);
                        dst += sizeof(uint16_t);
                        src += sizeof(uint16_t);
                        size -= sizeof(uint16_t);
                }

                return old_dst;
        }

        while(size) {
                *((uint8_t*) dst++) = *((uint8_t*) src++); 
                size--;
        }

        return old_dst;
}

void* memset(void* dst, int c, size_t size)
{
        void* old_dst = dst;
        uint8_t z = (uint8_t) c;

        if (dst == NULL) {
                return NULL;
        }

        if (D_ALIGNED(dst) && D_ALIGNED(size)) {
                register uint32_t dtemp;

                dtemp = (z << (3 * sizeof(uint8_t))) +
                        (z << (2 * sizeof(uint8_t))) +
                        (z << sizeof(uint8_t)) + z;

                while (size) {
                        *((uint32_t*) dst) = dtemp;
                        dst += sizeof(uint32_t);
                        size -= sizeof(uint32_t);
                }

                return old_dst;
        }

        if (W_ALIGNED(dst) && W_ALIGNED(size)) {
                register uint16_t wtemp;

                wtemp = (z << sizeof(uint8_t)) + z;

                while (size) {
                        *((uint16_t*) dst) = wtemp;
                        dst += sizeof(uint16_t);
                        size -= sizeof(uint16_t);
                }

                return old_dst;
        }

        while (size) {
                *((uint8_t*) dst++) = z;
                size--;
        }

        return old_dst;
}
#endif

int is_correct_string(const char* string, size_t length) 
{
        size_t i = 0;
        for (i = 0; i < length && string[i] != '\0'; i++)
                if (is_correct_char(string[i]) == -1)
                        return -1;
        return 0;
}

int is_correct_filepath(const char* string) 
{
        if (is_correct_string(string, MAX_FILE_PATH) == -1)
                return -1;
        return 0;
}

int is_correct_dirpath(const char* string) 
{
        if (is_correct_string(string, MAX_DIR_PATH) == -1)
                return -1;
        return 0;
}

int is_correct_label(const char* string) 
{
        if (is_correct_string(string, MAX_VOL_LABEL) == -1)
                return -1;
        return 0;
}

uint8_t calc_checksum(uint8_t* mag_num, uint8_t* sfs_v, uint64_t* tot_size, 
                      uint32_t* res_size, uint8_t* b_size)
{
        uint8_t counter = 0;
        counter += mag_num[0] + mag_num[1] + mag_num[2];
        counter += *sfs_v;
        counter = counter +
                  (uint8_t)(((*tot_size) & 0xFF00000000000000)>>56) + 
                  (uint8_t)(((*tot_size) & 0x00FF000000000000)>>48) +
                  (uint8_t)(((*tot_size) & 0x0000FF0000000000)>>40) +
                  (uint8_t)(((*tot_size) & 0x000000FF00000000)>>32) +
                  (uint8_t)(((*tot_size) & 0x00000000FF000000)>>24) +
                  (uint8_t)(((*tot_size) & 0x0000000000FF0000)>>16) +
                  (uint8_t)(((*tot_size) & 0x000000000000FF00)>>8)  +
                  (uint8_t)((*tot_size) & 0x00000000000000FF);
        counter = counter +
                  (uint8_t)(((*res_size) & 0xFF000000)) +
                  (uint8_t)(((*res_size) & 0x00FF0000)) +
                  (uint8_t)(((*res_size) & 0x0000FF00)) +
                  (uint8_t)((*res_size) & 0x000000FF);
        counter += *b_size;
        return counter;        
}

int strcmp(const char* s1, const char* s2)
{
        while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0') {
                s1++;
                s2++;
        }

        if (*s1 == '\0' || *s2 == '\0')
                return 0;

        return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
        int i = 0;
        while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0' && i < n) {
                s1++;
                s2++;
                i++;
        }

        if (*s1 == '\0' || *s2 == '\0' || i == n)
                return 0;

        return *s1 - *s2;
}

size_t strlen(const char* s)
{
        int l = 0;

        while (s[l] != '\0') l++;

        return l;
}

size_t strnlen(const char* s, size_t n)
{
        size_t l = 0;

        while (n != 0 && s[l] != '\0') {
		n--;
		l++;
	}

        return l;
}

char* strcpy(char* dest, const char* src)
{
        size_t i = 0;

        while ((dest[i] = src[i]) != '\0') i++;

        return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
        size_t i;

        for (i = 0; i < n && src[i] != '\0'; i++)
            dest[i] = src[i];
        for ( ; i < n; i++)
            dest[i] = '\0';

        return dest;
}
