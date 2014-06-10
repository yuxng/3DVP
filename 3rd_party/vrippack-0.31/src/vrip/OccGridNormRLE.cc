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


#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "OccGridNormRLE.h"

OccGridNormRLE::OccGridNormRLE()
{
    this->xdim = this->ydim = this->zdim = 0;
    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    this->lengthChunker = NULL;
    this->elementChunker = NULL;

    this->rleScanline = new OccScanlineNormRLE;

    this->defaultElement = new OccNormElement;
    this->defaultElement->value = 0;
    this->defaultElement->totalWeight = 0;
    this->defaultElement->nx = 0;
    this->defaultElement->ny = 0;
    this->defaultElement->nz = 0;
    //this->defaultElement->more = FALSE;
}


OccGridNormRLE::OccGridNormRLE(int xd, int yd, int zd, int num)
{
    this->xdim = this->ydim = this->zdim = 0;
    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    this->rleScanline = new OccScanlineNormRLE;

    this->defaultElement = new OccNormElement;
    this->defaultElement->value = 0;
    this->defaultElement->totalWeight = 0;
    this->defaultElement->nx = 0;
    this->defaultElement->ny = 0;
    this->defaultElement->nz = 0;
    //this->defaultElement->more = FALSE;

    this->lengthChunker = NULL;
    this->elementChunker = NULL;

    this->init(xd, yd, zd, num);
}


void
OccGridNormRLE::init(int xd, int yd, int zd, int num)
{
    int size1, size2, size3, sliceSize;

    this->xdim = xd;
    this->ydim = yd;
    this->zdim = zd;

    int maxdim = MAX(this->xdim, MAX(this->ydim, this->zdim));
    this->sliceOrigins = new vec3f[maxdim];
    this->scanline = new OccNormElement[maxdim];

    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    size1 = xd*yd;
    size2 = xd*zd;
    size3 = yd*zd;

    sliceSize = MAX(MAX(size1, size2), size3);
    this->slice = new OccNormElement[sliceSize];

    this->rleScanline = new OccScanlineNormRLE;

    this->defaultElement = new OccNormElement;
    this->defaultElement->value = 0;
    this->defaultElement->totalWeight = 0;
    this->defaultElement->nx = 0;
    this->defaultElement->ny = 0;
    this->defaultElement->nz = 0;
    //this->defaultElement->more = FALSE;

    this->lengthAddr = new RunLength*[sliceSize];
    this->elementAddr = new OccNormElement*[sliceSize];
    
    if (this->lengthChunker == NULL)
	this->lengthChunker = new ChunkAllocator(num);
    else if (this->lengthChunker->chunkSize != num) {
	delete this->lengthChunker;
	this->lengthChunker = new ChunkAllocator(num);
    } else {
	this->lengthChunker->reset();
    }
    
    if (this->elementChunker == NULL)
	this->elementChunker = new ChunkAllocator(num);
    else if (this->elementChunker->chunkSize != num) {
	delete this->elementChunker;
	this->elementChunker = new ChunkAllocator(num);
    } else {
	this->elementChunker->reset();
    }
    

    // Compute the maximum bytes needed for a scanline
    //  Fix this!!
    if (sizeof(OccNormElement) >= 2*sizeof(ushort)) {
	this->transpXZbytesPerScanline = 
	    this->xdim*sizeof(OccNormElement)+3*sizeof(ushort);
    }
    else {
	this->transpXZbytesPerScanline = 
	    this->xdim/2*
	    (sizeof(OccNormElement)+2*sizeof(ushort)) + sizeof(ushort);
    }

    this->transpXZslice = new uchar[this->zdim*this->transpXZbytesPerScanline];

    this->clear();
}


OccGridNormRLE::~OccGridNormRLE()
{
    this->freeSpace();
}


void
OccGridNormRLE::freeSpace()
{
    if (this->sliceOrigins != NULL) {
	delete [] this->sliceOrigins;
    }
    if (this->lengthAddr != NULL) {
	delete [] this->lengthAddr;
    }
    if (this->elementAddr != NULL) {
	delete [] this->elementAddr;
    }
    if (this->slice != NULL) {
	delete [] this->slice;
    }
    if (this->scanline != NULL) {
	delete [] this->scanline;
    }
    if (this->rleScanline != NULL) {
	delete this->rleScanline;
    }
    if (this->defaultElement != NULL) {
	delete this->defaultElement;
    }
    if (this->elementChunker != NULL) {
	delete this->elementChunker;
    }
    if (this->lengthChunker != NULL) {
	delete this->lengthChunker;
    }
    if (this->transpXZslice != NULL) {
	delete [] this->transpXZslice;
    }
}


