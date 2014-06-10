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

CSRCS = 

CXXSRCS = main.cc getinfo.cc init.cc edge.cc cube.cc quit.cc \
	fatal.cc slices.cc OccGridRLE.cc SectionRLE.cc \
        ChunkAllocator.cc ChunkLink.cc OccGrid.cc

TARGETS = vripmarch

ROOT = ../..

LIBS = -lmcfile -lply -lm -lc

-include ../compile-flags

OBJ2 = fromden.o
OBJ3 = polys.o mcfile.o
OBJ4 = mctoply.o mcfile.o

vripmarch: $(OBJS) libmcfile.a 
	$(CXX) -o $@ $(OBJS) $(LIBPATHS) $(LIBS)

polys: $(OBJ3)
	$(CC) $(CFLAGS) $(C++FLAGS) $(OBJ3) -o polys $(LIBPATHS) $(LIBS) 

mctoply: $(OBJ4)
	$(CC) $(CFLAGS) $(C++FLAGS) $(OBJ4) -o mctoply $(LIBPATHS) $(LIBS) 

fromshd: $(OBJ2)
	$(CC) $(CFLAGS) -o fromshd $(OBJ2)

fromden: $(OBJ2)
	$(CC) $(CFLAGS) -o fromden $(OBJ2) $(LIBPATHS) -llevden 

libmcfile.a:	libmcfile.a(mcfile.o)

libmcfile.a(mcfile.o):	mcfile.h

