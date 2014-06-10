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


#ifndef _OCC_
#define _OCC_


#include "defines.h"


typedef float vec3f[3];
typedef float vec2f[2];


struct BBox3f {
    vec3f fll, nur;
};


struct BBox2f {
    vec2f fll, nur;
};

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


enum {X_AXIS, Y_AXIS, Z_AXIS};

#define FAR_AWAY_DEPTH -1e20
#define IS_VALID_DEPTH(z) (z > FAR_AWAY_DEPTH)
#define MAX_DEPTH_DIFFERENCE 0.004
#define CHUNK_SIZE 1000000
#define DEPTH_TREE_GRANULARITY 8


#endif
