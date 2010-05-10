AM_LDFLAGS = `libpng-config --ldflags` -lz
AM_CFLAGS = `libpng-config --I_opts` -Wall --pedantic -std=gnu99

bin_PROGRAMS = pngnq pngcomp
pngnq_SOURCES = pngnq.c neuquant32.c rwpng.c  neuquant32.h rwpng.h errors.h
pngcomp_SOURCES = pngcomp.c rwpng.c colorspace.c  colorspace.h
