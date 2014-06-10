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

HFILES = Linear.h \
	Matrix2f.h \
	Matrix3f.h \
	Matrix4f.h \
	Quaternion.h \
	Vec3f.h \
	ply.h \
	sl_export.H \
	sl_framebuffer.H \
	sl_texture.H \
	sl_val.H \
	sl_vertex.H 


default: all

all: $(HFILES)

clean: 
	-rm -f *.o *~

clobber:
	-rm -f *.o *~ $(HFILES)

depend:
	echo > makedep
	makedepend -fmakedep -- $(CFLAGS) -- $(SRCS)
	-rm makedep.bak

install: clobber depend all


Linear.h:
	ln -s ../src/linear/Linear.h Linear.h 

Matrix2f.h:
	ln -s ../src/linear/Matrix2f.h Matrix2f.h 

Matrix3f.h:
	ln -s ../src/linear/Matrix3f.h Matrix3f.h 

Matrix4f.h:
	ln -s ../src/linear/Matrix4f.h Matrix4f.h 

Quaternion.h:
	ln -s ../src/linear/Quaternion.h Quaternion.h 

Vec3f.h:
	ln -s ../src/linear/Vec3f.h Vec3f.h 

ply.h:
	ln -s ../src/libply/ply.h ply.h 

sl_export.H:
	ln -s ../src/softrender/sl_export.H sl_export.H 

sl_framebuffer.H:
	ln -s ../src/softrender/sl_framebuffer.H sl_framebuffer.H 

sl_texture.H:
	ln -s ../src/softrender/sl_texture.H sl_texture.H 

sl_val.H:
	ln -s ../src/softrender/sl_val.H sl_val.H 

sl_vertex.H:
	ln -s ../src/softrender/sl_vertex.H sl_vertex.H 
