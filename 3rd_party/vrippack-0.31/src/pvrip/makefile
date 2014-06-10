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

CXXSRCS = \
	plycullmaxx.cc \
	plymerge.cc


TARGETS = \
	plycullmaxx \
	plymerge

# Targets to be built on SGI only (need SGI libraries)...
SGICXXSRCS = matrix2quat.cc
SGITARGETS = matrix2quat

LIBS = -lm -lply

ROOT = ../..

-include ../compile-flags

matrix2quat: matrix2quat.o
	$(CXX) -o $@ matrix2quat.o $(LIBPATHS) $(LIBS) -lInventor 

plycullmaxx: plycullmaxx.o
	$(CXX) -o $@ plycullmaxx.o $(LIBPATHS) $(LIBS) 

plymerge: plymerge.o
	$(CXX) -o $@ plymerge.o $(LIBPATHS) $(LIBS) 