void
OccGridNormRLE::copyParams(OccGridNormRLE *other)
{
    this->xdim = other->xdim;
    this->ydim = other->ydim;
    this->zdim = other->zdim;
    this->axis = other->axis;
    this->flip = other->flip;
    this->origin[0] = other->origin[0];
    this->origin[1] = other->origin[1];
    this->origin[2] = other->origin[2];
}


void
OccGridNormRLE::allocNewRun(int y, int z)
{
    // Make room for contiguous runs and point the length and data
    // pointers to the next place that length and data will be put

    this->elementChunker->newChunkIfNeeded(sizeof(OccNormElement)*(this->xdim+1));
    this->elementAddr[y + z*this->ydim] = 
	(OccNormElement *)this->elementChunker->currentAddr;

    this->lengthChunker->newChunkIfNeeded(sizeof(RunLength)*(this->xdim+1));
    this->lengthAddr[y + z*this->ydim] = 
	(RunLength *)this->lengthChunker->currentAddr;

    setScanline(y, z);
}


void
OccGridNormRLE::setScanline(int y, int z)
{
    currentLength = this->lengthAddr[y + z*this->ydim];
    currentElem = this->elementAddr[y + z*this->ydim];
}


int
OccGridNormRLE::write(const char *filename)
{
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL)
	return FALSE;

    fwrite(&this->xdim, sizeof(int), 1, fp);
    fwrite(&this->ydim, sizeof(int), 1, fp);
    fwrite(&this->zdim, sizeof(int), 1, fp);
    fwrite(&this->axis, sizeof(int), 1, fp);
    fwrite(&this->flip, sizeof(int), 1, fp);
    fwrite(&this->origin, sizeof(float), 3, fp);
    fwrite(&this->resolution, sizeof(float), 1, fp);
    
    int yy, zz, numRuns, numElems;



    // I will write out the file as though it hos no normal
    // information for now.


    // First count bytes
    int lengthByteCount = 0;
    int elementByteCount = 0;
    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    runStats(yy, zz, &numRuns, &numElems);
	    // Don't forget the END_OF_RUN 
	    lengthByteCount = lengthByteCount + 
		(numRuns + 1)*sizeof(RunLength);
	    elementByteCount = elementByteCount + 
		(numElems)*sizeof(OccElement);
	}
    }

    fwrite(&lengthByteCount, sizeof(int), 1, fp);
    fwrite(&elementByteCount, sizeof(int), 1, fp);

    int i;
    RunLength length;
    int runType;
    OccNormElement *element;

    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    setScanline(yy, zz);
	    while (TRUE) {
		length = getNextLength();
		fwrite(&length, sizeof(RunLength), 1, fp);
		
		runType = getRunType(&length);

		if (runType == OccGridNormRLE::END_OF_RUN)
		    break;
		
		if (runType == OccGridNormRLE::CONSTANT_DATA) {
		    element = getNextElement();
		    fwrite(element, sizeof(OccElement), 1, fp);
		}
		else {
		    for (i=0; i<length; i++) {
			element = getNextElement();
			fwrite(element, sizeof(OccElement), 1, fp);
		    }
		}
	    }
	}
    }

    fclose(fp);

    return TRUE;
}


