OBJECTS = rails.png  shield-pickup.png  unowned-minigun.png \
	unowned-plasma-cannon.png  unowned-rocket-launcher.png

sharedatadir = $(datadir)/emergence/stock-object-textures

sharedata_DATA = $(OBJECTS)
EXTRA_DIST = $(OBJECTS)

$(OBJECTS):
	ln -s ../../stock-object-textures/$@
                                                                                                                               
all: $(OBJECTS)

clean:
	rm -f $(OBJECTS)
                                                                                                                               
distclean: clean
	rm -f Makefile
