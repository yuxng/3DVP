/*

Brian Curless

Computer Graphics Laboratory
Stanford University

---------------------------------------------------------------------

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include "DepthMap.h"

void
fillBackground(DepthMap *dm) 
{
    int xx, yy, xlim;
    float z, lastOutZ, lastInZ, newZ, slope;
    int in, lastOut, lastIn, i;

    for (yy = 0; yy < dm->ydim; yy++) {
	// Fill in the border
	xx = 0;
	z = dm->elems[xx+yy*dm->xdim].z;
	while (!IS_VALID_DEPTH(z) && xx < dm->xdim) {
	    xx++;
	    z = dm->elems[xx+yy*dm->xdim].z;
	}
	xlim = xx;

	if (xlim != dm->xdim) {
	    for (xx = 0; xx < xlim; xx++) {
		dm->elems[xx+yy*dm->xdim].z = z;
	    }

	    xx = dm->xdim - 1;
	    z = dm->elems[xx+yy*dm->xdim].z;
	    while (!IS_VALID_DEPTH(z) && xx >= 0) {
		xx--;
		z = dm->elems[xx+yy*dm->xdim].z;
	    }
	    xlim = xx;
	    
	    for (xx = xlim+1; xx < dm->xdim; xx++) {
		dm->elems[xx+yy*dm->xdim].z = z;
	    }
	}
	else {
	    for (xx = 0; xx < xlim; xx++) {
		dm->elems[xx+yy*dm->xdim].z = -1000;
	    }
	}
    }
}

void
makeSilhouette(DepthMap *dm) 
{
    int xx, yy;
    float z;

    for (yy = 0; yy < dm->ydim; yy++) {
       for (xx = 0; xx < dm->xdim; xx++) {
	  z = dm->elems[xx+yy*dm->xdim].z;
	  if (IS_VALID_DEPTH(z)) {
	     dm->elems[xx+yy*dm->xdim].z = -FAR_AWAY_DEPTH;
	  } else {
	     dm->elems[xx+yy*dm->xdim].z = FAR_AWAY_DEPTH/2;
	  }
       }
    }
}


void
fillGaps(DepthMap *dm) 
{
    int xx, yy, xlim;
    float z, lastOutZ, lastInZ, newZ, slope;
    int in, lastOut, lastIn, i;

    for (yy = 0; yy < dm->ydim; yy++) {

	// Fill in gaps with simple linear interpolation
	// between depths of adjacent valid pixels
	
	in = FALSE;
	lastOut = -1;

	for (xx = 0; xx < dm->xdim; xx++) {
	    z = dm->elems[xx+yy*dm->xdim].z;
	    if (IS_VALID_DEPTH(z) && !in) {

		lastIn = xx;
		// not used: // lastInZ = z;
		in = TRUE;

		if (lastOut == -1)
		    continue;

		slope = (z-lastOutZ)/(lastIn - lastOut);
		newZ = lastOutZ + slope;
		for (i = lastOut+1; i <lastIn; i++, newZ+=slope) {
		    dm->elems[i+yy*dm->xdim].z = newZ;
		}
	    } 
	    else if (!IS_VALID_DEPTH(z) && in) {		
		lastOut = xx-1;
		lastOutZ = dm->elems[xx-1+yy*dm->xdim].z;
		in = FALSE;
	    }
	}
    }
}


void
extendEdges(DepthMap *dm) 
{
    int xx, yy, xlim;
    float z, lastOutZ, lastInZ, newZ, slope;
    int in, lastOut, lastIn, i;

    for (yy = 0; yy < dm->ydim; yy++) {

	// Fill in gaps with simple linear interpolation
	// between depths of adjacent valid pixels
	
	in = FALSE;
	lastOut = -1;

	for (xx = 0; xx < dm->xdim; xx++) {
	    z = dm->elems[xx+yy*dm->xdim].z;
	    if (IS_VALID_DEPTH(z) && !in) {

		lastIn = xx;
		// not used: // lastInZ = z;
		in = TRUE;

		if (lastOut == -1)
		    continue;

		slope = (z-lastOutZ)/(lastIn - lastOut);
		newZ = lastOutZ + slope;
		for (i = lastOut+1; i <lastIn; i++, newZ+=slope) {
		    dm->elems[i+yy*dm->xdim].z = newZ;
		    dm->elems[i+yy*dm->xdim].conf = 0;
		}
	    } 
	    else if (!IS_VALID_DEPTH(z) && in) {		
		lastOut = xx-1;
		lastOutZ = dm->elems[xx-1+yy*dm->xdim].z;
		in = FALSE;
	    }
	}
    }
}
