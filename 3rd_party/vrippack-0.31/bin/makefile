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

# Make the symbolic links necessary for this directory,
# since cvs doesn't save link files...

PROGS = \
	crunch.tcl \
	ply2asc \
	ply2bin \
	ply2verts \
	plyaddinfo \
	plyarea \
	plybbox \
	plybound \
	plycomps \
	plycrunch \
	plyflip \
	plyhead \
	plynormals \
	plyremove \
	plys2set \
	plysets2conf \
	plysetlevel \
	ply2bboxcube \
	ply2crunchset \
	plyshared \
	plystrip \
	plysubtract \
	plyunshared \
	plyusages \
	plyxform \
	plydice \
	plymirror \
	plyvolume \
	plysubdiv \
	xyz2ply \
	mm2plys \
	vrip \
	vriphead \
	vripmarch \
	vripnew \
	vripslicer \
	vripsurf \
	vripupdate \
	loadbalance \
	permute \
	plycull \
	plycullmaxx \
	plymerge \
	pvrip1 \
	pvrip2 \
	pvripsplit \
	vripsplit \
	vripsubvollist \
	plyclean \
	plygridtomesh \
	$(NULL)

default: all

all: $(PROGS)

clean: 
	-rm -f *.o *~

clobber:
	-rm -f *.o *~ $(PROGS)

depend:
	echo > makedep
	makedepend -fmakedep -- $(CFLAGS) -- $(CSRCS)
	-rm makedep.bak

install: clobber depend all

# Get OS uname
UNAME := $(shell uname -o)

ifeq ($(UNAME), Cygwin)

    crunch.tcl:
	    cp ../src/plytools/crunch.tcl crunch.tcl

    ply2asc:
	    cp ../src/plytools/ply2asc.exe .

    ply2bin:
	    cp ../src/plytools/ply2bin.exe .

    ply2bboxcube:
	    cp ../src/plytools/ply2bboxcube .

    ply2crunchset:
	    cp ../src/plytools/ply2crunchset .

    ply2verts:
	    cp ../src/plytools/ply2verts.exe .

    plyaddinfo:
	    cp ../src/plytools/plyaddinfo.exe .

    plyarea:
	    cp ../src/plytools/plyarea.exe .

    plybbox:
	    cp ../src/plytools/plybbox.exe .

    plybound:
	    cp ../src/plytools/plybound.exe .

    plycomps:
	    cp ../src/plytools/plycomps.exe .

    plycrunch:
	    cp ../src/plytools/plycrunch.exe .

    plyflip:
	    cp ../src/plytools/plyflip.exe .

    plyhead:
	    cp ../src/plytools/plyhead.exe .

    plynormals:
	    cp ../src/plytools/plynormals.exe .

    plyremove:
	    cp ../src/plytools/plyremove.exe .

    plys2set:
	    cp ../src/plytools/plys2set .

    plysets2conf:
	    cp ../src/plytools/plysets2conf .

    plysetlevel:
	    cp ../src/plytools/plysetlevel .

    plyshared:
	    cp ../src/plytools/plyshared.exe .

    plysubtract:
	    cp ../src/plytools/plysubtract.exe .

    plystrip:
	    cp ../src/plytools/plystrip.exe .

    plyunshared:
	    cp ../src/plytools/plyunshared.exe .

    plyusages:
	    cp ../src/plytools/plyusages.csh plyusages

    plyxform:
	    cp ../src/plytools/plyxform.exe .

    plydice:
	    cp ../src/plytools/plydice.exe .

    plymirror:
	    cp ../src/plytools/plymirror.exe .

    plyvolume:
	    cp ../src/plytools/plyvolume.exe .

    plysubdiv:
	    cp ../src/plytools/plysubdiv.exe .

    xyz2ply:
	    cp ../src/plytools/xyz2ply.exe .

    mm2plys:
	    cp ../src/plytools/mm2plys .

    vrip:
	    cp ../src/vrip/vrip.csh vrip

    vriphead:
	    cp ../src/vrip/vriphead.exe .

    vripmarch:
	    cp ../src/march/vripmarch.exe .

    vripnew:
	    cp ../src/vrip/vripnew.csh vripnew

    vripslicer:
	    cp ../src/vrip/vripslicer.csh vripslicer

    vripsurf:
	    cp ../src/vrip/vripsurf.csh vripsurf

    vripupdate:
	    cp ../src/vrip/vripupdate.csh vripupdate

    loadbalance:
	    cp ../src/pvrip/loadbalance .

    permute:
	    cp ../src/pvrip/permute .

    plycull:
	    cp ../src/pvrip/plycull .

    plycullmaxx:
	    cp ../src/pvrip/plycullmaxx.exe .

    plymerge:
	    cp ../src/pvrip/plymerge.exe .

    pvrip1:
	    cp ../src/pvrip/pvrip1 .

    pvrip2:
	    cp ../src/pvrip/pvrip2 .

    pvripsplit:
	    cp ../src/pvrip/pvripsplit .

    vripsplit:
	    cp ../src/pvrip/vripsplit .

    vripsubvollist:
	    cp ../src/pvrip/vripsubvollist .

    plyclean:
	    cp ../src/plyclean/plyclean.exe .

    plygridtomesh:
	    cp ../src/vrip/plygridtomesh .

