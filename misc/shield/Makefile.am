LDADD = ../gsub/libgsub.a ../common/libcommon.a -lm -lz -lpng
CFLAGS = -march=pentium3 -O3 -pipe -fomit-frame-pointer -fforce-addr -falign-functions=4 -fprefetch-loop-arrays -Wall -DLINUX -DPKGDATADIR=\""$(datadir)/emergence/em-edit"\"

bin_PROGRAMS = shield

shield_SOURCES = main.c

