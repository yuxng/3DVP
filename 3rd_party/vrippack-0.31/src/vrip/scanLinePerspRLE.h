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


#include "vrip.h"
#include "OccGridRLE.h"
#include "DepthMap.h"

void scanConvertLinePerspTree(OccGridRLE *gridIn, OccGridRLE *gridOut, 
			      OrthoShear *shear, DepthMap *depthMap);
void scanConvertLinePerspTreeDragTails(OccGridRLE *gridIn, 
				       OccGridRLE *gridOut, 
				       OrthoShear *shear, DepthMap *depthMap);
void scanConvertLinePerspTreeTailsOnly(OccGridRLE *gridIn, 
				       OccGridRLE *gridOut, 
				       OrthoShear *shear, DepthMap *depthMap);
void scanConvertLinePerspTreeTailsOnlyFast(OccGridRLE *gridIn, 
					   OccGridRLE *gridOut, 
					   OrthoShear *shear, 
					   DepthMap *depthMap);
