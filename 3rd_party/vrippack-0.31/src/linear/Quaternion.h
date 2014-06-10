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


#ifndef _QUATERNION_
#define _QUATERNION_

#include "Matrix4f.h"

class Quaternion {

  public:
    float q[4];

    Quaternion();
    ~Quaternion();

    void setValue(Quaternion &q);
    void setValue(float q0, float q1, float q2, float q3);
    void setValue(float *quat);

    void toMatrix(Matrix4f &);
    void fromMatrix(Matrix4f &);

    void normalize();
    void toAxisAngle(float &angle, Vec3f &axis);
	
};



#endif

