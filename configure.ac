AC_INIT([sfs-utils], [0.02], [proffk@linux.com])
AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB 

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
                 Makefile
                 src/mksfs/Makefile
                 src/lib/Makefile
                 src/lib/sfs/Makefile
                 tests/Makefile
                 src/fuse/Makefile
                 ])

PKG_CHECK_MODULES([CHECK], [check])

AC_OUTPUT
