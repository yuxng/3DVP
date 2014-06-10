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


#ifndef _RESAMPLE_
#define _RESAMPLE_

#include "DepthMap.h"
#include "Linear.h"

void resample(DepthMap *depthMap, float xOff, float yOff, 
	      float *depth, float *confidence);

void resampleBetter(DepthMap *depthMap, float xOff, float yOff, 
		    float *depth, float *confidence);

void resampleWithEdge(DepthMap *depthMap, float xOff, float yOff, 
		      float *depth, float *confidence, float *edge);

void resampleForCarving(DepthMap *depthMap, float xOff, float yOff, 
			float *depth, float *confidence);

void resampleNorm(DepthMap *depthMap, float xOff, float yOff, 
		  float *depth, float *confidence, Vec3f &norm);

#endif

