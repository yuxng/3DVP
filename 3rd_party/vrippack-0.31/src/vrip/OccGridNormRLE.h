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


#ifndef _OCC_GRID_NORM_RLE_
#define _OCC_GRID_NORM_RLE_

#include "vrip.h"
#include "ChunkAllocator.h"
#include <limits.h>


typedef ushort RunLength;


class OccScanlineNormRLE {

  public:

    RunLength *lengths;
    OccNormElement *elements;
    RunLength *currentLength;
    OccNormElement *currentElem;

    void reset();
    OccNormElement *getNextElement();
    RunLength getNextLength();
    void putNextElement(OccNormElement *element);
    void putNextLength(RunLength length);
};



class OccGridNormRLE {

  public:
    enum {CONSTANT_DATA, VARYING_DATA, END_OF_RUN=USHRT_MAX};
    static const ushort HIGHEST_BIT = 0x8000;

    int xdim, ydim, zdim;
    float resolution;
    vec3f origin;
    vec3f *sliceOrigins;
    int axis;
    int flip;

    RunLength **lengthAddr;
    ChunkAllocator *lengthChunker;
    RunLength *currentLength;

    OccNormElement **elementAddr;
    ChunkAllocator *elementChunker;
    OccNormElement *currentElem;

    OccScanlineNormRLE *rleScanline;
    OccNormElement *slice;
    OccNormElement *otherSlice;
    OccNormElement *scanline;
    OccNormElement *defaultElement;
    int transpXZbytesPerScanline;
    uchar *transpXZslice;

    OccGridNormRLE();
    OccGridNormRLE(int,int,int,int);

    void init(int,int,int,int);
    void freeSpace();

    void copy(OccGridNormRLE *other);
    void copyParams(OccGridNormRLE *other);

    int transposeXZ(OccGridNormRLE *other);
    int transposeXY(OccGridNormRLE *other);
    int transposeYZ(OccGridNormRLE *other);
    
    void clear();
    void reset();

    OccNormElement *getSlice(const char *axis, int sliceNum, int *pxdim, int *pydim);
    OccNormElement *getScanline(int y, int z);
    void putScanline(OccNormElement *line, int y, int z);
    void copyScanline(OccScanlineNormRLE *rleScanline, int y, int z);
    OccNormElement *transposeSlice(int xdim,int ydim);
    OccNormElement *getElement(int xx, int yy, int zz);
    void runStats(int yy, int zz, int *numRuns, int *numElems);
    int getRunType(RunLength *length);
    void setRunType(RunLength *length, int runType);
    void putNextElement(OccNormElement *element);
    void putNextLength(RunLength length);
    OccNormElement *getNextElement();
    RunLength getNextLength();
    void allocNewRun(int y, int z);
    void setScanline(int y, int z);
    int writeDen(const char *);
    int write(const char *);
    int read(const char *);
    OccScanlineNormRLE *getRLEScanline(int yy, int zz);
    ~OccGridNormRLE();
};


inline void
OccScanlineNormRLE::reset()
{
    currentElem = elements;
    currentLength = lengths;
}

inline OccNormElement *
OccScanlineNormRLE::getNextElement()
{
    return currentElem++;
}


inline RunLength 
OccScanlineNormRLE::getNextLength()
{
    return *currentLength++;
}


inline void 
OccScanlineNormRLE::putNextElement(OccNormElement *element)
{
    *currentElem = *element;

/*
    currentElem->value = element->value;
    currentElem->totalWeight = element->totalWeight;
    */

    currentElem++;
}


inline void 
OccScanlineNormRLE::putNextLength(RunLength length)
{
    *currentLength = length;
    currentLength++;
}


inline void
OccGridNormRLE::putNextElement(OccNormElement *element)
{
    OccNormElement *newElem = 
	(OccNormElement *)this->elementChunker->alloc(sizeof(OccNormElement));

    *newElem = *element;


/*
    newElem->value = element->value;
    newElem->totalWeight = element->totalWeight;
    */
}


inline void
OccGridNormRLE::putNextLength(RunLength length)
{
    RunLength *newLength = 
	(RunLength *)this->lengthChunker->alloc(sizeof(RunLength));
    *newLength = length;
}


inline OccNormElement *
OccGridNormRLE::getNextElement()
{
    return currentElem++;
}

inline RunLength 
OccGridNormRLE::getNextLength()
{
    return *currentLength++;
}


inline int
OccGridNormRLE::getRunType(RunLength *length)
{
    int flag;
    

    if (*length == OccGridNormRLE::END_OF_RUN)
	return OccGridNormRLE::END_OF_RUN;
    
    flag = *length & OccGridNormRLE::HIGHEST_BIT;
    *length = *length & ~OccGridNormRLE::HIGHEST_BIT;
    if (flag) {
	return OccGridNormRLE::VARYING_DATA;
    } else {
	return OccGridNormRLE::CONSTANT_DATA;
    }
}


inline void
OccGridNormRLE::setRunType(RunLength *length, int runType)
{
    if (runType == OccGridNormRLE::END_OF_RUN)
	*length = OccGridNormRLE::END_OF_RUN;
    else if (runType == OccGridNormRLE::VARYING_DATA)
	*length = *length | OccGridNormRLE::HIGHEST_BIT;

    // else leave it alone
}



#endif

