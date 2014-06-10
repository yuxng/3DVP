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


#include <Linear.h>

#include "vrip.h"

OrthoShear *initLinePersp(Quaternion &quat, Vec3f &trans, 
			  Vec3f &center, float resolution);
void applyLinePersp(Vec3f &vin, Vec3f &vout);
void applyInvLinePersp(Vec3f &vin, Vec3f &vout);
