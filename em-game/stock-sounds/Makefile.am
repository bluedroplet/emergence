SOURCES = railgun.wav teleporter.wav speedup-ramp.wav
OBJECTS = $(SOURCES:.wav=.ogg)

OGGENCFLAGS = -q 3

all: $(OBJECTS)

%.ogg: %.wav
	oggenc $(OGGENCFLAGS) $<

clean:
	rm -f $(OBJECTS) Makefile

distclean: clean

sounddatadir = $(pkgdatadir)/stock-sounds
sounddata_DATA = $(OBJECTS)
EXTRA_DIST = $(OBJECTS)

.PHONY: all clean
