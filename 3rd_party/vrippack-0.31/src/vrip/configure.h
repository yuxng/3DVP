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
#include "BBox3f.h"
#include "OccGrid.h"
#include "OccGridRLE.h"
#include "OccGridNormRLE.h"

OrthoShear *computeShear(vec3f dir);
BBox3f *getBBox(OccGrid *grid);
BBox3f *getBBox(OccGridRLE *grid);
BBox3f *getBBox(OccGridNormRLE *grid);
BBox3f *getBBoxLinePersp(OccGridRLE *grid, int *numLines);
BBox3f *getBBoxPersp(OccGridRLE *grid, int *numLines);
void configureGrid(OccGrid *grid, OrthoShear *shear);
void configureGrid(OccGridNormRLE **pInGrid, OccGridNormRLE **pOutGrid, 
		   OrthoShear *shear);
void configureGrid(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, 
		   OrthoShear *shear);
void configureGridLinePersp(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, 
			    OrthoShear *shear);
void configureGridPersp(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, 
			OrthoShear *shear);
