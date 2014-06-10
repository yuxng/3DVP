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


#ifndef _OCC_FUNC_
#define _OCC_FUNC_

#include "vrip.h"
#include "Linear.h"

void initOccFunc();
void updateCell(float cellDepth, float rangeDepth, float sampleSpacing,
		float rangeConfidence, OccElement *cell);
void updateCellForCarving(float cellDepth, float rangeDepth, 
			  float sampleSpacing,
			  float rangeConfidence, OccElement *cell);

void updateCell(float cellDepth, float rangeDepth, Vec3f &norm,
		float sampleSpacing, float rangeConfidence, 
		OccNormElement *cell);
void updateCellForCarving(float cellDepth, float rangeDepth, 
			  float sampleSpacing,
			  float rangeConfidence, 
			  OccNormElement *cell);

void updateCellBiasEdgesEmpty(float cellDepth, float rangeDepth, 
			      float sampleSpacing,
			      float rangeConfidence,
			      float edgeConfidence, 
			      OccElement *cell);

#endif
