AM_CPPFLAGS = -DLINUX -DPKGDATADIR=\""$(datadir)/emergence/"\"

noinst_LIBRARIES = libgsub.a
libgsub_a_SOURCES = alpha_pixel_plot_x86.S convert_16bit_to_32bit_mmx.S \
	resample.c alpha_rect_draw_mmx.S fb_update_mmx.S surface_blit_mmx.S \
	alpha_surface_blit_mmx.S gsub.c surfaces.c bb_clear_mmx.S gsub.h \
	text.c blit_ops.c line.c

BUILT_SOURCES = $(libgsub_a_SOURCES)
CLEANFILES = $(libgsub_a_SOURCES)
  
$(libgsub_a_SOURCES):
	ln -s ../../gsub/$@
