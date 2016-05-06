bin_PROGRAMS = fusesfs
fusesfs_SOURCES = main.c


fusesfs_LDFLAGS = -L$(top_srcdir)/src/lib -L$(top_srcdir)/src/lib/sfs -lblock -lsfs 
fusesfs_LDADD = -lfuse -lsfs -lblock 
AM_CFLAGS = -g3 -Wall -Werror -Wno-unused-function -I$(top_srcdir)/src/include `pkg-config --cflags fuse`
AM_LDFLAGS = `pkg-config --libs fuse`
