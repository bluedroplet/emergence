AM_CPPFLAGS = -DLINUX -DEMSERVER -Wall

noinst_LIBRARIES = libshared.a
libshared_a_SOURCES = bsp.c  network.h parse.h sgame.c timer.h \
	bsp.h cvar.c objects.h rdtsc.h sgame.h cvar.h \
	parse.c rdtsc.S timer.c

BUILT_SOURCES = $(libshared_a_SOURCES)
CLEANFILES = $(libshared_a_SOURCES)
                                                                                                  
$(libshared_a_SOURCES):
	ln -s ../../shared/$@