int
OccGridNormRLE::read(const char *filename)
{
    int size1, size2, size3, xd, yd, zd, sliceSize;

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
	return FALSE;

    fread(&xd, sizeof(int), 1, fp);
    fread(&yd, sizeof(int), 1, fp);
    fread(&zd, sizeof(int), 1, fp);

    if (xd != this->xdim || yd != this->ydim || zd != this->zdim) {
	size1 = xd*yd;
	size2 = xd*zd;
	size3 = yd*zd;
	
	sliceSize = MAX(MAX(size1, size2), size3);

	if (this->slice != NULL)
	    delete [] this->slice;
	this->slice = new OccNormElement[sliceSize];

	if (this->lengthAddr != NULL)
	    delete [] this->lengthAddr;
	this->lengthAddr = new RunLength*[sliceSize];

	if (this->elementAddr != NULL)
	    delete [] this->elementAddr;
	this->elementAddr = new OccNormElement*[sliceSize];

	int maxdim = MAX(xd, MAX(yd, zd));

	if (this->sliceOrigins != NULL)
	    delete [] this->sliceOrigins;
	this->sliceOrigins = new vec3f[maxdim];

	if (this->scanline != NULL)
	    delete [] this->scanline;
	this->scanline = new OccNormElement[maxdim];

	this->xdim = xd;
	this->ydim = yd;
	this->zdim = zd;

	if (sizeof(OccNormElement) > 2*sizeof(ushort)) {
	    this->transpXZbytesPerScanline = 
		this->xdim*sizeof(OccNormElement)+3*sizeof(ushort);
	}
	else {
	    this->transpXZbytesPerScanline = 
		this->xdim/2*
		(sizeof(OccNormElement)+2*sizeof(ushort)) + sizeof(ushort);
	}

	if (this->transpXZslice != NULL)
	    delete [] this->transpXZslice;
	this->transpXZslice = new uchar
	    [this->zdim*this->transpXZbytesPerScanline];
    }

    fread(&this->axis, sizeof(int), 1, fp);
    fread(&this->flip, sizeof(int), 1, fp);
    fread(&this->origin, sizeof(float), 3, fp);
    fread(&this->resolution, sizeof(float), 1, fp);

    int lengthByteCount, elementByteCount;
    fread(&lengthByteCount, sizeof(int), 1, fp);
    fread(&elementByteCount, sizeof(int), 1, fp);

    reset();

    int i, yy, zz;
    RunLength length;
    int runType;
    OccNormElement element;

    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    allocNewRun(yy, zz);
	    while (TRUE) {
		fread(&length, sizeof(RunLength), 1, fp);
		putNextLength(length);
		
		runType = getRunType(&length);

		if (runType == OccGridNormRLE::END_OF_RUN)
		    break;

		if (runType == OccGridNormRLE::CONSTANT_DATA) {
		    fread(&element, sizeof(OccNormElement), 1, fp);
		    putNextElement(&element);
		}
		else {
		    for (i=0; i<length; i++) {
			fread(&element, sizeof(OccNormElement), 1, fp);
			putNextElement(&element);
		    }
		}
	    }
	}
    }

    fclose(fp);

    return TRUE;
}


int
OccGridNormRLE::writeDen(const char *)
{

/*
    orig_min[0] = extr_min[0] = map_min[0] = 0;
    orig_min[1] = extr_min[1] = map_min[1] = 0;
    orig_min[2] = extr_min[2] = map_min[2] = 0;

    orig_max[0] = extr_max[0] = map_max[0] = this->xdim - 1;
    orig_max[1] = extr_max[1] = map_max[1] = this->ydim - 1;
    orig_max[2] = extr_max[2] = map_max[2] = this->zdim - 1;

    orig_len[0] = extr_len[0] = map_len[0] = this->xdim;
    orig_len[1] = extr_len[1] = map_len[1] = this->ydim;
    orig_len[2] = extr_len[2] = map_len[2] = this->zdim;

    map_warps = 0;

    map_length = (long)map_len[X] * (long)map_len[Y] * (long)map_len[Z];

    map_address = (uchar *)(this->elems);

    Store_Indexed_DEN_File(filename, sizeof(OccNormElement));
*/
    
    // So much for error checking...
    return TRUE;
}


void
OccGridNormRLE::copy(OccGridNormRLE *other)
{
    OccScanlineNormRLE *rleScanline;

    this->reset();
    this->copyParams(other);

    for (int zz = 0; zz < this->zdim; zz++) {
	for (int yy = 0; yy < this->ydim; yy++) {
	    rleScanline = other->getRLEScanline(yy, zz);
	    this->copyScanline(rleScanline, yy, zz);
	}
    }

}


