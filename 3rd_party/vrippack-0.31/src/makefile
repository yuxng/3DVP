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

SUBDIRS = \
	libply \
	linear \
	march \
	plyclean \
	plytools \
	softrender \
	vrip \
	pvrip

default:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) default); done

clean:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean); done

clobber:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clobber); done

depend:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) depend); done

cleandepend:
	@for d in $(SUBDIRS); do (cd $$d; rm makedep); done

install:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) install); done

all:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) all); done

