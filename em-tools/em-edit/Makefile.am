SUBDIRS = share

LDADD =   @GNOME_LIBS@  ../gsub/libgsub.a ../common/libcommon.a -lm -lpthread -lz -lpng
AM_CPPFLAGS = @GNOME_CFLAGS@ -DLINUX -DPKGDATADIR=\""$(datadir)/emergence/"\" -Wall

bin_PROGRAMS = em-edit

em_edit_SOURCES = bezier.c conns.h grid.h main_lock.c nodes.h worker.c \
	bezier.h curves.c interface.c main_lock.h objects.c worker.h \
	bsp.c curves.h interface.h objects.h worker_thread.c bsp.h \
	em-edit.glade lines.c points.c worker_thread.h \
	cameron.c fills.c lines.h map.c points.h cameron.h fills.h \
	main.c map.h tiles.c conns.c grid.c main.h nodes.c tiles.h \
	glade.c glade.h callbacks.h support.h conic.c conic.h \
	floats.c floats.h

BUILT_SOURCES = glade.c glade.h

glade.c:
	glade-2 -w em-edit.glade

EXTRA_DIST = em-edit.glade em-edit.gladep
