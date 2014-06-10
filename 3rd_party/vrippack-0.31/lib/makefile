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

# Make the symbolic links necessary for this directory,
# since cvs doesn't save link files...

LIBS =	liblinear.a \
	libply.a \
	libsl.a

default: all

all: $(LIBS)

clean: 
	-rm -f *.o *~

clobber:
	-rm -f *.o *~ $(LIBS)

depend:
	echo > makedep
	makedepend -fmakedep -- $(CFLAGS) -- $(CSRCS)
	-rm makedep.bak

install: clobber depend all

liblinear.a:
	ln -s ../src/linear/liblinear.a liblinear.a

libply.a:
	ln -s ../src/libply/libply.a libply.a 

libsl.a:
	ln -s ../src/softrender/libsl.a libsl.a 
