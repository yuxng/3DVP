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
#include "scanLinePerspRLE.h"


static OccScanlineRLE *theOutScanline;
static OccGridRLE *theInGrid;
static int currentType;
static RunLength currentLength;
static OccElement lastElement;


static void initScanline(OccGridRLE *, OccScanlineRLE *);
static void appendScanlinePortion(OccGridRLE *inGrid, int inType, 
				  OccElement *inElement, RunLength length);
static void appendScanline(OccElement *inElement);
static void sealScanline();




void
scanConvertLinePersp(OccGridRLE *gridIn, OccGridRLE *gridOut, 
		     OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    Vec3f pos, newpos;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	if (!Quiet)
	    printf("\rTraversing slice %d of %d.", zz, gridIn->zdim-1);
	fflush(stdout);
	yOff = (gridIn->sliceOrigins[zz][1] - depthMap->origin[1])/res;
	for (yy = 0; yy < gridIn->ydim; yy++, yOff++) {	
	    buf = scanline = gridIn->getScanline(yy,zz);
	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (xx = 0; xx < gridIn->xdim; xx++, xOff++, buf++) {

		pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
			     yy*res+gridIn->sliceOrigins[zz][1],
			     gridIn->sliceOrigins[zz][2]);
		applyLinePersp(pos, newpos);
		xOff = (newpos.x - depthMap->origin[0])/res;
		yOff = (newpos.y - depthMap->origin[1])/res;

#if 1
		resampleForCarving(depthMap, xOff, yOff, 
				   &depth, &confidence);

		// Using the full-blown filler!
		updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
				     sampleSpacing, confidence, buf);
#else

		resample(depthMap, xOff, yOff, &depth, &confidence);
		updateCell(gridIn->sliceOrigins[zz][2], depth, 
			   sampleSpacing, confidence, buf);

#endif

	    }
	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}


void
scanConvertLinePerspTree(OccGridRLE *gridIn, OccGridRLE *gridOut, 
			 OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    Vec3f pos, newpos;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff, y1, y2;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	if (!Quiet)
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

	    pos.setValue(0*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y1 = (newpos.y - depthMap->origin[1])/res;

	    pos.setValue(gridIn->xdim*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y2 = (newpos.y - depthMap->origin[1])/res;

	    // Split the y-value down the middle
	    yOff = (y1+y2)/2;

	    // Huber fix of 4/5/01
	    // look for areas with depth between zmin and zmax in this row
     	    if ((int(yOff) >= 0) && (int(yOff) < depthMap->ydim)) {
	       depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
     	    } else {
	       numRuns = 0;
	       printf("y out of range (z=%d y=%d offset=%f)\n", zz, yy, yOff);
     	    }

	    if (numRuns == 0) {
		rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		continue;

	    }
	    scanline = gridIn->getScanline(yy,zz);
	    for (int i = 0; i < numRuns; i++) {

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = MIN(xmin, int((newpos.x - 
				      gridIn->sliceOrigins[zz][0])/res));
		xmin = MAX(xmin, 0);


		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = int(ceil((newpos.x - 
				 gridIn->sliceOrigins[zz][0])/res));

		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = MAX(xmax, 
			   int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res)));
		xmax = MIN(xmax, gridIn->xdim-1);

		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

		    pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				 yy*res+gridIn->sliceOrigins[zz][1],
				 gridIn->sliceOrigins[zz][2]);
		    applyLinePersp(pos, newpos);
		    xOff = (newpos.x - depthMap->origin[0])/res;
		    yOff = (newpos.y - depthMap->origin[1])/res;

		    resample(depthMap, xOff, yOff, &depth, &confidence);
		    updateCell(gridIn->sliceOrigins[zz][2], depth, 
			       sampleSpacing, confidence, buf);
		}
	    }
	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}



