SOURCES = stock-speedup-ramp.svg
OBJECTS = $(SOURCES:.svg=.png)

sharedatadir = $(datadir)/emergence/em-edit

sharedata_DATA = about.png splash.png gravity-well-placeholder.png \
	node.png teleporter-placeholder.png \
	point.png vect-sat.png select.png width-sat.png \
	spawn-point-placeholder.png $(OBJECTS)

EXTRA_DIST = about.png splash.png gravity-well-placeholder.png \
	node.png teleporter-placeholder.png \
	point.png vect-sat.png select.png width-sat.png \
	spawn-point-placeholder.png $(SOURCES)

 
all: $(OBJECTS)
 
%.png: %.svg
	sodipodi -z -e $@ -w 256 -f $<
 
clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f Makefile
 
.PHONY: all clean

