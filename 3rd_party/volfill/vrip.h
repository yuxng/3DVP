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


#ifndef _VRIP_
#define _VRIP_


#include "defines.h"

typedef float vec3f[3];
typedef float vec2f[2];


struct OrthoShear {
    int axis;
    int flip;
    float sx;
    float sy;
};


struct OccElement {
    ushort value;
    ushort totalWeight;
};


struct OccElementDbl {
    double value;
    double totalWeight;
};


struct OccNormElement {
    ushort value;
    ushort totalWeight;
    signed char nx;
    signed char ny;
    signed char nz;
    uchar more;

#if 0

    ushort value2;
    ushort totalWeight2;
    char nx2;
    char ny2;
    char nz2;
    uchar more2;

    ushort value3;
    ushort totalWeight3;
    char nx3;
    char ny3;
    char nz3;
    uchar more3;

    ushort value4;
    ushort totalWeight4;
    char nx4;
    char ny4;
    char nz4;
#endif

};


struct OccNormElementDbl {
    double value;
    double totalWeight;
    double nx;
    double ny;
    double nz;

    double value2;
    double totalWeight2;
    double nx2;
    double ny2;
    double nz2;

    double value3;
    double totalWeight3;
    double nx3;
    double ny3;
    double nz3;

    double value4;
    double totalWeight4;
    double nx4;
    double ny4;
    double nz4;
};


enum {X_AXIS, Y_AXIS, Z_AXIS};

#define FAR_AWAY_DEPTH -1e20
#define IS_VALID_DEPTH(z) (z > FAR_AWAY_DEPTH)
#define CHUNK_SIZE 1000000
#define DEPTH_TREE_GRANULARITY 8


#endif

