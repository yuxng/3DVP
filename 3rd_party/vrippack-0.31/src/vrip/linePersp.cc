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


#include "linePersp.h"
#include "vrip.h"
#include "vripGlobals.h"


Vec3f theRayDir;
Vec3f thePoint;
Matrix4f theMat1, theMat1Inverse;
Matrix4f theMat2, theMat2Inverse;
float thePerspA, thePerspB;


OrthoShear *
initLinePersp(Quaternion &quat, Vec3f &trans, Vec3f &center, float resolution)
{
    Vec3f vec, dir;
    Matrix4f mrot, mtrans, mpermute, mflipz;
    Matrix4f mrotz, mtransyz, mshear, mprod;
    Matrix4f mrotzinv, mtransyzinv, mshearinv;
    float dot;

    // Set up the laser projection line

    theRayDir.setValue(LASER_LINE_DIR_X, LASER_LINE_DIR_Y, LASER_LINE_DIR_Z);
    thePoint.setValue(LASER_LINE_AT_T0_X, LASER_LINE_AT_T0_Y, 
		      LASER_LINE_AT_T0_Z);

    

    // Set up the pose matrices
    quat.toMatrix(mrot);
    mtrans.makeIdentity();
    mtrans.translate(trans);


    // Transform the line

    mrot.multVec(theRayDir, vec);
    theRayDir.setValue(vec);
    mrot.multVec(thePoint, vec);
    thePoint.setValue(vec);
    thePoint += trans;
    

    // Choose permutation and flip

    vec = thePoint - center;
    dot = -vec.dot(theRayDir);
    dir.setValue(theRayDir);
    dir *= dot;
    dir += vec;


    OrthoShear *shear = new OrthoShear;

    shear->flip = FALSE;
    if (fabs(dir[0]) > fabs(dir[1])) {
	if (fabs(dir[0]) > fabs(dir[2])) {
	    shear->axis = X_AXIS;
	    shear->flip = dir[0] < 0;
	}
	else {
	    shear->axis = Z_AXIS;
	    shear->flip = dir[2] < 0;
	}
    } 
    else if (fabs(dir[1]) > fabs(dir[2])) {
	shear->axis = Y_AXIS;
	shear->flip = dir[1] < 0;
    }
    else {
	shear->axis = Z_AXIS;
	shear->flip = dir[2] < 0;
    }

    
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


    // Update the line into the new permuted coordinate system

    mpermute.multVec(theRayDir, vec);
    theRayDir.setValue(vec);
    mpermute.multVec(thePoint, vec);
    thePoint.setValue(vec);


    // Set up flip matrix

    mflipz.makeIdentity();
    if (shear->flip)
	mflipz.m[2][2] = -1;
    

    // Update the line into the new flipped coordinate system

    mflipz.multVec(theRayDir, vec);
    theRayDir.setValue(vec);
    mflipz.multVec(thePoint, vec);
    thePoint.setValue(vec);


    // Rotate the direction parallel to the xz plane

    mrotz.makeIdentity();
    float mag = 1/sqrt(theRayDir.x*theRayDir.x + theRayDir.y*theRayDir.y);
    mrotz.m[0][0] = theRayDir.x*mag;
    mrotz.m[0][1] = theRayDir.y*mag;
    mrotz.m[1][0] = -theRayDir.y*mag;
    mrotz.m[1][1] = theRayDir.x*mag;


    // Update the laser line

    mrotz.multVec(theRayDir, vec);
    theRayDir.setValue(vec);
    mrotz.multVec(thePoint, vec);
    thePoint.setValue(vec);


    // Translate into the x-z plane and move in depth so that the
    // image plane is at z = 0

    mtransyz.makeIdentity();
    mtransyz.translate(0, -thePoint.y, -center.z);

    // Update the laser line

    mtransyz.multVec(thePoint, vec);
    thePoint.setValue(vec);


    // Shear the projection planes to be perpendicular to the x-y plane

    mshear.makeIdentity();
    mshear.m[0][2] = theRayDir.z/theRayDir.x;

    // Update the laser line

    mshear.multVec(theRayDir, vec);
    theRayDir.setValue(vec);
    mshear.multVec(thePoint, vec);
    thePoint.setValue(vec);


    // Compute the front end matrix

    theMat1.makeIdentity();
    theMat1.multLeft(mrotz);
    theMat1.multLeft(mtransyz);
    theMat1.multLeft(mshear);

    mrotzinv.setValue(mrotz);
    mrotzinv.transpose();

    mtransyzinv.setValue(mtransyz);
    mtransyzinv.m[0][3] = -mtransyzinv.m[0][3];
    mtransyzinv.m[1][3] = -mtransyzinv.m[1][3];
    mtransyzinv.m[2][3] = -mtransyzinv.m[2][3];

    mshearinv.setValue(mshear);
    mshearinv.m[0][2] = -mshearinv.m[0][2];

    theMat1Inverse.makeIdentity();
    theMat1Inverse.multLeft(mshearinv);
    theMat1Inverse.multLeft(mtransyzinv);
    theMat1Inverse.multLeft(mrotzinv);


    // Compute the components of the perspective-in-y transformation

    thePerspA = theRayDir.z/theRayDir.x;
    thePerspB = thePoint.z - thePerspA*thePoint.x;


    // Compute the back end transformation

    theMat2.makeIdentity();
    theMat2.multLeft(mtransyzinv);
    theMat2.multLeft(mrotzinv);

    theMat2Inverse.makeIdentity();
    theMat2Inverse.multLeft(mrotz);
    theMat2Inverse.multLeft(mtransyz);


    // Extract the approximate shear
    mprod.setValue(theMat1);
    mprod.multLeft(theMat2);
    shear->sx = mprod.m[0][2];
    shear->sy = mprod.m[1][2];

    if (Verbose) {
	printf("Axis = %d,   flip = %d,  sx = %f,  sy = %f\n", 
	       shear->axis, shear->flip, shear->sx, shear->sy);

	printf("Matrix 1:\n");
	theMat1.print();

	printf("\nPersp: a = %f  b = %f\n", thePerspA, thePerspB);

	printf("\nMatrix 2:\n");
	theMat2.print();

	printf("\nMatrix 2 * matrix 1:\n");
	mprod.print();

	mprod.setValue(theMat1);
	mprod.multLeft(theMat1Inverse);
	printf("\nInv(matrix 1) * matrix 1:\n");
	mprod.print();

	mprod.setValue(theMat2);
	mprod.multLeft(theMat2Inverse);
	printf("\nInv(matrix 2) * matrix 2:\n");
	mprod.print();
    }

#if 0
    Vec3f temp(1,2,3);
    Vec3f temp2, temp3;
    applyLinePersp(temp, temp2);
    applyInvLinePersp(temp2, temp3);
    printf("%f, %f, %f  ->  %f, %f, %f\n",
	   temp.x, temp.y, temp.z, temp3.x, temp3.y, temp3.z);
#endif

   return shear;
}


void
applyLinePersp(Vec3f &vin, Vec3f &vout)
{
    Vec3f vec;

    theMat1.multVec(vin, vec);
    
    vec.y = vec.y/(1-vec.z/(thePerspA*vec.x + thePerspB));

    theMat2.multVec(vec, vout);
}


void
applyInvLinePersp(Vec3f &vin, Vec3f &vout)
{
    Vec3f vec;

    theMat2Inverse.multVec(vin, vec);
    
    vec.y = vec.y*(1-vec.z/(thePerspA*vec.x + thePerspB));

    theMat1Inverse.multVec(vec, vout);
}


