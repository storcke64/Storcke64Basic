COMPONENT = gb.hash
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.hash.la

gb_hash_la_LDFLAGS = -module @LD_FLAGS@
gb_hash_la_CFLAGS = -I$(top_srcdir)/share $(AM_CFLAGS)

gb_hash_la_SOURCES = \
 gb.hash.h main.h main.c \
 hash.h hash.c platform.h


