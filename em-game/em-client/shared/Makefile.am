AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DLINUX -DEMCLIENT -Wall -I../../../stage3s/current-stage3/usr/include/

noinst_LIBRARIES = libshared.a
libshared_a_SOURCES = bsp.c config.h parse.h sgame.c timer.h \
	bsp.h cvar.c objects.h rdtsc.h sgame.h config.c cvar.h \
	parse.c rdtsc.S timer.c network.c network.h alarm.c alarm.h \
	openssl.c openssl.h fileinfo.c fileinfo.h servers.c servers.h

BUILT_SOURCES = $(libshared_a_SOURCES)
CLEANFILES = $(libshared_a_SOURCES)
                                                                                                  
$(libshared_a_SOURCES):
	ln -s ../../shared/$@
