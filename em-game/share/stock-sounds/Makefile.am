SOURCES = railgun.wav teleporter.wav speedup-ramp.wav explosion.wav
OBJECTS = $(SOURCES:.wav=.ogg)

OGGENCFLAGS = -q 3

all: $(OBJECTS)

%.ogg: %.wav
	oggenc $(OGGENCFLAGS) $<

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f Makefile

sounddatadir = $(pkgdatadir)/stock-sounds
sounddata_DATA = $(OBJECTS)
EXTRA_DIST = $(OBJECTS)

.PHONY: all clean
