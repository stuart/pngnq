AM_LDFLAGS = ${shell libpng-config --ldflags} -lz    
AM_CFLAGS = ${shell libpng-config --I_opts}
bin_PROGRAMS = pngnq pngcomp
pngnq_SOURCES = pngnq.c neuquant32.c rwpng.c
pngcomp_SOURCES = pngcomp.c rwpng.c colorspace.c
