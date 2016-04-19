#ifndef _SFS_DEBUG_
#define _SFS_DEBUG_
#include <stdio.h>

#ifdef SFS_DEBUG
#define SFS_TRACE(...)     {                           \
                fprintf(stderr, "SFS   %s:%4d[%s] : ", \
                        __FILE__, __LINE__, __func__); \
                fprintf(stderr, __VA_ARGS__);          \
                fprintf(stderr, "\n\r"); }
#else
#define SFS_TRACE(...)
#endif

#ifdef IO_DEBUG
#define IO_TRACE(...)     {                              \
                fprintf(stderr, "SFS IO  %s:%4d[%s] : ", \
                        __FILE__, __LINE__, __func__);   \
                fprintf(stderr, __VA_ARGS__);            \
                fprintf(stderr, "\n\r"); }
#else
#define IO_TRACE(...)
#endif

#endif
