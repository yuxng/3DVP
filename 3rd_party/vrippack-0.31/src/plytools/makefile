#! gmake

#
#  Brian Curless
#  
#  Computer Graphics Laboratory
#  Stanford University
#  
#  ---------------------------------------------------------------------
#  
#  Copyright (1997) The Board of Trustees of the Leland Stanford Junior
#  University. Except for commercial resale, lease, license or other
#  commercial transactions, permission is hereby given to use, copy,
#  modify this software for academic purposes only.  No part of this
#  software or any derivatives thereof may be used in the production of
#  computer models for resale or for use in a commercial
#  product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
#  CONCERNING THIS SOFTWARE.  No support is implied or provided.
#  

CSRCS = \
	ply2verts.c  plycomps.c    plycrunch.c  plynormals.c \
	plybbox.c    plyconvert.c  plyflip.c    plyremove.c \
	plybound.c   plyhead.c     plyunshared.c

CXXSRCS = \
	plyarea.cc      plyxform.cc plyconfthresh.cc\
	plyaddinfo.cc    plymirror.cc \
	plystrip.cc      plyconfcrop.cc  plysubtract.cc \
	plyshared.cc     plyvolume.cc	 plydice.cc \
	plyplanecrop.cc xyz2ply.cc	 plysubdiv.cc

PERLSRCS = \
	plys2set	plysets2conf	plysetlevel \
	ply2crunchset   ply2bboxcube

TARGETS = \
	plybbox      plyflip        plyvolume \
	plybound     plyhead        ply2asc \
	plycomps     plynormals     plyunshared \
	ply2verts    plyremove      plyxform \
	plyaddinfo   plyshared      ply2bin \
	plyarea      plycrunch	    plymirror \
	plydice	     plyconfcrop    plysubtract \
	plyplanecrop plysubdiv    plystrip       xyz2ply \
	plyconfthresh

SGICXXSRCS = iv2ply.cc ply2iv.cc

SGITARGETS = \
	iv2ply ply2iv

LIBS = -lply -lm

ROOT = ../..

-include ../compile-flags

iv2ply: iv2ply.o
	$(CXX) -o $@ iv2ply.o $(LIBPATHS) -lInventor $(LIBS)

ply2iv: ply2iv.o
	$(CXX) -o $@ ply2iv.o $(LIBPATHS) -lInventor $(LIBS)

plyaddinfo: plyaddinfo.o
	$(CXX) -o $@ plyaddinfo.o $(LIBPATHS) $(LIBS)

plyarea: plyarea.o
	$(CXX) -o $@ plyarea.o $(LIBPATHS) $(LIBS)

plyhead: plyhead.o
	$(CC) -o $@ plyhead.o $(LIBPATHS) $(LIBS)

plyflip: plyflip.o
	$(CC) -o $@ plyflip.o $(LIBPATHS) $(LIBS)

plybbox: plybbox.o
	$(CC) -o $@ plybbox.o $(LIBPATHS) $(LIBS)

plynormals: plynormals.o
	$(CC) -o $@ plynormals.o $(LIBPATHS) $(LIBS)

plycrunch: plycrunch.o
	$(CC) -o $@ plycrunch.o $(LIBPATHS) $(LIBS)

plyshared: plyshared.o
	$(CXX) -o $@ plyshared.o $(LIBPATHS) $(LIBS)

plysubtract: plysubtract.o
	$(CXX) -o $@ plysubtract.o $(LIBPATHS) $(LIBS)

plyunshared: plyunshared.o
	$(CC) -o $@ plyunshared.o $(LIBPATHS) $(LIBS)

plyremove: plyremove.o
	$(CC) -o $@ plyremove.o $(LIBPATHS) $(LIBS)

plybound: plybound.o
	$(CC) -o $@ plybound.o $(LIBPATHS) $(LIBS)

plycomps: plycomps.o
	$(CC) -o $@ plycomps.o $(LIBPATHS) $(LIBS)

plyvolume: plyvolume.o
	$(CXX) -o $@ plyvolume.o $(LIBPATHS) $(LIBS) -llinear

plyxform: plyxform.o
	$(CXX) -o $@ plyxform.o $(LIBPATHS) -llinear $(LIBS)

plydice: plydice.o
	$(CXX) -o $@ plydice.o $(LIBPATHS) -llinear $(LIBS)

plymirror: plymirror.o
	$(CXX) -o $@ plymirror.o $(LIBPATHS) -llinear $(LIBS)

plystrip: plystrip.o
	$(CXX) -o $@ plystrip.o $(LIBPATHS) $(LIBS)

plyconfcrop: plyconfcrop.o
	$(CXX) -o $@ plyconfcrop.o $(LIBPATHS) -llinear $(LIBS)

plyconfthresh: plyconfthresh.o
	$(CXX) -o $@ plyconfthresh.o $(LIBPATHS) -llinear $(LIBS)

plyplanecrop: plyplanecrop.o
	$(CXX) -o $@ plyplanecrop.o $(LIBPATHS) -llinear $(LIBS)

ply2asc.o: plyconvert.c
	$(CC) -c plyconvert.c $(CFLAGS) -DWRITE_ASCII
	mv plyconvert.o ply2asc.o

ply2bin.o: plyconvert.c
	$(CC) -c plyconvert.c $(CFLAGS) -DWRITE_BINARY
	mv plyconvert.o ply2bin.o

ply2asc: ply2asc.o
	$(CC) -o $@ ply2asc.o $(LIBPATHS) $(LIBS)

ply2bin: ply2bin.o
	$(CC) -o $@ ply2bin.o $(LIBPATHS) $(LIBS)

ply2verts: ply2verts.o
	$(CC) -o $@ ply2verts.o $(LIBPATHS) $(LIBS)

xyz2ply: xyz2ply.o
	$(CXX) -o $@ xyz2ply.o $(LIBPATHS)

plysubdiv: plysubdiv.o
	$(CXX) -o $@ plysubdiv.o $(LIBPATHS) $(LIBS)