int
OccGridNormRLE::transposeXZ(OccGridNormRLE *outGrid)
{
    int zz, yy;
    OccScanlineNormRLE *rleScanlines;
    OccNormElement *elementData;
    RunLength *lengthData;
    int *runTypes;

    outGrid->reset();
    outGrid->copyParams(this);
        
    SWAP_INT(outGrid->xdim, outGrid->zdim);
    SWAP_FLOAT(outGrid->origin[0], outGrid->origin[2]);

    // Set up the new scanlines
    rleScanlines = new OccScanlineNormRLE[outGrid->zdim];
    lengthData = new RunLength[(outGrid->xdim+1)*outGrid->zdim];
    elementData = new OccNormElement[(outGrid->xdim)*outGrid->zdim];
    for (zz = 0; zz < outGrid->zdim; zz++) {
	rleScanlines[zz].lengths = lengthData + zz*(outGrid->xdim + 1);
	rleScanlines[zz].elements = elementData + zz*(outGrid->xdim);
    }

    // For bookeeping
    runTypes = new int[outGrid->zdim];

    // Rename the bookeeping
    int *runOffset = runTypes;

    // Loop over slices
    for (yy = 0; yy < this->ydim; yy++) {

	// Initialize scanlines and bookkeeping
	for (zz = 0; zz < outGrid->zdim; zz++) {
	    rleScanlines[zz].reset();
	    runOffset[zz] = 0;
	}
	
	int xx;
	RunLength length;
	int runType, i;
	OccNormElement *element;


	// Lay down first scanline

	xx = 0;
	setScanline(yy, 0);
	while (TRUE) {
	    length = getNextLength();
	    
	    runType = getRunType(&length);
	    
	    if (runType == OccGridNormRLE::END_OF_RUN)
		break;
	    
	    if (runType == OccGridNormRLE::CONSTANT_DATA) {
		element = getNextElement();
		for (i=0; i < length; i++, xx++) {
		    rleScanlines[xx].putNextElement(element);
		}
	    }
	    else {
		for (i=0; i<length; i++, xx++) {
		    element = getNextElement();
		    rleScanlines[xx].putNextElement(element);
		}
	    }		
	}
	assert(xx == this->xdim);


	// Process the current and previous scanlines concurrently

	int end1, end2, type1, type2;
	RunLength length1, length2;
	OccNormElement *elem1, *elem2;
	OccScanlineNormRLE *lastScanline;

	// Loop over scanlines

	for (zz = 1; zz < this->zdim; zz++) {

	    // Load up previous scanline
	    lastScanline = getRLEScanline(yy, zz-1);
	    lastScanline->reset();

	    // Load up current scanline
	    setScanline(yy, zz);

	    // Initialize
	    xx = 0;
	    end1 = 0;
	    end2 = 0;

	    while (xx < this->xdim) {
		// Load up new lengths and/or new element from prior scanline
		if (xx == end1) {
		    length1 = lastScanline->getNextLength();
		    type1 = getRunType(&length1);
		    end1 += length1;
		    elem1 = lastScanline->getNextElement();
		} else if (type1 == OccGridNormRLE::VARYING_DATA) {
		    elem1 = lastScanline->getNextElement();
		}

		// Load up new lengths and/or new element from current scanline
		if (xx == end2) {
		    length2 = getNextLength();
		    type2 = getRunType(&length2);
		    end2 += length2;
		    elem2 = getNextElement();
		} else if (type2 == OccGridNormRLE::VARYING_DATA) {
		    elem2 = getNextElement();
		}

		if (type1 == OccGridNormRLE::CONSTANT_DATA &&
		    type2 == OccGridNormRLE::CONSTANT_DATA &&
		    elem1->value == elem2->value &&
		    elem2->totalWeight == elem2->totalWeight) {

		    // If both are the same constant value, then skip to end
		    // of one of them, whichever comes first
		    xx = MIN(end1, end2);

		} 
		else if (type1 == OccGridNormRLE::VARYING_DATA &&
			 type2 == OccGridNormRLE::VARYING_DATA) {

		    // If both are varying values, then write the new value
		    rleScanlines[xx].putNextElement(elem2);
		    xx++;

		} else {
		    // Else, clean up old run by updating length
		    length = zz - runOffset[xx];
		    setRunType(&length, type1);
		    rleScanlines[xx].putNextLength(length);

		    // Prepare for new run
		    runOffset[xx] = zz;
		    rleScanlines[xx].putNextElement(elem2);
		    xx++;
		}
	    }
	}


	// One last run through to finish up lengths
	zz = this->zdim;
	lastScanline = getRLEScanline(yy, zz-1);
	lastScanline->reset();
	xx = 0;
	end1 = 0;	
	while (xx < this->xdim) {
	    if (xx == end1) {
		length1 = lastScanline->getNextLength();
		type1 = getRunType(&length1);
		end1 += length1;
		elem1 = lastScanline->getNextElement();
	    } else if (type1 == OccGridNormRLE::VARYING_DATA) {
		elem1 = lastScanline->getNextElement();
	    }
	    length = zz - runOffset[xx];
	    setRunType(&length, type1);
	    rleScanlines[xx].putNextLength(length);
	    xx++;
	}


	// Write out the end-of-run flags and copy runs to output grid
	for (zz = 0; zz < outGrid->zdim; zz++) {
	    rleScanlines[zz].putNextLength(OccGridNormRLE::END_OF_RUN);
	    outGrid->allocNewRun(yy, zz);
	    outGrid->copyScanline(&rleScanlines[zz], yy, zz);
	}
    }

    // Re-compactify
    this->copy(outGrid);

    // Put into output
    outGrid->copy(this);

    delete [] rleScanlines;
    delete [] lengthData;
    delete [] elementData;
    delete [] runTypes;

    return TRUE;
}


