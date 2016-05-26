/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Klim Kireev>

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

#ifndef _SFS_DEBUG_
#define _SFS_DEBUG_
#include <stdio.h>
//#define SFS_DEBUG
//#define IO_DEBUG
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
