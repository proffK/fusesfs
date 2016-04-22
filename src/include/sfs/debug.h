#ifndef _SFS_DEBUG_
#define _SFS_DEBUG_
#include <stdio.h>

#ifdef SFS_DEBUG
#define SFS_TRACE(...)    do {                           \
                fprintf(stderr, "SFS   %s:%4d[%s] : ", \
                        __FILE__, __LINE__, __func__); \
                fprintf(stderr, __VA_ARGS__);          \
                fprintf(stderr, "\n\r"); } while(0)
#else
#define SFS_TRACE(...)
#endif

#ifdef IO_DEBUG
#define IO_TRACE(...)   do {                              \
                fprintf(stderr, "SFS IO  %s:%4d[%s] : ", \
                        __FILE__, __LINE__, __func__);   \
                fprintf(stderr, __VA_ARGS__);            \
                fprintf(stderr, "\n\r"); } while(0);
#else
#define IO_TRACE(...)
#endif

#endif