int
OccGridNormRLE::transposeXY(OccGridNormRLE *outGrid)
{
    int zz, yy;
    OccScanlineNormRLE *rleScanlines;
    OccNormElement *elementData;
    RunLength *lengthData;
    int *runTypes;

    outGrid->reset();
    outGrid->copyParams(this);
        
    SWAP_INT(outGrid->xdim, outGrid->ydim);
    SWAP_FLOAT(outGrid->origin[0], outGrid->origin[1]);

    rleScanlines = new OccScanlineNormRLE[outGrid->ydim];
    lengthData = new RunLength[(outGrid->xdim+1)*outGrid->ydim];
    elementData = new OccNormElement[(outGrid->xdim)*outGrid->ydim];
    for (yy = 0; yy < outGrid->ydim; yy++) {
	rleScanlines[yy].lengths = lengthData + yy*(outGrid->xdim + 1);
	rleScanlines[yy].elements = elementData + yy*(outGrid->xdim);
    }
    runTypes = new int[outGrid->ydim];

    // Rename the bookeeping
    int *runOffset = runTypes;

    // Loop over slices
    for (zz = 0; zz < this->zdim; zz++) {

	// Initialize scanlines and bookkeeping
	for (yy = 0; yy < outGrid->ydim; yy++) {
	    rleScanlines[yy].reset();
	    runOffset[yy] = 0;
	}
	
	int xx;
	RunLength length;
	int runType, i;
	OccNormElement *element;


	// Lay down first scanline

	xx = 0;
	setScanline(0, zz);
	while (TRUE) {
	    length = getNextLength();
	    
	    runType = getRunType(&length);
	    
	    if (runType == OccGridNormRLE::END_OF_RUN)
		break;
	    
	    if (runType == OccGridNormRLE::CONSTANT_DATA) {
		element = getNextElement();
		for (i=0; i < length; i++, xx++) {
		    rleScanlines[xx].putNextElement(element);
		}
	    }
	    else {
		for (i=0; i<length; i++, xx++) {
		    element = getNextElement();
		    rleScanlines[xx].putNextElement(element);
		}
	    }		
	}
	assert(xx == this->xdim);


	// Process the current and previous scanlines concurrently

	int end1, end2, type1, type2;
	RunLength length1, length2;
	OccNormElement *elem1, *elem2;
	OccScanlineNormRLE *lastScanline;

	// Loop over scanlines

	for (yy = 1; yy < this->ydim; yy++) {

	    // Load up previous scanline
	    lastScanline = getRLEScanline(yy-1, zz);
	    lastScanline->reset();

	    // Load up current scanline
	    setScanline(yy, zz);

	    // Initialize
	    xx = 0;
	    end1 = 0;
	    end2 = 0;

	    while (xx < this->xdim) {
		// Load up new lengths and/or new element from prior scanline
		if (xx == end1) {
		    length1 = lastScanline->getNextLength();
		    type1 = getRunType(&length1);
		    end1 += length1;
		    elem1 = lastScanline->getNextElement();
		} else if (type1 == OccGridNormRLE::VARYING_DATA) {
		    elem1 = lastScanline->getNextElement();
		}

		// Load up new lengths and/or new element from current scanline
		if (xx == end2) {
		    length2 = getNextLength();
		    type2 = getRunType(&length2);
		    end2 += length2;
		    elem2 = getNextElement();
		} else if (type2 == OccGridNormRLE::VARYING_DATA) {
		    elem2 = getNextElement();
		}

		if (type1 == OccGridNormRLE::CONSTANT_DATA &&
		    type2 == OccGridNormRLE::CONSTANT_DATA &&
		    elem1->value == elem2->value &&
		    elem2->totalWeight == elem2->totalWeight) {

		    // If both are the same constant value, then skip to end
		    // of one of them, whichever comes first
		    xx = MIN(end1, end2);

		} 
		else if (type1 == OccGridNormRLE::VARYING_DATA &&
			 type2 == OccGridNormRLE::VARYING_DATA) {

		    // If both are varying values, then write the new value
		    rleScanlines[xx].putNextElement(elem2);
		    xx++;

		} else {
		    // Else, clean up old run by updating length
		    length = yy - runOffset[xx];
		    setRunType(&length, type1);
		    rleScanlines[xx].putNextLength(length);

		    // Prepare for new run
		    runOffset[xx] = yy;
		    rleScanlines[xx].putNextElement(elem2);
		    xx++;
		}
	    }
	}


	// One last run through to finish up lengths
	yy = this->ydim;
	lastScanline = getRLEScanline(yy-1, zz);
	lastScanline->reset();
	xx = 0;
	end1 = 0;	
	while (xx < this->xdim) {
	    if (xx == end1) {
		length1 = lastScanline->getNextLength();
		type1 = getRunType(&length1);
		end1 += length1;
		elem1 = lastScanline->getNextElement();
	    } else if (type1 == OccGridNormRLE::VARYING_DATA) {
		elem1 = lastScanline->getNextElement();
	    }
	    length = yy - runOffset[xx];
	    setRunType(&length, type1);
	    rleScanlines[xx].putNextLength(length);
	    xx++;
	}


	// Write out the end-of-run flags and copy runs to output grid
	for (yy = 0; yy < outGrid->ydim; yy++) {
	    rleScanlines[yy].putNextLength(OccGridNormRLE::END_OF_RUN);
	    outGrid->allocNewRun(yy, zz);
	    outGrid->copyScanline(&rleScanlines[yy], yy, zz);
	}
    }
    
    // Re-compactify
    this->copy(outGrid);

    // Put into output
    outGrid->copy(this);

    delete [] rleScanlines;
    delete [] lengthData;
    delete [] elementData;
    delete [] runTypes;

    return TRUE;
}