void
scanConvertLinePerspTreeDragTails(OccGridRLE *gridIn, OccGridRLE *gridOut, 
				  OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    Vec3f pos, newpos;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff, y1, y2;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	if (!Quiet)
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

	    pos.setValue(0*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y1 = (newpos.y - depthMap->origin[1])/res;

	    pos.setValue(gridIn->xdim*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y2 = (newpos.y - depthMap->origin[1])/res;

	    // Split the y-value down the middle
	    yOff = (y1+y2)/2;

	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns != 0) {
		scanline = gridIn->getScanline(yy,zz);
		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);

		    buf = scanline + xmin;
		    for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

			pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				     yy*res+gridIn->sliceOrigins[zz][1],
				     gridIn->sliceOrigins[zz][2]);
			applyLinePersp(pos, newpos);
			xOff = (newpos.x - depthMap->origin[0])/res;
			yOff = (newpos.y - depthMap->origin[1])/res;

			resampleForCarving(depthMap, xOff, yOff, 
					   &depth, &confidence);
			updateCellForCarving(gridIn->sliceOrigins[zz][2], 
					     depth, 
					     sampleSpacing, confidence, buf);
		    }
		}
	    }
	    oldNumRuns = numRuns;

	    // Points in front of the surface
	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsUpperBound
		(yOff, zmin, &numRuns);

	    int skip = 0;
	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		skip = 1;
	    } else if (numRuns == 0) {
		gridOut->putScanline(scanline,yy,zz);
		skip = 1;
	    } else if (oldNumRuns == 0) {
		scanline = gridIn->getScanline(yy,zz);
		skip = 0;
	    }

	    if (!skip) {
		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);

		    buf = scanline + xmin;
		    for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
			if (buf->totalWeight == 0) {
			    buf->value = 0;
			}
		    }
		}
	    }

	    oldNumRuns += numRuns;

	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsEdges
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

	    for (int i = 0; i < numRuns; i++) {

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = MIN(xmin, int((newpos.x - 
				      gridIn->sliceOrigins[zz][0])/res));
		xmin = MAX(xmin, 0);


		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = int(ceil((newpos.x - 
				 gridIn->sliceOrigins[zz][0])/res));

		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = MAX(xmax, 
			   int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res)));
		xmax = MIN(xmax, gridIn->xdim-1);

		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

		    pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				 yy*res+gridIn->sliceOrigins[zz][1],
				 gridIn->sliceOrigins[zz][2]);
		    applyLinePersp(pos, newpos);
		    xOff = (newpos.x - depthMap->origin[0])/res;
		    yOff = (newpos.y - depthMap->origin[1])/res;

		    resampleForCarving(depthMap, xOff, yOff, 
				       &depth, &confidence);
		    updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
					 sampleSpacing, confidence, buf);
		}
	    }

	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}


void
scanConvertLinePerspTreeTailsOnly(OccGridRLE *gridIn, OccGridRLE *gridOut, 
				  OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    Vec3f pos, newpos;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff, y1, y2;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	if (!Quiet)
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

	    pos.setValue(0*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y1 = (newpos.y - depthMap->origin[1])/res;

	    pos.setValue(gridIn->xdim*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y2 = (newpos.y - depthMap->origin[1])/res;

	    // Split the y-value down the middle
	    yOff = (y1+y2)/2;

	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns != 0) {
		scanline = gridIn->getScanline(yy,zz);
		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);

		    buf = scanline + xmin;
		    for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

			pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				     yy*res+gridIn->sliceOrigins[zz][1],
				     gridIn->sliceOrigins[zz][2]);
			applyLinePersp(pos, newpos);
			xOff = (newpos.x - depthMap->origin[0])/res;
			yOff = (newpos.y - depthMap->origin[1])/res;

			resampleForCarving(depthMap, xOff, yOff, 
					   &depth, &confidence);

			// Setting confidence to zero only allows for carving
			confidence = 0;
			updateCellForCarving(gridIn->sliceOrigins[zz][2], 
					     depth, 
					     sampleSpacing, confidence, buf);
		    }
		}
	    }
	    oldNumRuns = numRuns;

	    // Points in front of the surface
	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsUpperBound
		(yOff, zmin, &numRuns);

	    int skip = 0;
	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		skip = 1;
	    } else if (numRuns == 0) {
		gridOut->putScanline(scanline,yy,zz);
		skip = 1;
	    } else if (oldNumRuns == 0) {
		scanline = gridIn->getScanline(yy,zz);
		skip = 0;
	    }

	    if (!skip) {
		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);

		    buf = scanline + xmin;
		    for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
			if (buf->totalWeight == 0) {
			    buf->value = 0;
			}
		    }
		}
	    }

	    oldNumRuns += numRuns;

	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsEdges
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

	    for (int i = 0; i < numRuns; i++) {

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = MIN(xmin, int((newpos.x - 
				      gridIn->sliceOrigins[zz][0])/res));
		xmin = MAX(xmin, 0);


		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = int(ceil((newpos.x - 
				 gridIn->sliceOrigins[zz][0])/res));

		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = MAX(xmax, 
			   int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res)));
		xmax = MIN(xmax, gridIn->xdim-1);

		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

		    pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				 yy*res+gridIn->sliceOrigins[zz][1],
				 gridIn->sliceOrigins[zz][2]);
		    applyLinePersp(pos, newpos);
		    xOff = (newpos.x - depthMap->origin[0])/res;
		    yOff = (newpos.y - depthMap->origin[1])/res;


		    resampleForCarving(depthMap, xOff, yOff, 
				       &depth, &confidence);

		    // Setting confidence to zero only allows for carving
		    confidence = 0;
		    updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
					 sampleSpacing, confidence, buf);
		}
	    }

	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}



