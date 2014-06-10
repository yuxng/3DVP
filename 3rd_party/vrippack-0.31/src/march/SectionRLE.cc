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

#include "SectionRLE.h"

SectionRLE::SectionRLE()
{
    this->xdim = this->ydim = 0;
    this->rleScanline = new SectionScanlineRLE;
    this->defaultElement = new SectionElement;
    this->defaultElement->density = 0;
    this->defaultElement->nx = 0;
    this->defaultElement->ny = 0;
    this->defaultElement->nz = 0;
    this->defaultElement->confidence = 0;
    this->defaultElement->valid = FALSE;
    this->defaultElement->realData = TRUE;
    this->defaultElement->set.ntris = 0;
}


SectionRLE::SectionRLE(int xd, int yd, int num)
{
    this->init(xd, yd, num);
}


void
SectionRLE::init(int xd, int yd, int num)
{
    this->xdim = xd;
    this->ydim = yd;

    this->rleScanline = new SectionScanlineRLE;

    this->lengthAddr = new RunLength*[this->ydim];

    this->elementAddr = new SectionElement*[this->ydim];

    // Lucas optimization init.  Set cachedlength to be huge,
    // so that it will not be the "right run" for the first
    // try, and then will get initialized correctly.
    this->cachedX = new RunLength[this->ydim];
    this->cachedLengthAddr = new RunLength*[this->ydim];

    this->cachedElemAddr = new SectionElement*[this->ydim];

    // Invalidate the cache
    for (int myy = 0; myy < this->ydim; myy++) {
      this->cachedX[myy] = USHRT_MAX;
    }

    this->lengthChunker = new ChunkAllocator(num);
    this->elementChunker = new ChunkAllocator(num);

    this->defaultElement = new SectionElement;
    this->defaultElement->density = 0;
    this->defaultElement->nx = 0;
    this->defaultElement->ny = 0;
    this->defaultElement->nz = 0;
    this->defaultElement->valid = FALSE;
    this->defaultElement->set.ntris = 0;

    this->clear();
}


SectionRLE::~SectionRLE()
{
    this->freeSpace();
}


void
SectionRLE::freeSpace()
{
    if (this->lengthAddr != NULL) {
	delete [] this->lengthAddr;
    }
    if (this->elementAddr != NULL) {
	delete [] this->elementAddr;
    }
    if (this->defaultElement != NULL) {
	delete this->defaultElement;
    }
}

void
SectionRLE::clear()
{
    RunLength length, end;
    
    this->reset();

    setRunType(&end, SectionRLE::END_OF_RUN);

    length = this->xdim;
    setRunType(&length, SectionRLE::CONSTANT_DATA);
    
    for (int yy = 0; yy < this->ydim; yy++) {
	allocNewRun(yy);
	putNextLength(length);
	putNextElement(defaultElement);
	putNextLength(end);
    }
}




void
SectionRLE::setRunType(RunLength *length, int runType)
{
    if (runType == SectionRLE::END_OF_RUN)
	*length = SectionRLE::END_OF_RUN;
    else if (runType == SectionRLE::VARYING_DATA)
	*length = *length | SectionRLE::HIGHEST_BIT;

    // else leave it alone
}

void
SectionRLE::reset()
{
    this->lengthChunker->reset();
    this->elementChunker->reset();
}


void
SectionRLE::putNextElement(SectionElement *element)
{
    SectionElement *newElem = 
	(SectionElement *)this->elementChunker->alloc(sizeof(SectionElement));
    newElem->density = element->density;
    newElem->nx = element->nx;
    newElem->ny = element->ny;
    newElem->nz = element->nz;
    newElem->confidence = element->confidence;
    newElem->valid = element->valid;
    newElem->realData = element->realData;
    newElem->set.ntris = element->set.ntris;
}


void
SectionRLE::putNextLength(RunLength length)
{
    RunLength *newLength = 
	(RunLength *)this->lengthChunker->alloc(sizeof(RunLength));
    *newLength = length;
}




void
SectionRLE::allocNewRun(int y)
{
    // Make room for contiguous runs and point the length and data
    // pointers to the next place that length and data will be put

    this->elementChunker->newChunkIfNeeded(sizeof(SectionElement)*(this->xdim+1));
    this->elementAddr[y] = 
	(SectionElement *)this->elementChunker->currentAddr;

    this->lengthChunker->newChunkIfNeeded(sizeof(RunLength)*(this->xdim+1));
    this->lengthAddr[y] = 
	(RunLength *)this->lengthChunker->currentAddr;

    setScanline(y);

    // Lucas:  Invalidate the cached value
    this->cachedX[y] = USHRT_MAX;
}


void
SectionRLE::setScanline(int y)
{
    currentLength = this->lengthAddr[y];
    currentElem = this->elementAddr[y];
}

// Lucas:
// getElement is now an inline function,
// which calls getElementSlow if necessary
// (e.g. it doesn't have the right run in cache...)


SectionElement *
SectionRLE::getElementSlow(int xx, int yy)
{
    SectionElement *element=NULL;
    RunLength *oldCurrentLength;
    SectionElement *oldCurrentElem;

    RunLength length;
    int runType;

    // Save the old position pointers
    oldCurrentLength = currentLength;
    oldCurrentElem = currentElem;

    setScanline(yy);
    
    RunLength currentX = 0;

    // If cached values are less than the desired X,
    // use it as a starting point...
    if (cachedX[yy] <= xx) {
      currentX = cachedX[yy];
      currentLength = cachedLengthAddr[yy];
      currentElem = cachedElemAddr[yy];
    }

    for (  ; currentX <= xx; currentX += length) {

      // Lucas:  Save the starting point for this run.
      // when loop exits, cache saves last run used.
      cachedX[yy] = currentX;
      cachedLengthAddr[yy] = currentLength;
      cachedElemAddr[yy] = currentElem;

	length = getNextLength();
	runType = getRunType(&length);


	// Lucas:  Be dangerous.  You only live once.
	// skip the assertion. It's inside a critical loop.
	// assert(runType != SectionRLE::END_OF_RUN);

	if (runType == SectionRLE::CONSTANT_DATA) {
	    element = getNextElement();
	} else {
	    if (xx < currentX+length) {
		currentElem += (xx - currentX);
		element = getNextElement();
	    } else {
		currentElem += length;
	    }
	}
    }

    // Restore the old position pointers
    currentLength = oldCurrentLength;
    currentElem = oldCurrentElem;

    return element;
}


void
SectionRLE::copyScanline(SectionScanlineRLE *rleScanline, int y)
{
    int i;
    RunLength length;
    int runType;
    SectionElement *element;

    rleScanline->reset();
    allocNewRun(y);

    while (TRUE) {
	length = rleScanline->getNextLength();
	putNextLength(length);

	runType = getRunType(&length);

	if (runType == SectionRLE::END_OF_RUN)
	    break;

	if (runType == SectionRLE::CONSTANT_DATA) {
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
SectionRLE::copy(SectionRLE *other)
{
    SectionScanlineRLE *rleScanline;

    this->reset();

    for (int yy = 0; yy < this->ydim; yy++) {
	rleScanline = other->getRLEScanline(yy);
	this->copyScanline(rleScanline, yy);
    }
}


SectionScanlineRLE *
SectionRLE::getRLEScanline(int y)
{
    setScanline(y);
    rleScanline->lengths = currentLength;
    rleScanline->elements = currentElem;
    rleScanline->reset();
    return rleScanline;
}





