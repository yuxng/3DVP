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

#ifdef linux
#include <endian.h>
#endif


#include "OccGridRLE.h"

// Lucas:
// Should we recompact (copy twice) the RLE pointers after doing
// a transpose?  It would have better memory coherence, but it also
// costs a lot...  In my tests, it was faster to avoid recompacting.
#define OCCGRIDRLE_RECOMPACT 0

OccScanlineRLE::OccScanlineRLE()
{
    this->lengths = NULL;
    this->elements = NULL;
}


OccScanlineRLE::OccScanlineRLE(int dim)
{
    this->lengths = new RunLength[dim];
    this->elements = new OccElement[dim];
}


OccScanlineRLE::~OccScanlineRLE()
{
    if (this->lengths != NULL) {
	delete [] this->lengths;
    }

    if (this->elements != NULL) {
	delete [] this->elements;
    }
}



OccGridRLE::OccGridRLE()
{
    this->xdim = this->ydim = this->zdim = 0;
    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    this->lengthChunker = NULL;
    this->elementChunker = NULL;

    this->rleScanline = new OccScanlineRLE;

    this->defaultElement = new OccElement;
    //this->defaultElement->value = 0;
    this->defaultElement->value = USHRT_MAX;
    this->defaultElement->totalWeight = 0;

    this->emptyNoWeight.value = 0;
    this->emptyNoWeight.totalWeight = 0;

    this->fullNoWeight.value = USHRT_MAX;
    this->fullNoWeight.totalWeight = 0;
}


OccGridRLE::OccGridRLE(int xd, int yd, int zd, int num)
{
    this->xdim = this->ydim = this->zdim = 0;
    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    this->rleScanline = new OccScanlineRLE;

    this->defaultElement = new OccElement;
    this->defaultElement->value = 0;
    this->defaultElement->totalWeight = 0;

    this->emptyNoWeight.value = 0;
    this->emptyNoWeight.totalWeight = 0;

    this->fullNoWeight.value = USHRT_MAX;
    this->fullNoWeight.totalWeight = 0;

    this->lengthChunker = NULL;
    this->elementChunker = NULL;

    this->init(xd, yd, zd, num);
}


void
OccGridRLE::init(int xd, int yd, int zd, int num)
{
    int size1, size2, size3, sliceSize;

    this->xdim = xd;
    this->ydim = yd;
    this->zdim = zd;

    int maxdim = MAX(this->xdim, MAX(this->ydim, this->zdim));
    this->sliceOrigins = new vec3f[maxdim];
    this->scanline = new OccElement[maxdim];

    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    size1 = xd*yd;
    size2 = xd*zd;
    size3 = yd*zd;

    sliceSize = MAX(MAX(size1, size2), size3);
    this->slice = new OccElement[sliceSize];

    this->rleScanline = new OccScanlineRLE;

    this->defaultElement = new OccElement;
    this->defaultElement->value = USHRT_MAX;
    this->defaultElement->totalWeight = 0;

    this->emptyNoWeight.value = 0;
    this->emptyNoWeight.totalWeight = 0;

    this->fullNoWeight.value = USHRT_MAX;
    this->fullNoWeight.totalWeight = 0;

    this->lengthAddr = new RunLength*[sliceSize];
    this->elementAddr = new OccElement*[sliceSize];
    
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
    if (sizeof(OccElement) >= 2*sizeof(ushort)) {
	this->transpXZbytesPerScanline = 
	    this->xdim*sizeof(OccElement)+3*sizeof(ushort);
    }
    else {
	this->transpXZbytesPerScanline = 
	    this->xdim/2*
	    (sizeof(OccElement)+2*sizeof(ushort)) + sizeof(ushort);
    }

    this->transpXZslice = new uchar[this->zdim*this->transpXZbytesPerScanline];

    this->clear();
}


OccGridRLE::~OccGridRLE()
{
    this->freeSpace();
}


void
OccGridRLE::freeSpace()
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
OccGridRLE::copyParams(OccGridRLE *other)
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
OccGridRLE::allocNewRun(int y, int z)
{
    // Make room for contiguous runs and point the length and data
    // pointers to the next place that length and data will be put

    this->elementChunker->newChunkIfNeeded(sizeof(OccElement)*(this->xdim+1));
    this->elementAddr[y + z*this->ydim] = 
	(OccElement *)this->elementChunker->currentAddr;

    this->lengthChunker->newChunkIfNeeded(sizeof(RunLength)*(this->xdim+1));
    this->lengthAddr[y + z*this->ydim] = 
	(RunLength *)this->lengthChunker->currentAddr;

    setScanline(y, z);
}


