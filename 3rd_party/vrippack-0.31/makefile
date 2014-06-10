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

# Make all subdirectories
# Requires gnu make (e.g. /usr/common/bin/make)
# Run "make install" to make clobber, depend, all

SUBDIRS = \
	include \
	src \
	lib \
	bin \

default:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) default); done

clean:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean); done

clobber:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clobber); done
	cd include; make
	cd lib; make

depend:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) depend); done

install:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) install); done

all:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) all); done

tar:
	cd .. && tar zcvf vrip_plytools_src.tar.gz `find ply -name CVS -prune -o -type f -print`
