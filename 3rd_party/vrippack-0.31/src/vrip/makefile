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

CSRCS = 

CXXSRCS = vripInit.cc vripMain.cc vripMiscCmds.cc OccGrid.cc \
	OccGridRLE.cc plyio.cc DepthMap.cc Mesh.cc softRenderGeom.cc \
	ChunkAllocator.cc ChunkLink.cc DepthMapAux.cc \
	vripFillCmds.cc vripFileCmds.cc vripGridCmds.cc vripRangeCmds.cc \
	vripGUICmds.cc vripGlobals.cc vripAux.cc BBox3f.cc rangePly.cc \
	linePersp.cc perspective.cc scan.cc scanRLE.cc scanLinePerspRLE.cc \
	configure.cc occFunc.cc resample.cc OccGridNormRLE.cc scanNormRLE.cc \
	vripPlyCmds.cc scanPerspRLE.cc

TARGETS = vrip vriphead

LIBS = -lply -llinear -ltk -ltcl -ldl \
	-lsl -lc -lm 

ROOT = ../..

-include ../compile-flags

vrip: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBPATHS) $(LIBS)

vriphead: vriphead.o
	$(CXX) -o $@ vriphead.o $(LIBPATHS) $(LIBS)

cysiltoply: cysiltoply.o strings.o
	$(CXX) -o $@ cysiltoply.o strings.o $(LIBPATHS) -limg -limage $(LIBS)

gltest: gltest.o
	$(CXX) -o $@ gltest.o -lgl

gltest2: gltest2.o
	$(CXX) -o $@ gltest2.o -lgl

