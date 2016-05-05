bin_PROGRAMS = fusesfs
fusesfs_SOURCES = main.c

fusesfs_LDADD = -lm -L$(top_srcdir)/src/lib -L$(top_srcdir)/src/lib/sfs \
	        -lblock  -lsfs `pkg-config --cflags fuse`
AM_FLAGS = -Wall -Werror -I$(top_srcdir)/src/include `pkg-config --libs fuse`