void
scanConvertLinePerspTreeTailsOnlyFast(OccGridRLE *gridIn, OccGridRLE *gridOut, 
				      OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline, outElement;
    int xx,yy,zz;
    Vec3f pos, newpos;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff, y1, y2;
    int xmin, xmax;
    OccScanlineRLE *rleScanline, *outScanline, *inScanline, *tempScanline;

    inScanline = new OccScanlineRLE(gridIn->xdim);
    outScanline = new OccScanlineRLE(gridIn->xdim);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < gridIn->zdim; zz++) {
	if (!Quiet)
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

	    pos.setValue(0*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y1 = (newpos.y - depthMap->origin[1])/res;

	    pos.setValue(gridIn->xdim*res+gridIn->sliceOrigins[zz][0],
			 yy*res+gridIn->sliceOrigins[zz][1],
			 gridIn->sliceOrigins[zz][2]);
	    applyLinePersp(pos, newpos);
	    y2 = (newpos.y - depthMap->origin[1])/res;

	    // Split the y-value down the middle
	    yOff = (y1+y2)/2;

	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns != 0) {

		gridIn->setScanline(yy, zz);
		RunLength inLength = gridIn->getNextLength();
		int inType = gridIn->getRunType(&inLength);
		int nextInOffset = inLength;
		OccElement *inElement;

		if (inType == OccGridRLE::CONSTANT_DATA)
		    inElement = gridIn->getNextElement();

		// Prepare output scanline
		initScanline(gridIn, outScanline);
		xx = 0;

		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);

		    // Copy over all whole runs that precede the current xmin

		    while (nextInOffset < xmin) {
			if (nextInOffset != xx) {
			    appendScanlinePortion(gridIn, inType, inElement, 
						  nextInOffset - xx);
			}
			xx = nextInOffset;

			// Update for next run
			inLength = gridIn->getNextLength();
			inType = gridIn->getRunType(&inLength);
			nextInOffset += inLength; 
			if (inType == OccGridRLE::CONSTANT_DATA) {
			    inElement = gridIn->getNextElement();
			}
		    }
		

		    // Copy over the remaining portion of the first
		    // overlapping run

		    if (xx != xmin) {
			appendScanlinePortion(gridIn, inType, inElement, 
					      xmin - xx);
		    }

		    xx = xmin;
		    
		    // Update the output based on new data combined with
		    //  old data

		    while (xx <= xmax) {

			// If old input run is finished, start a new one

			if (xx == nextInOffset) {
			    inLength = gridIn->getNextLength();
			    inType = gridIn->getRunType(&inLength);
			    nextInOffset += inLength; 
			    if (inType == OccGridRLE::CONSTANT_DATA) {
				inElement = gridIn->getNextElement();
			    }
			}

			// If varying run, get next element

			if (inType == OccGridRLE::VARYING_DATA) {
			    inElement = gridIn->getNextElement();
			}

			// Copy over element and update

			outElement = *inElement;

			pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				     yy*res+gridIn->sliceOrigins[zz][1],
				     gridIn->sliceOrigins[zz][2]);
			applyLinePersp(pos, newpos);
			xOff = (newpos.x - depthMap->origin[0])/res;
			yOff = (newpos.y - depthMap->origin[1])/res;

			resampleForCarving(depthMap, xOff, yOff, 
					   &depth, &confidence);

			// Setting confidence to zero only allows for carving
			confidence = 0;
			updateCellForCarving(gridIn->sliceOrigins[zz][2], 
					     depth, 
					     sampleSpacing, confidence, 
					     &outElement);

			// Write element to the output grid
			appendScanline(&outElement);

			xNewOff++;
			xx++;
		    }
		}

		// Finish up any remaining runs

		while (xx < gridOut->xdim) {
		    if (nextInOffset != xx) {
			appendScanlinePortion(gridIn, inType, inElement, 
					      nextInOffset - xx);
		    }
		    xx = nextInOffset;

		    // Update for next run
		    inLength = gridIn->getNextLength();
		    inType = gridIn->getRunType(&inLength);

		    if (inType == OccGridRLE::END_OF_RUN) {
			break;
		    }

		    nextInOffset += inLength; 
		    if (inType == OccGridRLE::CONSTANT_DATA) {
			inElement = gridIn->getNextElement();
		    }
		}

		// Seal off the old run
		sealScanline();

		gridOut->copyScanline(outScanline, yy, zz);
		scanline = gridOut->getScanline(yy, zz);

		gridIn->copyScanline(outScanline, yy, zz);
	    }

	    oldNumRuns = numRuns;

	    // Points in front of the surface
	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsUpperBound
		(yOff, zmin, &numRuns);

	    int skip = 0;
	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		skip = 1;
	    } else if (numRuns == 0) {
		gridOut->copyScanline(outScanline,yy,zz);
		skip = 1;
	    } else if (oldNumRuns == 0) {
		skip = 0;
	    }

	    if (!skip) {

		gridIn->setScanline(yy, zz);
		RunLength inLength = gridIn->getNextLength();
		int inType = gridIn->getRunType(&inLength);
		int nextInOffset = inLength;
		OccElement *inElement;

		if (inType == OccGridRLE::CONSTANT_DATA)
		    inElement = gridIn->getNextElement();

		// Prepare output scanline
		initScanline(gridIn, outScanline);
		xx = 0;

		for (int i = 0; i < numRuns; i++) {

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		    pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmin = MIN(xmin, int((newpos.x - 
					  gridIn->sliceOrigins[zz][0])/res));
		    xmin = MAX(xmin, 0);


		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y1*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res));

		    pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
				 y2*res + depthMap->origin[1],
				 gridIn->sliceOrigins[zz][2]);
		    applyInvLinePersp(pos, newpos);
		    xmax = MAX(xmax, 
			       int(ceil((newpos.x - 
					 gridIn->sliceOrigins[zz][0])/res)));
		    xmax = MIN(xmax, gridIn->xdim-1);


		    // Copy over all whole runs that precede the current xmin

		    while (nextInOffset < xmin) {
			if (nextInOffset != xx) {
			    appendScanlinePortion(gridIn, inType, inElement, 
						  nextInOffset - xx);
			}
			xx = nextInOffset;

			// Update for next run
			inLength = gridIn->getNextLength();
			inType = gridIn->getRunType(&inLength);
			nextInOffset += inLength; 
			if (inType == OccGridRLE::CONSTANT_DATA) {
			    inElement = gridIn->getNextElement();
			}
		    }
		

		    // Copy over the remaining portion of the first
		    // overlapping run

		    if (xx != xmin) {
			appendScanlinePortion(gridIn, inType, inElement, 
					      xmin - xx);
		    }

		    xx = xmin;
		    
		    // Update the output based on new data combined with
		    //  old data

		    while (xx <= xmax) {

			// If old input run is finished, start a new one

			if (xx == nextInOffset) {
			    inLength = gridIn->getNextLength();
			    inType = gridIn->getRunType(&inLength);
			    nextInOffset += inLength; 
			    if (inType == OccGridRLE::CONSTANT_DATA) {
				inElement = gridIn->getNextElement();
			    }
			}

			// If varying run, get next element

			if (inType == OccGridRLE::VARYING_DATA) {
			    inElement = gridIn->getNextElement();
			}

			// Copy over element and update

			outElement = *inElement;

			if (outElement.totalWeight == 0)
			    outElement.value = 0;

			// Write element to the output grid
			appendScanline(&outElement);

			xNewOff++;
			xx++;
		    }
		}


		// Finish up any remaining runs

		while (xx < gridOut->xdim) {
		    if (nextInOffset != xx) {
			appendScanlinePortion(gridIn, inType, inElement, 
					      nextInOffset - xx);
		    }
		    xx = nextInOffset;

		    // Update for next run
		    inLength = gridIn->getNextLength();
		    inType = gridIn->getRunType(&inLength);

		    if (inType == OccGridRLE::END_OF_RUN) {
			break;
		    }

		    nextInOffset += inLength; 
		    if (inType == OccGridRLE::CONSTANT_DATA) {
			inElement = gridIn->getNextElement();
		    }
		}

		// Seal off the old run
		sealScanline();

		gridOut->copyScanline(outScanline, yy, zz);
		scanline = gridOut->getScanline(yy, zz);
		gridIn->copyScanline(outScanline, yy, zz);
	    }

	    oldNumRuns += numRuns;

	    yOff = (y1+y2)/2;
	    depthRuns = depthMap->getDepthRunsEdges
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

	    for (int i = 0; i < numRuns; i++) {

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = int((newpos.x - gridIn->sliceOrigins[zz][0])/res);

		pos.setValue(depthRuns[2*i]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmin = MIN(xmin, int((newpos.x - 
				      gridIn->sliceOrigins[zz][0])/res));
		xmin = MAX(xmin, 0);


		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y1*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = int(ceil((newpos.x - 
				 gridIn->sliceOrigins[zz][0])/res));

		pos.setValue(depthRuns[2*i+1]*res + depthMap->origin[0],
			     y2*res + depthMap->origin[1],
			     gridIn->sliceOrigins[zz][2]);
		applyInvLinePersp(pos, newpos);
		xmax = MAX(xmax, 
			   int(ceil((newpos.x - 
				     gridIn->sliceOrigins[zz][0])/res)));
		xmax = MIN(xmax, gridIn->xdim-1);

		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {

		    pos.setValue(xx*res+gridIn->sliceOrigins[zz][0],
				 yy*res+gridIn->sliceOrigins[zz][1],
				 gridIn->sliceOrigins[zz][2]);
		    applyLinePersp(pos, newpos);
		    xOff = (newpos.x - depthMap->origin[0])/res;
		    yOff = (newpos.y - depthMap->origin[1])/res;


		    resampleForCarving(depthMap, xOff, yOff, 
				       &depth, &confidence);

		    // Setting confidence to zero only allows for carving
		    confidence = 0;
		    updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
					 sampleSpacing, confidence, buf);
		}
	    }

	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");

    
    delete inScanline;
    delete outScanline;
}