int
OccGridNormRLE::transposeYZ(OccGridNormRLE *outGrid)
{
    int yy,zz;
    OccScanlineNormRLE *rleScanline;

    outGrid->reset();
    outGrid->copyParams(this);	
    SWAP_INT(outGrid->ydim, outGrid->zdim);
    SWAP_FLOAT(outGrid->origin[1], outGrid->origin[2]);

    for (zz = 0; zz < outGrid->zdim; zz++) {
	for (yy = 0; yy < outGrid->ydim; yy++) {
	    rleScanline = this->getRLEScanline(zz, yy);
	    outGrid->copyScanline(rleScanline, yy, zz);
	}
    }

    if (outGrid->axis == Y_AXIS)
	outGrid->axis = Z_AXIS;
    else if (outGrid->axis == Z_AXIS)
	outGrid->axis = Y_AXIS;

    return TRUE;
}


OccScanlineNormRLE *
OccGridNormRLE::getRLEScanline(int y, int z)
{
    setScanline(y, z);
    rleScanline->lengths = currentLength;
    rleScanline->elements = currentElem;
    rleScanline->reset();
    return rleScanline;
}


OccNormElement *
OccGridNormRLE::getScanline(int y, int z)
{
    int i;

    setScanline(y, z);

    OccNormElement *scanPtr = this->scanline;

    RunLength length;
    int runType;
    OccNormElement *element;

    int xx = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	if (runType == OccGridNormRLE::END_OF_RUN)
	    break;

	if (runType == OccGridNormRLE::CONSTANT_DATA) {
	    element = getNextElement();
	    for (i=0; i<length; i++, scanPtr++, xx++) {
		*scanPtr = *element;
/*
		scanPtr->value = element->value;
		scanPtr->totalWeight = element->totalWeight;
		*/
	    }
	}
	else {
	    for (i=0; i<length; i++, scanPtr++, xx++) {
		element = getNextElement();
		*scanPtr = *element;

/*
		scanPtr->value = element->value;
		scanPtr->totalWeight = element->totalWeight;
		*/
	    }
	}
    }

    assert(xx == this->xdim);

    return this->scanline;
}


