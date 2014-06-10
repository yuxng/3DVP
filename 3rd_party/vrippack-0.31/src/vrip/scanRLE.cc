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
#include "resample.h"
#include "occFunc.h"
#include "scanRLE.h"


static OccGridRLE *theOutGrid;
static int currentType;
static RunLength currentLength;
static OccElement lastElement;


static void initScanline(OccGridRLE *, int, int);
static void appendScanlinePortion(OccGridRLE *inGrid, int inType, 
				  OccElement *inElement, RunLength length);
static void appendScanline(OccElement *inElement);
static void sealScanline();



// What happens when resample and updateCell are essentially no-ops?

#if 0

#define resample(depthMap, xOff, yOff, pDepth, pConfidence) *pDepth = 100; *pConfidence = 0;

#define updateCell(zz, depth, sampleSpacing, confidence, buf) confidence = 1;

#endif



void
scanConvert(OccGridRLE *gridIn, OccGridRLE *gridOut, 
	    OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;

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
		resample(depthMap, xOff, yOff, &depth, &confidence);

		// Using the full-blown filler!
		updateCell(gridIn->sliceOrigins[zz][2], depth, 
			   sampleSpacing, confidence, buf);
	    }
	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}


void
scanConvertDragTails(OccGridRLE *gridIn, OccGridRLE *gridOut, 
		     OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;

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
		resample(depthMap, xOff, yOff, &depth, &confidence);

		// Using the full-blown filler!
		updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
				     sampleSpacing, confidence, buf);
	    }
	    gridOut->putScanline(scanline,yy,zz);
	}
    }
    if (!Quiet)
	printf("\n");
}



void
scanConvertTree(OccGridRLE *gridIn, OccGridRLE *gridOut,
		 OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff;
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
		    resample(depthMap, xNewOff, yOff, &depth, &confidence);
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
scanConvertTreeFast(OccGridRLE *gridIn, OccGridRLE *gridOut,
		    OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline, outElement;
    int xx,yy,zz;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;


    //printf("Using bias toward emptiness\n");

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
	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns == 0) {
		rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		continue;
	    }

	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    xx = 0;

	    // Prepare input scanline
	    gridIn->setScanline(yy, zz);
	    RunLength inLength = gridIn->getNextLength();
	    int inType = gridIn->getRunType(&inLength);
	    int nextInOffset = inLength;
	    OccElement *inElement;

	    if (inType == OccGridRLE::CONSTANT_DATA)
		inElement = gridIn->getNextElement();

	    // Prepare output scanline
	    initScanline(gridOut, yy, zz);

	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, gridIn->xdim-1);
		xNewOff = xOff + xmin;

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
		    //resample(depthMap, xNewOff, yOff, &depth, &confidence);
		    resampleBetter(depthMap, xNewOff, yOff, &depth, &confidence);

		    updateCell(gridIn->sliceOrigins[zz][2], 
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
	}
    }
    if (!Quiet)
	printf("\n");
}


void
scanConvertTreeBiasEdgesEmpty(OccGridRLE *gridIn, OccGridRLE *gridOut,
			      OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline, outElement;
    int xx,yy,zz;
    int *depthRuns, numRuns;
    float zmin, zmax, xNewOff;
    float edgeConfidence;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;


//    printf("Using bias toward emptiness\n");

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
	    depthRuns = depthMap->getDepthRuns(yOff, zmin, zmax, &numRuns);
	    if (numRuns == 0) {
		rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		continue;
	    }

	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    xx = 0;

	    // Prepare input scanline
	    gridIn->setScanline(yy, zz);
	    RunLength inLength = gridIn->getNextLength();
	    int inType = gridIn->getRunType(&inLength);
	    int nextInOffset = inLength;
	    OccElement *inElement;

	    if (inType == OccGridRLE::CONSTANT_DATA)
		inElement = gridIn->getNextElement();

	    // Prepare output scanline
	    initScanline(gridOut, yy, zz);

	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, gridIn->xdim-1);
		xNewOff = xOff + xmin;

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
		    resampleWithEdge(depthMap, xNewOff, yOff, 
				     &depth, &confidence, &edgeConfidence);

		    updateCellBiasEdgesEmpty(gridIn->sliceOrigins[zz][2], 
					     depth, 
					     sampleSpacing, confidence, 
					     edgeConfidence,
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
	}
    }
    if (!Quiet)
	printf("\n");
}



static void
initScanline(OccGridRLE *grid, int yy, int zz)
{
    theOutGrid = grid;
    theOutGrid->allocNewRun(yy, zz);
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
		theOutGrid->setRunType(&currentLength, currentType);
		theOutGrid->putNextLength(currentLength);

		theOutGrid->putNextElement(inElement);
		lastElement = *inElement;
		currentLength = length;
	    }
	} else {

	    // If old and new are varying, then add on new data
	    for (i = 0; i < length-1; i++) {
		theOutGrid->putNextElement(inGrid->getNextElement());
	    }
	    lastElement = *(inGrid->getNextElement());
	    theOutGrid->putNextElement(&lastElement);
	    currentLength += length;
	}
    }
    else {
	// If different, then seal up the old run and start a
	// new one

	if (currentLength > 0) {
	    theOutGrid->setRunType(&currentLength, currentType);
	    theOutGrid->putNextLength(currentLength);
	}
	
	if (inType == OccGridRLE::CONSTANT_DATA) {
	    theOutGrid->putNextElement(inElement);
	    lastElement = *inElement;
	} else {
	    for (i = 0; i < length-1; i++) {
		theOutGrid->putNextElement(inGrid->getNextElement());
	    }
	    lastElement = *(inGrid->getNextElement());
	    theOutGrid->putNextElement(&lastElement);
	}
	currentLength = length;
	currentType = inType;
    }
}


