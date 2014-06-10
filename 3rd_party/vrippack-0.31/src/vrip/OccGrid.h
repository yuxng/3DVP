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


#ifndef _OCC_GRID_
#define _OCC_GRID_

#include "vrip.h"

class OccGrid {

  public:
    int xdim, ydim, zdim;
    float resolution;
    vec3f origin;
    vec3f *sliceOrigins;
    int axis;
    int flip;
    OccElement *elems;
    OccElement *slice;

    OccGrid();
    OccGrid(int,int,int);

    int transposeXZ();
    int transposeYZ();
    int copy(OccGrid *src);
    
    void clear();

    OccElement *address(int x, int y, int z)
    { return(this->elems + x + y*this->xdim+ z*this->xdim*this->ydim); };

    OccElement *getSlice(const char *axis, int sliceNum, int *pxdim, int *pydim);

    int writeDen(const char *);
    int write(const char *);
    int read(const char *);

    ~OccGrid();
};


#endif