void
OccGridNormRLE::copyScanline(OccScanlineNormRLE *rleScanline, int y, int z)
{
    int i;
    RunLength length;
    int runType;
    OccNormElement *element;

    rleScanline->reset();
    allocNewRun(y,z);

    while (TRUE) {
	length = rleScanline->getNextLength();
	putNextLength(length);

	runType = getRunType(&length);

	if (runType == OccGridNormRLE::END_OF_RUN)
	    break;

	if (runType == OccGridNormRLE::CONSTANT_DATA) {
	    element = rleScanline->getNextElement();
	    putNextElement(element);
	}
	else {
	    for (i=0; i<length; i++) {
		element = rleScanline->getNextElement();
		putNextElement(element);
	    }
	}
    }
}


void
OccGridNormRLE::putScanline(OccNormElement *line, int y, int z)
{
    OccNormElement *scan = line;
    ushort count;

    int xoff = 0;

    allocNewRun(y, z);

    OccNormElement emptyNoWeight, fullNoWeight;

    emptyNoWeight.value = 0;
    emptyNoWeight.totalWeight = 0;
    emptyNoWeight.nx = 0;
    emptyNoWeight.ny = 0;
    emptyNoWeight.nz = 0;
    //emptyNoWeight.more = FALSE;

    fullNoWeight.value = USHRT_MAX;
    fullNoWeight.totalWeight = 0;
    fullNoWeight.nx = 0;
    fullNoWeight.ny = 0;
    fullNoWeight.nz = 0;
    //fullNoWeight.more = FALSE;

    while (TRUE) {
	if (scan->totalWeight == emptyNoWeight.totalWeight 
	    && scan->value == emptyNoWeight.value) {
	    count = 0;
	    while(scan->totalWeight == emptyNoWeight.totalWeight 
		  && scan->value == emptyNoWeight.value
		  && xoff < this->xdim) {
		scan++;
		count++;
		xoff++;
	    }



	    setRunType(&count, OccGridNormRLE::CONSTANT_DATA);
	    putNextLength(count);
	    putNextElement(&emptyNoWeight);
	}
	else if (scan->totalWeight == fullNoWeight.totalWeight 
	    && scan->value == fullNoWeight.value) {
	    count = 0;
	    while(scan->totalWeight == fullNoWeight.totalWeight 
		  && scan->value == fullNoWeight.value
		  && xoff < this->xdim) {
		scan++;
		count++;
		xoff++;
	    }
	    setRunType(&count, OccGridNormRLE::CONSTANT_DATA);
	    putNextLength(count);
	    putNextElement(&fullNoWeight);
	}
	else {
	    count = 0;
	    while (!(scan->totalWeight == emptyNoWeight.totalWeight 
		     && scan->value == emptyNoWeight.value) && 
		   !(scan->totalWeight == fullNoWeight.totalWeight 
		     && scan->value == fullNoWeight.value) &&
		   xoff < this->xdim) {

		putNextElement(scan);
		scan++;
		count++;
		xoff++;		
	    }
	    setRunType(&count, OccGridNormRLE::VARYING_DATA);
	    putNextLength(count);
	}

	if (xoff == this->xdim) {
	    count = 0;
	    setRunType(&count, OccGridNormRLE::END_OF_RUN);	    
	    putNextLength(count);
	    break;
	}
    }
}