static void
appendScanline(OccElement *inElement)
{
    int i, inType;

    inType = theOutGrid->decideType(inElement);
    if (currentType == inType && currentLength > 0) {
	if (currentType == OccGridRLE::CONSTANT_DATA) {
	    if (inElement->value == lastElement.value &&
		inElement->totalWeight == lastElement.totalWeight) {

		// If same constant then just increment length.
		currentLength++;
	    } else {

		// If different constants, then seal old run and start
		// a new one
		theOutGrid->setRunType(&currentLength, currentType);
		theOutGrid->putNextLength(currentLength);
		theOutGrid->putNextElement(inElement);
		lastElement = *inElement;
		currentLength = 1;
	    }
	} else {

	    // If old and new are varying, then add on new data
	    theOutGrid->putNextElement(inElement);
	    lastElement = *inElement;
	    currentLength++;
	}
    }
    else {
	// If different, then seal up the old run and start a
	// new one

	if (currentLength > 0) {
	    theOutGrid->setRunType(&currentLength, currentType);
	    theOutGrid->putNextLength(currentLength);
	}

	lastElement = *inElement;
	theOutGrid->putNextElement(inElement);
	currentLength = 1;
	currentType = inType;
    }
}


static void
sealScanline()
{
    theOutGrid->setRunType(&currentLength, currentType);
    theOutGrid->putNextLength(currentLength);
    theOutGrid->putNextLength(OccGridRLE::END_OF_RUN);	    
}


void
scanConvertTreeDragTails(OccGridRLE *gridIn, OccGridRLE *gridOut,
			 OrthoShear *shear, DepthMap *depthMap)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff;
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
			resampleForCarving(depthMap, xNewOff, yOff, 
					   &depth, &confidence);
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
	    }

	    oldNumRuns += numRuns;

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

	    xOff = (gridIn->sliceOrigins[zz][0] - depthMap->origin[0])/res;
	    for (int i = 0; i < numRuns; i++) {
		xmin = int(depthRuns[2*i] - xOff);
		xmin = MAX(xmin, 0);
		xmax = int(ceil(depthRuns[2*i+1] - xOff));
		xmax = MIN(xmax, gridIn->xdim-1);
		xNewOff = xOff + xmin;
		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
		    resampleForCarving(depthMap, xNewOff, yOff, 
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



int
scanConvertTreeDragTailsOneLine(OccGridRLE *gridIn, OccGridRLE *gridOut,
				OrthoShear *shear, DepthMap *depthMap, 
				int line)
{
    float xOff, yOff, res, depth, confidence;
    OccElement *buf, *scanline;
    int xx,yy,zz;
    int nothingNew;
    int *depthRuns, numRuns, oldNumRuns;
    float zmin, zmax, xNewOff;
    int xmin, xmax;
    OccScanlineRLE *rleScanline;

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    res = gridIn->resolution;

    gridOut->copyParams(gridIn);

    gridOut->reset();

    for (zz = 0; zz < line; zz++) {
	for (yy = 0; yy < gridIn->ydim; yy++) {	
	    rleScanline = gridIn->getRLEScanline(yy, zz);
	    gridOut->copyScanline(rleScanline, yy, zz);
	}
    }

    for (zz = line; zz < line+1; zz++) {

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
			resampleForCarving(depthMap, xNewOff, yOff, 
					   &depth, &confidence);
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

	    int skip = 0;
	    nothingNew = 0;
	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		nothingNew = 1;
		skip = 1;
	    } else if (numRuns == 0) {
		gridOut->putScanline(scanline,yy,zz);
		skip = 1;
	    } else if (oldNumRuns == 0) {
		scanline = gridIn->getScanline(yy,zz);
		skip = 0;
	    }

	    if (!skip) {
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
	    }

	    oldNumRuns += numRuns;

	    depthRuns = depthMap->getDepthRunsEdges
		(yOff, zmin, &numRuns);

	    if (oldNumRuns == 0 && numRuns == 0) {
	        rleScanline = gridIn->getRLEScanline(yy, zz);
		gridOut->copyScanline(rleScanline, yy, zz);
		if (nothingNew)
		    nothingNew = 1;
		else
		    nothingNew = 0;
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
		xNewOff = xOff + xmin;
		buf = scanline + xmin;
		for (xx = xmin; xx <= xmax; xx++, xNewOff++, buf++) {
		    resampleForCarving(depthMap, xNewOff, yOff, 
				       &depth, &confidence);
		    updateCellForCarving(gridIn->sliceOrigins[zz][2], depth, 
					 sampleSpacing, confidence, buf);
		}
	    }

	    gridOut->putScanline(scanline,yy,zz);
	}
    }

    for (zz = line+1; zz < gridIn->zdim; zz++) {
	for (yy = 0; yy < gridIn->ydim; yy++) {	
	    rleScanline = gridIn->getRLEScanline(yy, zz);
	    gridOut->copyScanline(rleScanline, yy, zz);
	}
    }

    return !nothingNew;
}

