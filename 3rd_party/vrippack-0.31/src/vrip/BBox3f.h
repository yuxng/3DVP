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


#ifndef _BBOX3F_
#define _BBOX3F_

#include "Linear.h"

class BBox3f {

  public:
    Vec3f fll, nur;

    void init();
    void grow(BBox3f &bbox);
    void getSphere(Vec3f *center, float *radius);
    void update(Vec3f &);
    void clear();
};


#endif
