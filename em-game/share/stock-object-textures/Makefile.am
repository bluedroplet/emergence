SHARED_OBJECTS = rails.png  shield-pickup.png \
	unowned-minigun.png unowned-plasma-cannon.png  \
	unowned-rocket-launcher.png

OBJECTS =  rocket.png plasma.png mine.png craft-shield.png weapon-shield.png

sharedatadir = $(pkgdatadir)/stock-object-textures

sharedata_DATA = $(SHARED_OBJECTS) $(OBJECTS)
EXTRA_DIST = $(SHARED_OBJECTS) $(OBJECTS)

$(SHARED_OBJECTS):
	ln -s ../../../stock-object-textures/$@
                                                                                                                               
all: $(SHARED_OBJECTS)

clean:
	rm -f $(SHARED_OBJECTS)
                                                                                                                               
distclean: clean
	rm -f Makefile
