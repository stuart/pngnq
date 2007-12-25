# Makefile for pngnq
#
#
# Copyright (C) 2004-2007 by Stuart Coyle
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation.  This software is provided "as is" without express or
# implied warranty.
#
# This makefile should work on most Unixy systems. 
# 
 
VERSION = 0.5.1

CC=gcc

# Distro options
# Relies on uname, otherwise provide your own...
OS = ${shell uname -s}
ARCH=${shell uname -m}
DISTFILES = LICENSE README README.pngcomp pngnq.1 pngnq pngcomp 

# Installation options
# Change these to suit your system's requirements
BINDIR=/usr/bin
MANDIR=/usr/share/man/man1
DOCDIR=/usr/share/doc/pngnq

# Build options
PNGINC = ${shell libpng-config --cflags}
PNGLIB = ${shell libpng-config --ldflags}

CFLAGS = -O3 -g -Wall -pedantic -I. -D "VERSION=\"$(VERSION)\"" $(PNGINC) -funroll-loops -fomit-frame-pointer 

LDFLAGS = $(PNGLIB) -lz

OBJS = pngnq.o rwpng.o neuquant32.o freegetopt/getopt.o
#############################################
# Targets
# Default just makes pngnq
#############################################
pngnq: $(OBJS) freegetopt
	$(CC) -o $@ $(OBJS) $(LDFLAGS) -D$(VERSION)

pngcomp: rwpng.o pngcomp.o colorspace.o freegetopt
	$(CC) -o $@ rwpng.o pngcomp.o freegetopt/getopt.o colorspace.o $(LDFLAGS)

freegetopt: 
	$(MAKE) -C freegetopt 

###############################################
# Phony targets 
###############################################
.PHONY: all clean dist bindist
all: pngnq pngcomp

clean:
	rm -f pngnq pngcomp *.o
	$(MAKE) -C freegetopt clean

# Make a distribution tar.gz
dist:	clean
	rm -f *~
	tar -cz -C ../ -f ../pngnq-$(VERSION)-src.tar.gz $(VERSION)

# Make a binary distribution .tar.gz
bindist: all
	tar -cz -f ../pngnq-$(VERSION)-$(ARCH)-$(OS).tar.gz $(DISTFILES)

# Install
install: all
	install -t ${BINDIR} pngnq pngcomp
	install -t ${MANDIR} pngnq.1 
	install -d ${DOCDIR} 
	install -t ${DOCDIR} README LICENSE README.pngcomp 