void
OccGridRLE::setScanline(int y, int z)
{
    currentLength = this->lengthAddr[y + z*this->ydim];
    currentElem = this->elementAddr[y + z*this->ydim];
}


#if defined(linux) && BYTE_ORDER == LITTLE_ENDIAN

static inline void
swap_4(const void *p, size_t size)
{
    char t;
    for (char *pc = (char *) p; pc < (char *) p + size; pc += 4)
    {
        t = pc[0]; pc[0] = pc[3]; pc[3] = t;
        t = pc[1]; pc[1] = pc[2]; pc[2] = t;
    }
}

static inline void
swap_2(const void *p, size_t size)
{
    char t;
    for (char *pc = (char *) p; pc < (char *) p + size; pc += 2)
    {
        t = pc[0]; pc[0] = pc[1]; pc[1] = t;
    }
}

static inline void
write_be_int(const void *p, size_t size, size_t nitems, FILE *fp)
{
    swap_4(p, size * nitems);
    fwrite(p, size, nitems, fp);
    swap_4(p, size * nitems);
}

static inline void
write_be_float(const void *p, size_t size, size_t nitems, FILE *fp)
{
    swap_4(p, size * nitems);
    fwrite(p, size, nitems, fp);
    swap_4(p, size * nitems);
}

static inline void
write_be_ushort(const void *p, size_t size, size_t nitems, FILE *fp)
{
    swap_2(p, size * nitems);
    fwrite(p, size, nitems, fp);
    swap_2(p, size * nitems);
}

static inline void
read_be_int(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
    swap_4(p, size * nitems);
}

static inline void
read_be_float(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
    swap_4(p, size * nitems);
}

static inline void
read_be_ushort(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
    swap_2(p, size * nitems);
}

#else

static inline void
write_be_int(const void *p, size_t size, size_t nitems, FILE *fp)
{
    fwrite(p, size, nitems, fp);
}

static inline void
write_be_float(const void *p, size_t size, size_t nitems, FILE *fp)
{
    fwrite(p, size, nitems, fp);
}

static inline void
write_be_ushort(const void *p, size_t size, size_t nitems, FILE *fp)
{
    fwrite(p, size, nitems, fp);
}

static inline void
read_be_int(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
}

static inline void
read_be_float(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
}

static inline void
read_be_ushort(void *p, size_t size, size_t nitems, FILE *fp)
{
    fread(p, size, nitems, fp);
}

#endif

int
OccGridRLE::write(char *filename)
{
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL)
	return FALSE;

    write_be_int(&this->xdim, sizeof(int), 1, fp);
    write_be_int(&this->ydim, sizeof(int), 1, fp);
    write_be_int(&this->zdim, sizeof(int), 1, fp);
    write_be_int(&this->axis, sizeof(int), 1, fp);
    write_be_int(&this->flip, sizeof(int), 1, fp);
    write_be_float(&this->origin, sizeof(float), 3, fp);
    write_be_float(&this->resolution, sizeof(float), 1, fp);
    
    int yy, zz, numRuns, numElems;

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

    write_be_int(&lengthByteCount, sizeof(int), 1, fp);
    write_be_int(&elementByteCount, sizeof(int), 1, fp);

    int i;
    RunLength length;
    int runType;
    OccElement *element;

    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    setScanline(yy, zz);
	    while (TRUE) {
		length = getNextLength();
		write_be_ushort(&length, sizeof(RunLength), 1, fp);
		
		runType = getRunType(&length);

		if (runType == OccGridRLE::END_OF_RUN)
		    break;
		
		if (runType == OccGridRLE::CONSTANT_DATA) {
		    element = getNextElement();
		    write_be_ushort(element, sizeof(OccElement), 1, fp);
		}
		else {
		    for (i=0; i<length; i++) {
			element = getNextElement();
			write_be_ushort(element, sizeof(OccElement), 1, fp);
		    }
		}
	    }
	}
    }

    fclose(fp);

    return TRUE;
}