else

    crunch.tcl:
	    ln -s ../src/plytools/crunch.tcl crunch.tcl

    ply2asc:
	    ln -s ../src/plytools/ply2asc ply2asc

    ply2bin:
	    ln -s ../src/plytools/ply2bin ply2bin

    ply2bboxcube:
	    ln -s ../src/plytools/ply2bboxcube ply2bboxcube

    ply2crunchset:
	    ln -s ../src/plytools/ply2crunchset ply2crunchset

    ply2verts:
	    ln -s ../src/plytools/ply2verts ply2verts

    plyaddinfo:
	    ln -s ../src/plytools/plyaddinfo plyaddinfo

    plyarea:
	    ln -s ../src/plytools/plyarea plyarea

    plybbox:
	    ln -s ../src/plytools/plybbox plybbox

    plybound:
	    ln -s ../src/plytools/plybound plybound

    plycomps:
	    ln -s ../src/plytools/plycomps plycomps

    plycrunch:
	    ln -s ../src/plytools/plycrunch plycrunch

    plyflip:
	    ln -s ../src/plytools/plyflip plyflip

    plyhead:
	    ln -s ../src/plytools/plyhead plyhead

    plynormals:
	    ln -s ../src/plytools/plynormals plynormals

    plyremove:
	    ln -s ../src/plytools/plyremove plyremove

    plys2set:
	    ln -s ../src/plytools/plys2set plys2set

    plysets2conf:
	    ln -s ../src/plytools/plysets2conf plysets2conf

    plysetlevel:
	    ln -s ../src/plytools/plysetlevel plysetlevel

    plyshared:
	    ln -s ../src/plytools/plyshared plyshared

    plysubtract:
	    ln -s ../src/plytools/plysubtract plysubtract

    plystrip:
	    ln -s ../src/plytools/plystrip plystrip

    plyunshared:
	    ln -s ../src/plytools/plyunshared plyunshared

    plyusages:
	    ln -s ../src/plytools/plyusages.csh plyusages

    plyxform:
	    ln -s ../src/plytools/plyxform plyxform

    plydice:
	    ln -s ../src/plytools/plydice plydice

    plymirror:
	    ln -s ../src/plytools/plymirror plymirror

    plyvolume:
	    ln -s ../src/plytools/plyvolume plyvolume

    plysubdiv:
	    ln -s ../src/plytools/plysubdiv plysubdiv

    xyz2ply:
	    ln -s ../src/plytools/xyz2ply xyz2ply

    mm2plys:
	    ln -s ../src/plytools/mm2plys mm2plys

    vrip:
	    ln -s ../src/vrip/vrip.csh vrip

    vriphead:
	    ln -s ../src/vrip/vriphead vriphead

    vripmarch:
	    ln -s ../src/march/vripmarch vripmarch

    vripnew:
	    ln -s ../src/vrip/vripnew.csh vripnew

    vripslicer:
	    ln -s ../src/vrip/vripslicer.csh vripslicer

    vripsurf:
	    ln -s ../src/vrip/vripsurf.csh vripsurf

    vripupdate:
	    ln -s ../src/vrip/vripupdate.csh vripupdate

    loadbalance:
	    ln -s ../src/pvrip/loadbalance loadbalance

    permute:
	    ln -s ../src/pvrip/permute permute

    plycull:
	    ln -s ../src/pvrip/plycull plycull

    plycullmaxx:
	    ln -s ../src/pvrip/plycullmaxx plycullmaxx

    plymerge:
	    ln -s ../src/pvrip/plymerge plymerge

    pvrip1:
	    ln -s ../src/pvrip/pvrip1 pvrip1

    pvrip2:
	    ln -s ../src/pvrip/pvrip2 pvrip2

    pvripsplit:
	    ln -s ../src/pvrip/pvripsplit pvripsplit

    vripsplit:
	    ln -s ../src/pvrip/vripsplit vripsplit

    vripsubvollist:
	    ln -s ../src/pvrip/vripsubvollist vripsubvollist

    plyclean:
	    ln -s ../src/plyclean/plyclean plyclean

    plygridtomesh:
	    ln -s ../src/vrip/plygridtomesh plygridtomesh

endif


