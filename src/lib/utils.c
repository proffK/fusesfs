#include <sfs/utils.h>

#define D_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint32_t)))
#define W_ALIGNED(ptr) (!((uint64_t) ptr % sizeof(uint16_t)))

uint8_t* memcpy(uint8_t* src, uint8_t* dst, size_t size) 
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