int
OccGridRLE::read(char *filename)
{
    int size1, size2, size3, xd, yd, zd, sliceSize;

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
	return FALSE;

    read_be_int(&xd, sizeof(int), 1, fp);
    read_be_int(&yd, sizeof(int), 1, fp);
    read_be_int(&zd, sizeof(int), 1, fp);

    if (xd != this->xdim || yd != this->ydim || zd != this->zdim) {
	size1 = xd*yd;
	size2 = xd*zd;
	size3 = yd*zd;
	
	sliceSize = MAX(MAX(size1, size2), size3);

	if (this->slice != NULL)
	    delete [] this->slice;
	this->slice = new OccElement[sliceSize];

	if (this->lengthAddr != NULL)
	    delete [] this->lengthAddr;
	this->lengthAddr = new RunLength*[sliceSize];

	if (this->elementAddr != NULL)
	    delete [] this->elementAddr;
	this->elementAddr = new OccElement*[sliceSize];

	int maxdim = MAX(xd, MAX(yd, zd));

	if (this->sliceOrigins != NULL)
	    delete [] this->sliceOrigins;
	this->sliceOrigins = new vec3f[maxdim];

	if (this->scanline != NULL)
	    delete [] this->scanline;
	this->scanline = new OccElement[maxdim];

	this->xdim = xd;
	this->ydim = yd;
	this->zdim = zd;

	if (sizeof(OccElement) > 2*sizeof(ushort)) {
	    this->transpXZbytesPerScanline = 
		this->xdim*sizeof(OccElement)+3*sizeof(ushort);
	}
	else {
	    this->transpXZbytesPerScanline = 
		this->xdim/2*
		(sizeof(OccElement)+2*sizeof(ushort)) + sizeof(ushort);
	}

	if (this->transpXZslice != NULL)
	    delete [] this->transpXZslice;
	this->transpXZslice = new uchar
	    [this->zdim*this->transpXZbytesPerScanline];
    }

    read_be_int(&this->axis, sizeof(int), 1, fp);
    read_be_int(&this->flip, sizeof(int), 1, fp);
    read_be_float(&this->origin, sizeof(float), 3, fp);
    read_be_float(&this->resolution, sizeof(float), 1, fp);

    int lengthByteCount, elementByteCount;
    read_be_int(&lengthByteCount, sizeof(int), 1, fp);
    read_be_int(&elementByteCount, sizeof(int), 1, fp);

    reset();

    int i, yy, zz;
    RunLength length;
    int runType;
    OccElement element;

    for (zz = 0; zz < this->zdim; zz++) {
	for (yy = 0; yy < this->ydim; yy++) {
	    allocNewRun(yy, zz);
	    while (TRUE) {
		read_be_ushort(&length, sizeof(RunLength), 1, fp);
		putNextLength(length);
		
		runType = getRunType(&length);

		if (runType == OccGridRLE::END_OF_RUN)
		    break;

		if (runType == OccGridRLE::CONSTANT_DATA) {
		    read_be_ushort(&element, sizeof(OccElement), 1, fp);
		    putNextElement(&element);
		}
		else {
		    for (i=0; i<length; i++) {
			read_be_ushort(&element, sizeof(OccElement), 1, fp);
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
OccGridRLE::writeDen(char *)
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

    Store_Indexed_DEN_File(filename, sizeof(OccElement));
*/
    
    // So much for error checking...
    return TRUE;
}


void
OccGridRLE::copy(OccGridRLE *other)
{
    OccScanlineRLE *rleScanline;

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
OccGridRLE::transposeXZ(OccGridRLE *outGrid)
{
    int zz, yy;
    OccScanlineRLE *rleScanlines;
    OccElement *elementData;
    RunLength *lengthData;
    int *runTypes;

    outGrid->reset();
    outGrid->copyParams(this);
        
    SWAP_INT(outGrid->xdim, outGrid->zdim);
    SWAP_FLOAT(outGrid->origin[0], outGrid->origin[2]);

    // Set up the new scanlines
    rleScanlines = new OccScanlineRLE[outGrid->zdim];


/*
    rleScanlines = new OccScanlineRLE[outGrid->zdim];
    for (zz = 0; zz < outGrid->zdim; zz++) {
	rleScanlines[zz].lengths = new RunLength[outGrid->xdim + 1];
	rleScanlines[zz].elements = new OccElement[outGrid->xdim + 1];
    }    
    */    

    lengthData = new RunLength[(outGrid->xdim+1)*outGrid->zdim];
    elementData = new OccElement[(outGrid->xdim)*outGrid->zdim];
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
	OccElement *element;


	// Lay down first scanline

	xx = 0;
	setScanline(yy, 0);
	while (TRUE) {
	    length = getNextLength();
	    
	    runType = getRunType(&length);
	    
	    if (runType == OccGridRLE::END_OF_RUN)
		break;
	    
	    if (runType == OccGridRLE::CONSTANT_DATA) {
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

	int end1, end2, type1=0, type2=0;
	RunLength length1, length2;
	OccElement *elem1=NULL, *elem2=NULL;
	OccScanlineRLE *lastScanline;

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
		} else if (type1 == OccGridRLE::VARYING_DATA) {
		    elem1 = lastScanline->getNextElement();
		}

		// Load up new lengths and/or new element from current scanline
		if (xx == end2) {
		    length2 = getNextLength();
		    type2 = getRunType(&length2);
		    end2 += length2;
		    elem2 = getNextElement();
		} else if (type2 == OccGridRLE::VARYING_DATA) {
		    elem2 = getNextElement();
		}

		if (type1 == OccGridRLE::CONSTANT_DATA &&
		    type2 == OccGridRLE::CONSTANT_DATA &&
		    elem1->value == elem2->value &&
		    elem2->totalWeight == elem2->totalWeight) {

		    // If both are the same constant value, then skip to end
		    // of one of them, whichever comes first
		    xx = MIN(end1, end2);

		} 
		else if (type1 == OccGridRLE::VARYING_DATA &&
			 type2 == OccGridRLE::VARYING_DATA) {

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
	    } else if (type1 == OccGridRLE::VARYING_DATA) {
		elem1 = lastScanline->getNextElement();
	    }
	    length = zz - runOffset[xx];
	    setRunType(&length, type1);
	    rleScanlines[xx].putNextLength(length);
	    xx++;
	}


	// Write out the end-of-run flags and copy runs to output grid
	for (zz = 0; zz < outGrid->zdim; zz++) {
	    rleScanlines[zz].putNextLength(OccGridRLE::END_OF_RUN);
	    outGrid->allocNewRun(yy, zz);
	    outGrid->copyScanline(&rleScanlines[zz], yy, zz);
	}
    }

#if OCCGRIDRLE_RECOMPACT
    // Re-compactify
    this->copy(outGrid);

    // Put into output
    outGrid->copy(this);
#endif //OCCGRIDRLE_RECOMPACT

    // So that scanline deletion will not try to free these
    for (zz = 0; zz < outGrid->zdim; zz++) {
	rleScanlines[zz].lengths = NULL;
	rleScanlines[zz].elements = NULL;
    }

    delete [] rleScanlines;
    delete [] lengthData;
    delete [] elementData;
    delete [] runTypes;

    return TRUE;
}


int
OccGridRLE::transposeXY(OccGridRLE *outGrid)
{
    int zz, yy;
    OccScanlineRLE *rleScanlines;
    OccElement *elementData;
    RunLength *lengthData;
    int *runTypes;

    outGrid->reset();
    outGrid->copyParams(this);
        
    SWAP_INT(outGrid->xdim, outGrid->ydim);
    SWAP_FLOAT(outGrid->origin[0], outGrid->origin[1]);

    rleScanlines = new OccScanlineRLE[outGrid->ydim];
    lengthData = new RunLength[(outGrid->xdim+1)*outGrid->ydim];
    elementData = new OccElement[(outGrid->xdim)*outGrid->ydim];
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
	OccElement *element;


	// Lay down first scanline

	xx = 0;
	setScanline(0, zz);
	while (TRUE) {
	    length = getNextLength();
	    
	    runType = getRunType(&length);
	    
	    if (runType == OccGridRLE::END_OF_RUN)
		break;
	    
	    if (runType == OccGridRLE::CONSTANT_DATA) {
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

	int end1, end2, type1=0, type2=0;
	RunLength length1, length2;
	OccElement *elem1=NULL, *elem2=NULL;
	OccScanlineRLE *lastScanline;

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
		} else if (type1 == OccGridRLE::VARYING_DATA) {
		    elem1 = lastScanline->getNextElement();
		}

		// Load up new lengths and/or new element from current scanline
		if (xx == end2) {
		    length2 = getNextLength();
		    type2 = getRunType(&length2);
		    end2 += length2;
		    elem2 = getNextElement();
		} else if (type2 == OccGridRLE::VARYING_DATA) {
		    elem2 = getNextElement();
		}

		if (type1 == OccGridRLE::CONSTANT_DATA &&
		    type2 == OccGridRLE::CONSTANT_DATA &&
		    elem1->value == elem2->value &&
		    elem2->totalWeight == elem2->totalWeight) {

		    // If both are the same constant value, then skip to end
		    // of one of them, whichever comes first
		    xx = MIN(end1, end2);

		} 
		else if (type1 == OccGridRLE::VARYING_DATA &&
			 type2 == OccGridRLE::VARYING_DATA) {

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
	    } else if (type1 == OccGridRLE::VARYING_DATA) {
		elem1 = lastScanline->getNextElement();
	    }
	    length = yy - runOffset[xx];
	    setRunType(&length, type1);
	    rleScanlines[xx].putNextLength(length);
	    xx++;
	}


	// Write out the end-of-run flags and copy runs to output grid
	for (yy = 0; yy < outGrid->ydim; yy++) {
	    rleScanlines[yy].putNextLength(OccGridRLE::END_OF_RUN);
	    outGrid->allocNewRun(yy, zz);
	    outGrid->copyScanline(&rleScanlines[yy], yy, zz);
	}
    }
    
#if OCCGRIDRLE_RECOMPACT
    // Re-compactify
    this->copy(outGrid);

    // Put into output
    outGrid->copy(this);
#endif //OCCGRIDRLE_RECOMPACT

    // So that scanline deletion will not try to free these
    for (yy = 0; yy < outGrid->ydim; yy++) {
	rleScanlines[yy].lengths = NULL;
	rleScanlines[yy].elements = NULL;
    }

    delete [] rleScanlines;
    delete [] lengthData;
    delete [] elementData;
    delete [] runTypes;

    return TRUE;
}


int
OccGridRLE::transposeYZ(OccGridRLE *outGrid)
{
    int yy,zz;
    OccScanlineRLE *rleScanline;

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


OccScanlineRLE *
OccGridRLE::getRLEScanline(int y, int z)
{
    setScanline(y, z);
    rleScanline->lengths = currentLength;
    rleScanline->elements = currentElem;
    rleScanline->reset();
    return rleScanline;
}


OccElement *
OccGridRLE::getScanline(int y, int z)
{
    int i;
    assert (y<=ydim);
    assert (z<=zdim);
    assert (y>=0);
    assert (z>=0);
    setScanline(y, z);
    //printf ("%d %d\n",y,z);
    
    OccElement *scanPtr = this->scanline;

    RunLength length;
    int runType;
    OccElement *element;

    int xx = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	if (runType == OccGridRLE::END_OF_RUN)
	    break;

	if (runType == OccGridRLE::CONSTANT_DATA) {
	    element = getNextElement();
	    for (i=0; i<length; i++, scanPtr++, xx++) {
		scanPtr->value = element->value;
		scanPtr->totalWeight = element->totalWeight;
	    }
	}
	else {
	    for (i=0; i<length; i++, scanPtr++, xx++) {
		element = getNextElement();
		scanPtr->value = element->value;
		scanPtr->totalWeight = element->totalWeight;
	    }
	}
    }

    assert(xx == this->xdim);

    return this->scanline;
}


void
OccGridRLE::copyScanline(OccScanlineRLE *rleScanline, int y, int z)
{
    int i;
    RunLength length;
    int runType;
    OccElement *element;

    rleScanline->reset();
    allocNewRun(y,z);

    while (TRUE) {
	length = rleScanline->getNextLength();
	putNextLength(length);

	runType = getRunType(&length);

	if (runType == OccGridRLE::END_OF_RUN)
	    break;

	if (runType == OccGridRLE::CONSTANT_DATA) {
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
OccGridRLE::putScanline(OccElement *line, int y, int z)
{
    OccElement *scan = line;
    ushort count;

    int xoff = 0;

    allocNewRun(y, z);

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
	    setRunType(&count, OccGridRLE::CONSTANT_DATA);
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
	    setRunType(&count, OccGridRLE::CONSTANT_DATA);
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
	    setRunType(&count, OccGridRLE::VARYING_DATA);
	    putNextLength(count);
	}

	if (xoff == this->xdim) {
	    setRunType(&count, OccGridRLE::END_OF_RUN);	    
	    putNextLength(count);
	    break;
	}
    }
}


OccElement *
OccGridRLE::getSlice(char *axis, int sliceNum, int *pxdim, int *pydim)
{
    OccElement *buf1, *buf2;
    int xx, yy, zz;

    buf1 = slice;
    if (EQSTR(axis, "x")) {
	if (sliceNum < this->xdim) {
	    for (yy = 0; yy < this->ydim; yy++) {
		for (zz = 0; zz <this->zdim; zz++, buf1++) {
		    buf2 = this->getElement(sliceNum, yy, zz);
		    buf1->value = buf2->value;
		    buf1->totalWeight = buf2->totalWeight;
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
		}
	    }
	}
	*pxdim = this->xdim;
	*pydim = this->ydim;
    }

    return this->slice;
}


void
OccGridRLE::clear()
{
    RunLength length, end;
    
    this->reset();

    setRunType(&end, OccGridRLE::END_OF_RUN);

    length = this->xdim;
    setRunType(&length, OccGridRLE::CONSTANT_DATA);
    
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
OccGridRLE::reset()
{
    this->lengthChunker->reset();
    this->elementChunker->reset();
}


void
OccGridRLE::runStats(int yy, int zz, int *numRuns, int *numElems)
{
    int runType, runCount, elemCount;
    RunLength length;

    setScanline(yy,zz);

    runCount = 0;
    elemCount = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	if (runType == OccGridRLE::END_OF_RUN)
	    break;

	elemCount += length;
	runCount++;
    }

    *numRuns = runCount;
    *numElems = elemCount;
}


OccElement *
OccGridRLE::getElement(int xx, int yy, int zz)
{
    OccElement *element;
    RunLength length;
    int runType;

    setScanline(yy, zz);
    
    int currentX = 0;
    while (TRUE) {
	length = getNextLength();
	runType = getRunType(&length);

	assert(runType != OccGridRLE::END_OF_RUN);

	if (runType == OccGridRLE::CONSTANT_DATA) {
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

OccElement OccGridRLE::voxRead(int x, int y, int z)
{
  assert(x>=0 && x<xdim && y>=0 && y<ydim && z>=0 && z<zdim);
  OccElement *line;
  line = getScanline(y,z);

  return line[x];
  
}

void OccGridRLE::voxWrite(int x, int y, int z, OccElement &el)
{
  assert(x>=0 && x<xdim && y>=0 && y<ydim && z>=0 && z<zdim);
  
  OccElement *line;
  line = getScanline(y,z);
  line[x]=el;
  putScanline(line,y,z);
}

OccGrid *OccGridRLE::voxExpand()
{
  puts ("Expanding grid");
  printf("OccEl size %d\n",sizeof(OccElement));
  printf("About to use MB%d\n " ,(int)(sizeof(OccElement)*(double)xdim*(double)ydim*(double)zdim/(double)(1024*1024)));
  OccGrid *og=new OccGrid(xdim,ydim,zdim);
  for (int y=0;y<ydim;y++)
    for (int z=0;z<zdim; z++)
      {
	OccElement *line=getScanline(y,z);
	for  (int x=0;x<xdim;x++)
	  {
	    *(og->address(x,y,z))=line[x];
	  }
      }
  puts ("Done expanding grid");
  return og;
}

void OccGridRLE::voxRLE(OccGrid *og)
{
  puts ("Starting grid RLE");
   for (int y=0;y<ydim;y++)
    for (int z=0;z<zdim; z++)
      {
	OccElement *line=getScanline(y,z);
	for  (int x=0;x<xdim;x++)
	  {
	    line[x]=*(og->address(x,y,z));
	  }
	putScanline(line,y,z);
      }
   puts ("Finished grid RLE");
}
