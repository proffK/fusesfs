TESTS = check_block_write check_block_read check_block
AM_COLOR_TESTS = always
check_PROGRAMS = check_block check_block_write check_block_read
check_block_SOURCES = check_block.c
check_block_read_SOURCES = check_block_read.c
check_block_write_SOURCES = check_block_write.c
AM_CFLAGS = -I$(top_srcdir)/src/include @CHECK_CFLAGS@
LDADD = $(top_builddir)/src/lib/libblock.a @CHECK_LIBS@
