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

#include "occFunc.h"
#include "scan.h"
#include "vripGlobals.h"
#include "resample.h"



void
scanConvert(OccGrid *grid, OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf;
    int xx,yy,zz;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = grid->resolution;

    buf = grid->elems;
    for (zz = 0; zz < grid->zdim; zz++) {
	printf("\rTraversing slice %d of %d.", zz, grid->zdim-1);
	fflush(stdout);
	yOff = (grid->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < grid->ydim; yy++, yOff++) {	
	    xOff = (grid->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (xx = 0; xx < grid->xdim; xx++, xOff++, buf++) {
		resample(depthMap, xOff, yOff, &depth, &confidence);
		updateCell(grid->sliceOrigins[zz][2], depth, sampleSpacing,
			   confidence, buf);
	    }
	}
    }
    printf("\n");
}


void
scanConvertDragTails(OccGrid *grid, OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf;
    int xx,yy,zz;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = grid->resolution;

    buf = grid->elems;
    for (zz = 0; zz < grid->zdim; zz++) {
	printf("\rTraversing slice %d of %d.", zz, grid->zdim-1);
	fflush(stdout);
	yOff = (grid->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < grid->ydim; yy++, yOff++) {	
	    xOff = (grid->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (xx = 0; xx < grid->xdim; xx++, xOff++, buf++) {
		resample(depthMap, xOff, yOff, &depth, &confidence);
		updateCellForCarving(grid->sliceOrigins[zz][2], depth, sampleSpacing,
			   confidence, buf);
	    }
	}
    }
    printf("\n");
}


void
scanConvertTree(OccGrid *grid, OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf;
    int xx,yy,zz;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff;
    int xmin, xmax;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = grid->resolution;

    buf = grid->elems;
    for (zz = 0; zz < grid->zdim; zz++) {
	printf("\rTraversing slice %d of %d.", zz, grid->zdim-1);
	fflush(stdout);
	zmin = grid->sliceOrigins[zz][2] + C5/sampleSpacing;
	zmax = grid->sliceOrigins[zz][2] + C1/sampleSpacing;
	yOff = (grid->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < grid->ydim; yy++, yOff++) {	
	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns == 0)
		continue;
	    xOff = (grid->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, grid->xdim-1);
		xNewOff = xOff + xmin;
		buf = grid->elems + xmin + yy*grid->xdim + 
		    zz*grid->ydim*grid->xdim;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
		    resample(depthMap, xNewOff, yOff, &depth, &confidence);
		    updateCell(grid->sliceOrigins[zz][2], depth, 
			       sampleSpacing, confidence, buf);
		}
	    }
	}
    }
    printf("\n");
}