static void
initScanline(OccGridRLE *inGrid, OccScanlineRLE *scanline)
{
    theInGrid = inGrid;
    theOutScanline = scanline;
    theOutScanline->reset();
    currentLength = 0;
}


static void
appendScanlinePortion(OccGridRLE *inGrid, int inType, 
		      OccElement *inElement, RunLength length)
{
    int i;

    if (currentType == inType && currentLength > 0) {

	if (currentType == OccGridRLE::CONSTANT_DATA) {

	    if (inElement->value == lastElement.value &&
		inElement->totalWeight == lastElement.totalWeight) {

		// If same constant then just increment length.
		currentLength += length;
	    } else {
		
		// If different constants, then seal old run and start
		// a new one
		inGrid->setRunType(&currentLength, currentType);
		theOutScanline->putNextLength(currentLength);

		theOutScanline->putNextElement(inElement);
		lastElement = *inElement;
		currentLength = length;
	    }
	} else {

	    // If old and new are varying, then add on new data
	    for (i = 0; i < length-1; i++) {
		theOutScanline->putNextElement(inGrid->getNextElement());
	    }
	    lastElement = *(inGrid->getNextElement());
	    theOutScanline->putNextElement(&lastElement);
	    currentLength += length;
	}
    }
    else {
	// If different, then seal up the old run and start a
	// new one

	if (currentLength > 0) {
	    inGrid->setRunType(&currentLength, currentType);
	    theOutScanline->putNextLength(currentLength);
	}
	
	if (inType == OccGridRLE::CONSTANT_DATA) {
	    theOutScanline->putNextElement(inElement);
	    lastElement = *inElement;
	} else {
	    for (i = 0; i < length-1; i++) {
		theOutScanline->putNextElement(inGrid->getNextElement());
	    }
	    lastElement = *(inGrid->getNextElement());
	    theOutScanline->putNextElement(&lastElement);
	}
	currentLength = length;
	currentType = inType;
    }
}


