AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DLINUX -DEMGAME

noinst_LIBRARIES = libcommon.a
libcommon_a_SOURCES = buffer.c polygon.c stringbuf.h types.h buffer.h \
	llist.h minmax.h polygon.h stringbuf.c vertex.h \
	abs2rel.c abs2rel.h rel2abs.c rel2abs.h inout.c inout.h \
	user.c user.h prefix.c prefix.h resource.c resource.h

BUILT_SOURCES = $(libcommon_a_SOURCES)
CLEANFILES = $(libcommon_a_SOURCES)
 
$(libcommon_a_SOURCES):
	ln -s ../../common/$@
