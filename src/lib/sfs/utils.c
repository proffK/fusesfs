#include <sfs/utils.h>
#include <sfs/entry.h>

#define D_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint32_t)))
#define W_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint16_t)))

uint8_t* memcpy(uint8_t* dst, uint8_t* src, size_t size) 
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

int is_correct_string(char* string, size_t length) 
{
        size_t i = 0;
        for (i = 0; i < length; i++)
                if (is_correct_char(string[i]) == -1)
                        return -1;
        return 0;
}

int is_correct_filepath(char* string) 
{
        if (is_correct_string(string, MAX_FILE_PATH) == -1)
                return -1;
        return 0;
}

int is_correct_dirpath(char* string) 
{
        if (is_correct_string(string, MAX_DIR_PATH) == -1)
                return -1;
        return 0;
}

int is_correct_label(char* string) 
{
        if (is_correct_string(string, MAX_VOL_LABEL) == -1)
                return -1;
        return 0;
}

uint8_t calc_checksum(struct mbr_t* mbr_section)
{
        uint8_t buffer = 0;
        uint64_t counter = 0;
        uint64_t end = 0;
        counter = (uint64_t)mbr_section->magic_number;
        end = (uint64_t)mbr_section->checksum;
        for (; counter < end; counter++) 
                buffer += *(uint8_t*)counter;
        return buffer;        
}
