AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = lged

LIBS = $(top_builddir)/util/libutil.a
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lgeneral-redit/src
lged_CFLAGS = -DCMDLINE_ONLY -DTOP_SRCDIR=\"$(top_srcdir)\"
lged_LDFLAGS =
lged_SOURCES = lged.c misc.c parser.c list.c

