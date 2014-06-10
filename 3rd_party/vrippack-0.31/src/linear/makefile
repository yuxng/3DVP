#  
#  Brian Curless
#  
#  Computer Graphics Laboratory
#  Stanford University
#  
#  ---------------------------------------------------------------------
#  
#  Copyright (1997-2001) The Board of Trustees of the Leland Stanford Junior
#  University. Except for commercial resale, lease, license or other
#  commercial transactions, permission is hereby given to use, copy,
#  modify this software for academic purposes only.  No part of this
#  software or any derivatives thereof may be used in the production of
#  computer models for resale or for use in a commercial
#  product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
#  CONCERNING THIS SOFTWARE.  No support is implied or provided.
#  

CXXREGFILES = verify.cc

CXXLIBFILES = Vec3f.cc Matrix2f.cc Matrix3f.cc Matrix4f.cc \
		Quaternion.cc

CXXFILES = $(CXXREGFILES) $(CXXLIBFILES)

CFILES = 

TARGETS = liblinear.a verify

ROOT = ../..

-include ../compile-flags

CXXLLDLIBS =  -L. -L$(ROOT)/lib -llinear -lc -lm

liblinear.a: $(CXXLIBOBJS)
	 ar -rc liblinear.a $(CXXLIBOBJS)

verify: verify.o liblinear.a
	$(CXX) -o $@ verify.o $(CXXLLDLIBS)

