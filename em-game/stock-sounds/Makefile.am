SOURCES = railgun.wav teleporter.wav speedup-ramp.wav
OBJECTS = $(SOURCES:.wav=.ogg)

OGGENCFLAGS = -q 3

all: $(OBJECTS)

%.ogg: %.wav
	oggenc $(OGGENCFLAGS) $<

clean:
	rm -f $(OBJECTS) 

distclean: clean

sounddatadir = $(pkgdatadir)/stock-sounds
sounddata_DATA = $(OBJECTS)

.PHONY: all clean
