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


#include "perspective.h"
#include "vrip.h"
#include "configure.h"
#include "vripGlobals.h"


Vec3f thePerspViewRay;
Vec3f theCOP;
OrthoShear *thePerspShear;
float theFocalLength;

OrthoShear *
initPersp(Quaternion &quat, Vec3f &trans, Vec3f &center, 
	  float resolution)
{
    Vec3f vec, dir, ctr;
    Matrix4f mrot, mtrans, mpermute, mflipz;
    Matrix4f mrotz, mtransyz, mshear, mprod;
    Matrix4f mrotzinv, mtransyzinv, mshearinv;
    float dot;
    Matrix4f mrotx, mrotxinv;

    ctr = center;
    thePerspViewRay.setValue(PerspectiveDir);
    theCOP.setValue(PerspectiveCOP);

    // Set up the pose matrices
    quat.toMatrix(mrot);
    mtrans.makeIdentity();
    mtrans.translate(trans);

    // Transform the COP and center ray
    mrot.multVec(thePerspViewRay, vec);
    thePerspViewRay.setValue(vec);
    mrot.multVec(theCOP, vec);
    theCOP.setValue(vec);
    theCOP += trans;

    if (Verbose) {
       printf("\nCOP: (%f, %f, %f)\n", theCOP.x, theCOP.y, theCOP.z);
       printf("View direction: (%f, %f, %f)\n", thePerspViewRay.x, thePerspViewRay.y, thePerspViewRay.z);
    }

    vec3f rayDir;
    rayDir[0] = thePerspViewRay.x;
    rayDir[1] = thePerspViewRay.y;
    rayDir[2] = thePerspViewRay.z;
    OrthoShear *shear = computeShear(rayDir);
    thePerspShear = shear;
    
    // Set up permutation matrix

    mpermute.makeIdentity();
    if (shear->axis == X_AXIS) {
	mpermute.m[0][0] = 0;
	mpermute.m[0][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][0] = 1;
    } 
    else if (shear->axis == Y_AXIS) {
	mpermute.m[1][1] = 0;
	mpermute.m[1][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][1] = 1;
    }

    // Update the COP and center of image plane in the new coordinate system
    mpermute.multVec(theCOP, vec);
    theCOP.setValue(vec);
    mpermute.multVec(ctr, vec);
    ctr.setValue(vec);

    // Set up flip matrix
    mflipz.makeIdentity();
    if (shear->flip)
	mflipz.m[2][2] = -1;
    

    // Update the COP and image center in the new flipped coordinate system
    mflipz.multVec(theCOP, vec);
    theCOP.setValue(vec);
    mflipz.multVec(ctr, vec);
    ctr.setValue(vec);

    Matrix4f msh;
    mshear.makeIdentity();
    mshear.m[0][2] = shear->sx;
    mshear.m[1][2] = shear->sy;

    // Update the COP and image center
    mshear.multVec(theCOP, vec);
    theCOP.setValue(vec);
    mshear.multVec(ctr, vec);
    ctr.setValue(vec);

    theFocalLength = ctr.z - theCOP.z;

    if (Verbose) {
	printf("Axis = %d (x=%d, y=%d, z=%d),   flip = %d,  sx = %f,  sy = %f\n", 
	       shear->axis, X_AXIS, Y_AXIS, Z_AXIS, shear->flip, shear->sx, shear->sy);
	printf("\nTransformed COP: (%f, %f, %f)\n", theCOP.x, theCOP.y, theCOP.z);
	printf("\nFocal length: %f\n", theFocalLength);
    }

   return shear;
}


void 
applyPersp(Vec3f &vin, Vec3f &vout) {
   // apply shear
   vout.x = vin.x + vin.z*thePerspShear->sx;
   vout.y = vin.y + vin.z*thePerspShear->sy;

   // translate eye to origin
   vout.x = vout.x - theCOP.x;
   vout.y = vout.y - theCOP.y;

   // apply projection
   vout.x = vout.x*theFocalLength/(vin.z - theCOP.z);
   vout.y = vout.y*theFocalLength/(vin.z - theCOP.z);

   vout.z = vin.z;
}


void 
applyInvPersp(Vec3f &vin, Vec3f &vout) 
{
   // un-project
   vout.x = vin.x*(vin.z - theCOP.z)/theFocalLength;
   vout.y = vin.y*(vin.z - theCOP.z)/theFocalLength;

   // undo translation of eye to origin
   vout.x = vout.x + theCOP.x;
   vout.y = vout.y + theCOP.y;

   // un-apply shear
   vout.x = vout.x - vin.z*thePerspShear->sx;
   vout.y = vout.y - vin.z*thePerspShear->sy;

   vout.z = vin.z;
}