OccNormElement *
OccGridNormRLE::getSlice(const char *axis, int sliceNum, int *pxdim, int *pydim)
{
    OccNormElement *buf1, *buf2;
    int xx, yy, zz;

    buf1 = slice;
    if (EQSTR(axis, "x")) {
	if (sliceNum < this->xdim) {
	    for (yy = 0; yy < this->ydim; yy++) {
		for (zz = 0; zz <this->zdim; zz++, buf1++) {
		    buf2 = this->getElement(sliceNum, yy, zz);
		    buf1->value = buf2->value;
		    buf1->totalWeight = buf2->totalWeight;
		    buf1->nx = buf2->nx;		    
		    buf1->ny = buf2->ny;
		    buf1->nz = buf2->nz;
		}
	    }
	}
	*pxdim = this->zdim;
	*pydim = this->ydim;
    }
    else if (EQSTR(axis, "y")) {
	if (sliceNum < this->ydim) {
	    for (zz = 0; zz < this->zdim; zz++) {
		buf2 = this->getScanline(sliceNum,zz);
		for (xx = 0; xx < this->xdim; xx++, buf1++, buf2++) {
		    buf1->value = buf2->value;
		    buf1->totalWeight = buf2->totalWeight;
		    buf1->nx = buf2->nx;
		    buf1->ny = buf2->ny;
		    buf1->nz = buf2->nz;
		}
	    }
	}
	*pxdim = this->xdim;
	*pydim = this->zdim;
    }
    else if (EQSTR(axis, "z")) {
	if (sliceNum < this->zdim) {
	    for (yy = 0; yy < this->ydim; yy++) {
		buf2 = this->getScanline(yy, sliceNum);
		for (xx = 0; xx < this->xdim; xx++, buf1++, buf2++) {
		    buf1->value = buf2->value;
		    buf1->totalWeight = buf2->totalWeight;
		    buf1->nx = buf2->nx;
		    buf1->ny = buf2->ny;
		    buf1->nz = buf2->nz;
		}
	    }
	}
	*pxdim = this->xdim;
	*pydim = this->ydim;
    }

/*
    printf("Checking all scanlines\n");
    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    buf2 = this->getScanline(yy, zz);
	    for (xx = 0; xx < this->xdim; xx++) {
		if (buf2->ny > 70) {
		    printf("Bug");
		}
		buf2++;
	    }
	}
    }
    */

    return this->slice;
}


void
OccGridNormRLE::clear()
{
    RunLength length, end;
    
    this->reset();

    setRunType(&end, OccGridNormRLE::END_OF_RUN);

    length = this->xdim;
    setRunType(&length, OccGridNormRLE::CONSTANT_DATA);
    
    for (int zz = 0; zz < this->zdim; zz++) {
	for (int yy = 0; yy < this->ydim; yy++) {
	    allocNewRun(yy, zz);
	    putNextLength(length);
	    putNextElement(defaultElement);
	    putNextLength(end);
	}
    }
}


void
OccGridNormRLE::reset()
{
    this->lengthChunker->reset();
    this->elementChunker->reset();
}


void
OccGridNormRLE::runStats(int yy, int zz, int *numRuns, int *numElems)
{
    int runType, runCount, elemCount;
    RunLength length;

    setScanline(yy,zz);

    runCount = 0;
    elemCount = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	if (runType == OccGridNormRLE::END_OF_RUN)
	    break;

	elemCount += length;
	runCount++;
    }

    *numRuns = runCount;
    *numElems = elemCount;
}


OccNormElement *
OccGridNormRLE::getElement(int xx, int yy, int zz)
{
    OccNormElement *element;
    RunLength length;
    int runType;

    setScanline(yy, zz);
    
    int currentX = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	assert(runType != OccGridNormRLE::END_OF_RUN);

	if (runType == OccGridNormRLE::CONSTANT_DATA) {
	    element = getNextElement();
	    currentX += length;
	    if (xx < currentX)
		break;
	}
	else {
	    // Really inefficient!!
/*
	    for (int i = 0; i < length; i++, currentX++) {
		element = getNextElement();
		if (xx == currentX)
		    break;
	    }
*/

	    if (xx < currentX+length) {
		currentElem += (xx - currentX);
		element = getNextElement();
		break;
	    } else {
		currentX += length;
		currentElem += length;
	    }
	}
    }
    return element;
}

