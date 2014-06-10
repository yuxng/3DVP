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


#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "vrip.h"
#include "vripGlobals.h"
#include "linePersp.h"
#include "resample.h"
#include "occFunc.h"
#include "scanNormRLE.h"


void
scanConvertTree(OccGridNormRLE *gridIn, OccGridNormRLE *gridOut,
		 OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccNormElement *buf, *scanline;
    int xx,yy,zz;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff;
    int xmin, xmax;
    OccScanlineNormRLE *rleScanline;
    Vec3f norm;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	printf("\rTraversing slice %d of %d.", zz, gridIn->zdim-1);
	fflush(stdout);

	// Something is really backwards here!

/*
	zmin = gridIn->sliceOrigins[zz][2] + C5/sampleSpacing;
	zmax = gridIn->sliceOrigins[zz][2] + 2*C1/sampleSpacing;
*/
	zmin = gridIn->sliceOrigins[zz][2] - C1/sampleSpacing;
	zmax = gridIn->sliceOrigins[zz][2] - C5/sampleSpacing;
	yOff = (gridIn->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < gridIn->ydim; yy++, yOff++) {	
	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns == 0) {
		rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		continue;
	    }
	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    scanline = gridIn->getScanline(yy,zz);
	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, gridIn->xdim-1);
		xNewOff = xOff + xmin;
		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
		    resampleNorm(depthMap, xNewOff, yOff, 
				 &depth, &confidence, norm);

		    updateCell(gridIn->sliceOrigins[zz][2], depth, norm,
			       sampleSpacing, confidence, buf);
		}
	    }
	    
	    gridOut->putScanline(scanline,yy,zz);

	}
    }
    printf("\n");
}


void
scanConvertTree2(OccGridNormRLE *gridIn, OccGridNormRLE *gridOut,
		OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccNormElement *buf, *scanline;
    int xx,yy,zz;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff;
    int xmin, xmax;
    OccScanlineNormRLE *rleScanline;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	printf("\rTraversing slice %d of %d.", zz, gridIn->zdim-1);
	fflush(stdout);

	// Something is really backwards here!

/*
	zmin = gridIn->sliceOrigins[zz][2] + C5/sampleSpacing;
	zmax = gridIn->sliceOrigins[zz][2] + 2*C1/sampleSpacing;
*/
	zmin = gridIn->sliceOrigins[zz][2] - C1/sampleSpacing;
	zmax = gridIn->sliceOrigins[zz][2] - C5/sampleSpacing;
	yOff = (gridIn->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < gridIn->ydim; yy++, yOff++) {	

	    // Points near the surface
	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns != 0) {
		xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
		scanline = gridIn->getScanline(yy,zz);
		for (int i = 0; i < numRuns; i++) {
		    xmin = int(depthRuns[2*i] - xOff);
		    xmin = MAX(xmin, 0);
		    xmax = int(ceil(depthRuns[2*i+1] - xOff));
		    xmax = MIN(xmax, gridIn->xdim-1);
		    xNewOff = xOff + xmin;
		    buf = scanline + xmin;
		    for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
			resampleForCarving(depthMap, xNewOff, 
					    yOff, &depth, &confidence);
			updateCellForCarving(gridIn->sliceOrigins[zz][2], 
					     depth, 
					     sampleSpacing, confidence, buf);
		    }
		}
	    }

	    oldNumRuns = numRuns;

	    // Points in front of the surface
	    depthRuns = depthMap->getDepthRunsUpperBound
		(yOff, zmin, &numRuns);

	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		continue;
	    } else if (numRuns == 0) {
		gridOut->putScanline(scanline,yy,zz);
		continue;
	    } else if (oldNumRuns == 0) {
		scanline = gridIn->getScanline(yy,zz);
	    }

	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, gridIn->xdim-1);
		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, buf++) {
		    if (buf->totalWeight == 0) {
			buf->value = 0;
		    }
		}
	    }
	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    printf("\n");
}

