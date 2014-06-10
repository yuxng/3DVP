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


#ifndef _SECTION_RLE_
#define _SECTION_RLE_

#include "vrip.h"
#include <limits.h>
#include "mc_more.h"
#include "ChunkAllocator.h"

typedef ushort RunLength;

struct SectionElement {
    float density;
    float nx;
    float ny;
    float nz;
    float confidence;
    uchar valid;
    uchar realData;
    TriSet set;
};


class SectionScanlineRLE {

  public:

    RunLength *lengths;
    SectionElement *elements;
    RunLength *currentLength;
    SectionElement *currentElem;

    inline void reset();
    inline SectionElement *getNextElement();
    inline RunLength getNextLength();
};


class SectionRLE {

  public:
    enum {CONSTANT_DATA, VARYING_DATA, END_OF_RUN=USHRT_MAX};
    static const ushort HIGHEST_BIT = 0x8000;

    int xdim, ydim;
    uchar *current;

    RunLength **lengthAddr;
    ChunkAllocator *lengthChunker;
    RunLength *currentLength;

    // These three cache elements are optimizations for
    // getElement
    RunLength *cachedX;
    RunLength **cachedLengthAddr;
    SectionElement **cachedElemAddr;

    SectionElement **elementAddr;
    ChunkAllocator *elementChunker;
    SectionElement *currentElem;

    SectionElement *defaultElement;

    SectionScanlineRLE *rleScanline;

    SectionRLE();
    SectionRLE(int,int,int);

    void init(int,int,int);
    void freeSpace();

    void clear();
    void reset();

    uchar *allocBytes(int num);
    inline SectionElement *getElement(int xx, int yy);
    SectionElement *getElementSlow(int xx, int yy);
    inline int getRunType(RunLength *length);
    void setRunType(RunLength *length, int runType);
    void putNextElement(SectionElement *element);
    void putNextLength(RunLength length);
    inline SectionElement *getNextElement();
    inline RunLength getNextLength();
    void allocNewRun(int y);
    void setScanline(int y);
    SectionScanlineRLE *getRLEScanline(int yy);
    void copyScanline(SectionScanlineRLE *rleScanline, int y);
    void copy(SectionRLE *other);

    ~SectionRLE();
};


inline void
SectionScanlineRLE::reset()
{
    currentElem = elements;
    currentLength = lengths;
}

inline SectionElement *
SectionScanlineRLE::getNextElement()
{
    return currentElem++;
}

inline RunLength 
SectionScanlineRLE::getNextLength()
{
    return *currentLength++;
}


inline int
SectionRLE::getRunType(RunLength *length)
{
#if 0
    RunLength flag;

    if (*length == SectionRLE::END_OF_RUN)
	return SectionRLE::END_OF_RUN;
    
    flag = *length & SectionRLE::HIGHEST_BIT;
    *length = *length & ~SectionRLE::HIGHEST_BIT;
    if (flag) {
	return SectionRLE::VARYING_DATA;
    } else {
	return SectionRLE::CONSTANT_DATA;
    }
#else
    // Lucas:  Cutting down slightly on 'if' statements
    // to make it run faster
    RunLength flag = *length;
    if (flag == SectionRLE::END_OF_RUN) {
      return SectionRLE::END_OF_RUN;
    } else {
    *length = flag & ~SectionRLE::HIGHEST_BIT;
    // Move highest bit right to be 0 or 1
    // Danger! This assumes that CONSTANT_DATA is 0,
    // and VARYING_DATA is 1
    // since bitshift is faster than 'if' branch
      return(flag >> sizeof(RunLength)*8-1);
    }
#endif
}

inline SectionElement *
SectionRLE::getNextElement()
{
    return currentElem++;
}

inline RunLength 
SectionRLE::getNextLength()
{
    return *currentLength++;
}




inline SectionElement *
SectionRLE::getElement(int xx, int yy)
{
  // Warning!!! Unsafe (bypasses the proper access functions), 
  // but fast...
  // In the if statement below, cachedLenghthAddr might still be
  // null, but it should never check the second condition if
  // the first condition is false, since cachedX is initialized
  // to the biggest possible value.
  if (cachedX[yy] <= xx && 
      cachedX[yy]+((*cachedLengthAddr[yy]) &
		   (~SectionRLE::HIGHEST_BIT)) > xx) {
    // This is the right run. Just grab the right element
    if ((*cachedLengthAddr[yy])&(SectionRLE::HIGHEST_BIT)) {
      // varying data.. index into it
      return(cachedElemAddr[yy]+(xx-cachedX[yy]));
    } else {
      // constant data.. get first element
      return(cachedElemAddr[yy]);
    }
  } else {
    // If cache doesn't have right run, resort to old
    // method (which also will cache the new run.
    return(getElementSlow(xx, yy));
  }

  // This line isn't strictly necessary, but inserting it avoids
  // a compiler complaint. (Brian Curless, 6/5/06)
  return(getElementSlow(xx, yy));

}

#endif