static void
appendScanline(OccElement *inElement)
{
    int i, inType;

    inType = theInGrid->decideType(inElement);
    if (currentType == inType && currentLength > 0) {
	if (currentType == OccGridRLE::CONSTANT_DATA) {
	    if (inElement->value == lastElement.value &&
		inElement->totalWeight == lastElement.totalWeight) {

		// If same constant then just increment length.
		currentLength++;
	    } else {

		// If different constants, then seal old run and start
		// a new one
		theInGrid->setRunType(&currentLength, currentType);
		theOutScanline->putNextLength(currentLength);
		theOutScanline->putNextElement(inElement);
		lastElement = *inElement;
		currentLength = 1;
	    }
	} else {

	    // If old and new are varying, then add on new data
	    theOutScanline->putNextElement(inElement);
	    lastElement = *inElement;
	    currentLength++;
	}
    }
    else {
	// If different, then seal up the old run and start a
	// new one

	if (currentLength > 0) {
	    theInGrid->setRunType(&currentLength, currentType);
	    theOutScanline->putNextLength(currentLength);
	}

	lastElement = *inElement;
	theOutScanline->putNextElement(inElement);
	currentLength = 1;
	currentType = inType;
    }
}


static void
sealScanline()
{
    theInGrid->setRunType(&currentLength, currentType);
    theOutScanline->putNextLength(currentLength);
    theOutScanline->putNextLength(OccGridRLE::END_OF_RUN);	    
}


