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


#include "BBox3f.h"

#ifdef linux
#include <float.h>
#endif

void
BBox3f::init()
{
    fll.setValue(FLT_MAX, FLT_MAX, FLT_MAX);
    nur.setValue(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}


void
BBox3f::grow(BBox3f &bbox)
{
    fll.setValue(MIN(fll.x, bbox.fll.x),
		 MIN(fll.y, bbox.fll.y),
		 MIN(fll.z, bbox.fll.z));

    nur.setValue(MAX(nur.x, bbox.nur.x),
		 MAX(nur.y, bbox.nur.y),
		 MAX(nur.z, bbox.nur.z));
}


void
BBox3f::getSphere(Vec3f *center, float *radius)
{
    Vec3f diag;

    *center = fll + nur;
    *center /= 2;

    diag = nur - fll;
    diag /= 2;
    *radius = diag.length();
}


void
BBox3f::update(Vec3f &v)
{
    fll.setValue(MIN(fll.x, v.x),
		 MIN(fll.y, v.y),
		 MIN(fll.z, v.z));

    nur.setValue(MAX(nur.x, v.x),
		 MAX(nur.y, v.y),
		 MAX(nur.z, v.z));
}

void
BBox3f::clear()
{
    fll.setValue(0,0,0);
    nur.setValue(0,0,0);
}


