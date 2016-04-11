TESTS = check_block
AM_COLOR_TESTS = always
check_PROGRAMS = check_block 
check_block_SOURCES = check_block.c
AM_CFLAGS = -I$(top_srcdir)/src/include @CHECK_CFLAGS@
check_block_LDADD = $(top_builddir)/src/lib/libblock.a @CHECK_LIBS@
