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


#include "vripGlobals.h"
#include "configure.h"
#include "linePersp.h"
#include "perspective.h"

OrthoShear *
computeShear(vec3f dir)
{
    vec3f newDir;

    OrthoShear *shear = new OrthoShear;

    if (fabs(dir[0]) > fabs(dir[1])) {
	if (fabs(dir[0]) > fabs(dir[2]))
	    shear->axis = X_AXIS;
	else
	    shear->axis = Z_AXIS;
    } 
    else if (fabs(dir[1]) > fabs(dir[2]))
	shear->axis = Y_AXIS;
    else
	shear->axis = Z_AXIS;


    // Assumes a transpose operation

    switch (shear->axis) {

      case X_AXIS:
	newDir[0] = dir[2];
	newDir[1] = dir[1];
	newDir[2] = dir[0];
	break;

     case Y_AXIS:
	newDir[0] = dir[0];
	newDir[1] = dir[2];
	newDir[2] = dir[1];
	break;

      case Z_AXIS:
	newDir[0] = dir[0];
	newDir[1] = dir[1];
	newDir[2] = dir[2];
	break;
    }

    shear->flip = newDir[2] > 0;

    shear->sx = newDir[0]/fabs(newDir[2]);
    shear->sy = newDir[1]/fabs(newDir[2]);

    if (Verbose)
	printf("Shear: sx = %f  sy = %f  axis = %d  flip = %d\n", 
	       shear->sx, shear->sy, shear->axis, shear->flip);

    return shear;
}


void
configureGrid(OccGrid *grid, OrthoShear *shear)
{
    // Unflip the grid, if necessary
    if (grid->flip) {
	grid->flip = FALSE;
	grid->origin[2] = -grid->origin[2];
    }

    // Transpose to the desired axis
    if (shear->axis != grid->axis) {
	if (grid->axis == X_AXIS)
	    grid->transposeXZ();
	else if (grid->axis == Y_AXIS)
	    grid->transposeYZ();
	
	if (shear->axis == X_AXIS)
	    grid->transposeXZ();
	else if (shear->axis == Y_AXIS)
	    grid->transposeYZ();

	grid->axis = shear->axis;
    }

    // Flip the grid, if necessary
    grid->flip = shear->flip;
    grid->origin[2] = grid->flip ? -grid->origin[2] : grid->origin[2];

    float zinc = grid->flip ? -grid->resolution : grid->resolution;
    float z = grid->origin[2];
    for (int i = 0; i < grid->zdim; i++, z+= zinc) {
	grid->sliceOrigins[i][0] = grid->origin[0] + shear->sx * z;
	grid->sliceOrigins[i][1] = grid->origin[1] + shear->sy * z;
	grid->sliceOrigins[i][2] = z;
    }
}


void
configureGrid(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, OrthoShear *shear)
{
    OccGridRLE *temp, *inGrid, *outGrid;

    inGrid = *pInGrid;
    outGrid = *pOutGrid;

    // Unflip the grid, if necessary
    if (inGrid->flip) {
	inGrid->flip = FALSE;
	inGrid->origin[2] = -inGrid->origin[2];
    }

    outGrid->copyParams(inGrid);

    // Transpose to the desired axis
    if (shear->axis != inGrid->axis) {
	if (inGrid->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	else if (inGrid->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	
	if (shear->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	}
	else if (shear->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	}
	else {
	    SWAP(inGrid, outGrid, temp);
	}	    

	outGrid->axis = shear->axis;
    }
    else {
	SWAP(inGrid, outGrid, temp);
    }    

    // Flip the grid, if necessary
    outGrid->flip = shear->flip;
    outGrid->origin[2] = outGrid->flip ? -outGrid->origin[2] : outGrid->origin[2];

    float zinc = outGrid->flip ? -outGrid->resolution : outGrid->resolution;

    float z = outGrid->origin[2];

    for (int i = 0; i < outGrid->zdim; i++, z+= zinc) {
	outGrid->sliceOrigins[i][0] = outGrid->origin[0] + shear->sx * z;
	outGrid->sliceOrigins[i][1] = outGrid->origin[1] + shear->sy * z;
	outGrid->sliceOrigins[i][2] = z;

	inGrid->sliceOrigins[i][0] = inGrid->origin[0] + shear->sx * z;
	inGrid->sliceOrigins[i][1] = inGrid->origin[1] + shear->sy * z;
	inGrid->sliceOrigins[i][2] = z;
    }

    inGrid->copyParams(outGrid);

    *pInGrid = inGrid;
    *pOutGrid = outGrid;
}


void
configureGrid(OccGridNormRLE **pInGrid, OccGridNormRLE **pOutGrid, 
	      OrthoShear *shear)
{
    OccGridNormRLE *temp, *inGrid, *outGrid;

    inGrid = *pInGrid;
    outGrid = *pOutGrid;

    outGrid->copyParams(inGrid);

    // Transpose to the desired axis
    if (shear->axis != inGrid->axis) {
	if (inGrid->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	else if (inGrid->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	
	if (shear->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	}
	else if (shear->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	}
	else {
	    SWAP(inGrid, outGrid, temp);
	}	    

	outGrid->axis = shear->axis;
    }
    else {
	SWAP(inGrid, outGrid, temp);
    }    

    // Flip the grid, if necessary
    outGrid->flip = shear->flip;
    outGrid->origin[2] = outGrid->flip ? -outGrid->origin[2] : outGrid->origin[2];

    float zinc = outGrid->flip ? -outGrid->resolution : outGrid->resolution;

    float z = outGrid->origin[2];

    for (int i = 0; i < outGrid->zdim; i++, z+= zinc) {
	outGrid->sliceOrigins[i][0] = outGrid->origin[0] + shear->sx * z;
	outGrid->sliceOrigins[i][1] = outGrid->origin[1] + shear->sy * z;
	outGrid->sliceOrigins[i][2] = z;
    }

    inGrid->copyParams(outGrid);

    *pInGrid = inGrid;
    *pOutGrid = outGrid;
}


void
configureGridLinePersp(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, 
		       OrthoShear *shear)
{
    OccGridRLE *temp, *inGrid, *outGrid;

    inGrid = *pInGrid;
    outGrid = *pOutGrid;

    // Unflip the grid, if necessary
    if (inGrid->flip) {
	inGrid->flip = FALSE;
	inGrid->origin[2] = -inGrid->origin[2];
    }

    outGrid->copyParams(inGrid);

    // Transpose to the desired axis
    if (shear->axis != inGrid->axis) {
	if (inGrid->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	else if (inGrid->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	
	if (shear->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	}
	else if (shear->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	}
	else {
	    SWAP(inGrid, outGrid, temp);
	}	    

	outGrid->axis = shear->axis;
    }
    else {
	SWAP(inGrid, outGrid, temp);
    }    

    // Flip the grid, if necessary
    outGrid->flip = shear->flip;
    outGrid->origin[2] = outGrid->flip ? 
	-outGrid->origin[2] : outGrid->origin[2];

    float zinc = outGrid->flip ? -outGrid->resolution : outGrid->resolution;
    float z = outGrid->origin[2];
    for (int i = 0; i < outGrid->zdim; i++, z+= zinc) {
	outGrid->sliceOrigins[i][0] = outGrid->origin[0];
	outGrid->sliceOrigins[i][1] = outGrid->origin[1];
	outGrid->sliceOrigins[i][2] = z;
    }

    inGrid->copyParams(outGrid);

    *pInGrid = inGrid;
    *pOutGrid = outGrid;
}


void
configureGridPersp(OccGridRLE **pInGrid, OccGridRLE **pOutGrid, 
		   OrthoShear *shear)
{
    OccGridRLE *temp, *inGrid, *outGrid;

    inGrid = *pInGrid;
    outGrid = *pOutGrid;

    // Unflip the grid, if necessary
    if (inGrid->flip) {
	inGrid->flip = FALSE;
	inGrid->origin[2] = -inGrid->origin[2];
    }

    outGrid->copyParams(inGrid);

    // Transpose to the desired axis
    if (shear->axis != inGrid->axis) {
	if (inGrid->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	else if (inGrid->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	    SWAP(inGrid, outGrid, temp);
	}
	
	if (shear->axis == X_AXIS) {
	    inGrid->transposeXZ(outGrid);
	}
	else if (shear->axis == Y_AXIS) {
	    inGrid->transposeYZ(outGrid);
	}
	else {
	    SWAP(inGrid, outGrid, temp);
	}	    

	outGrid->axis = shear->axis;
    }
    else {
	SWAP(inGrid, outGrid, temp);
    }    

    // Flip the grid, if necessary
    outGrid->flip = shear->flip;
    outGrid->origin[2] = outGrid->flip ? 
	-outGrid->origin[2] : outGrid->origin[2];

    float zinc = outGrid->flip ? -outGrid->resolution : outGrid->resolution;
    float z = outGrid->origin[2];
    for (int i = 0; i < outGrid->zdim; i++, z+= zinc) {
	outGrid->sliceOrigins[i][0] = outGrid->origin[0];
	outGrid->sliceOrigins[i][1] = outGrid->origin[1];
	outGrid->sliceOrigins[i][2] = z;
    }

    inGrid->copyParams(outGrid);

    *pInGrid = inGrid;
    *pOutGrid = outGrid;
}


BBox3f *
getBBox(OccGrid *grid) 
{
    float d1, d2, d3, d4, res;

    BBox3f *bbox = new BBox3f;

    res = grid->resolution;

    d1 = grid->sliceOrigins[0][0];
    d2 = grid->sliceOrigins[0][0] + grid->xdim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][0];
    d4 = grid->sliceOrigins[grid->zdim-1][0] + grid->xdim*res;
    bbox->fll.x = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.x = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][1];
    d2 = grid->sliceOrigins[0][1] + grid->ydim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][1];
    d4 = grid->sliceOrigins[grid->zdim-1][1] + grid->ydim*res;
    bbox->fll.y = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.y = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][2];
    d2 = grid->sliceOrigins[grid->zdim-1][2];
    bbox->fll.z = MIN(d1,d2);
    bbox->nur.z = MAX(d1,d2);

    return bbox;
}


BBox3f *
getBBox(OccGridRLE *grid) 
{
    float d1, d2, d3, d4, res;

    BBox3f *bbox = new BBox3f;

    res = grid->resolution;

    d1 = grid->sliceOrigins[0][0];
    d2 = grid->sliceOrigins[0][0] + grid->xdim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][0];
    d4 = grid->sliceOrigins[grid->zdim-1][0] + grid->xdim*res;
    bbox->fll.x = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.x = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][1];
    d2 = grid->sliceOrigins[0][1] + grid->ydim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][1];
    d4 = grid->sliceOrigins[grid->zdim-1][1] + grid->ydim*res;
    bbox->fll.y = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.y = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][2];
    d2 = grid->sliceOrigins[grid->zdim-1][2];
    bbox->fll.z = MIN(d1,d2);
    bbox->nur.z = MAX(d1,d2);

    return bbox;
}


BBox3f *
getBBox(OccGridNormRLE *grid) 
{
    float d1, d2, d3, d4, res;

    BBox3f *bbox = new BBox3f;

    res = grid->resolution;

    d1 = grid->sliceOrigins[0][0];
    d2 = grid->sliceOrigins[0][0] + grid->xdim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][0];
    d4 = grid->sliceOrigins[grid->zdim-1][0] + grid->xdim*res;
    bbox->fll.x = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.x = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][1];
    d2 = grid->sliceOrigins[0][1] + grid->xdim*res;
    d3 = grid->sliceOrigins[grid->zdim-1][1];
    d4 = grid->sliceOrigins[grid->zdim-1][1] + grid->ydim*res;
    bbox->fll.y = MIN(MIN(d1,d2),MIN(d3,d4));
    bbox->nur.y = MAX(MAX(d1,d2),MAX(d3,d4));

    d1 = grid->sliceOrigins[0][2];
    d2 = grid->sliceOrigins[grid->zdim-1][2];
    bbox->fll.z = MIN(d1,d2);
    bbox->nur.z = MAX(d1,d2);

    return bbox;
}


BBox3f *
getBBoxLinePersp(OccGridRLE *grid, int *numLines) 
{
    Vec3f v, v1, v2, v3, v4, v5, v6, v7, v8;
    float res, dely;

    BBox3f *bbox = new BBox3f;

    res = grid->resolution;

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[0][2]);
    applyLinePersp(v, v1);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[0][2]);
    applyLinePersp(v, v2);

    dely = fabs(v1.y - v2.y);

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[0][2]);
    applyLinePersp(v, v3);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[0][2]);
    applyLinePersp(v, v4);

    dely = MAX(dely, fabs(v3.y - v4.y));

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyLinePersp(v, v5);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyLinePersp(v, v6);

    dely = MAX(dely, fabs(v5.y - v6.y));

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyLinePersp(v, v7);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyLinePersp(v, v8);

    dely = MAX(dely, fabs(v7.y - v8.y));


    bbox->fll.x = MIN(MIN(MIN(v1.x,v2.x),MIN(v3.x,v4.x)), 
		      MIN(MIN(v5.x,v6.x),MIN(v7.x,v8.x)));
    bbox->nur.x = MAX(MAX(MAX(v1.x,v2.x),MAX(v3.x,v4.x)), 
		      MAX(MAX(v5.x,v6.x),MAX(v7.x,v8.x)));

    bbox->fll.y = MIN(MIN(MIN(v1.y,v2.y),MIN(v3.y,v4.y)), 
		      MIN(MIN(v5.y,v6.y),MIN(v7.y,v8.y)));
    bbox->nur.y = MAX(MAX(MAX(v1.y,v2.y),MAX(v3.y,v4.y)), 
		      MAX(MAX(v5.y,v6.y),MAX(v7.y,v8.y)));

    bbox->fll.z = MIN(MIN(MIN(v1.z,v2.z),MIN(v3.z,v4.z)), 
		      MIN(MIN(v5.z,v6.z),MIN(v7.z,v8.z)));
    bbox->nur.z = MAX(MAX(MAX(v1.z,v2.z),MAX(v3.z,v4.z)), 
		      MAX(MAX(v5.z,v6.z),MAX(v7.z,v8.z)));



    *numLines = int(ceil(dely/res))+2;

#if 0
    // Shouldn't need to multiply by 2
    *numLines = 2*int(ceil(dely/res))+2;
    if (Warn)
	printf("Using conservative line width in depth map.\n");
#endif

    if (Verbose) {
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll.x, bbox->fll.y, bbox->fll.z, 
	       bbox->nur.x, bbox->nur.y, bbox->nur.z);
    }

    return bbox;
}


BBox3f *
getBBoxPersp(OccGridRLE *grid, int *numLines) 
{
    Vec3f v, v1, v2, v3, v4, v5, v6, v7, v8;
    float res, dely;

    BBox3f *bbox = new BBox3f;

    res = grid->resolution;

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[0][2]);
    applyPersp(v, v1);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[0][2]);
    applyPersp(v, v2);

    dely = fabs(v1.y - v2.y);

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[0][2]);
    applyPersp(v, v3);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[0][2]);
    applyPersp(v, v4);

    dely = MAX(dely, fabs(v3.y - v4.y));

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyPersp(v, v5);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1], 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyPersp(v, v6);

    dely = MAX(dely, fabs(v5.y - v6.y));

    v.setValue(grid->sliceOrigins[0][0], 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyPersp(v, v7);

    v.setValue(grid->sliceOrigins[0][0] + grid->xdim*res, 
	       grid->sliceOrigins[0][1] + grid->ydim*res, 
	       grid->sliceOrigins[grid->zdim-1][2]);
    applyPersp(v, v8);

    dely = MAX(dely, fabs(v7.y - v8.y));


    bbox->fll.x = MIN(MIN(MIN(v1.x,v2.x),MIN(v3.x,v4.x)), 
		      MIN(MIN(v5.x,v6.x),MIN(v7.x,v8.x)));
    bbox->nur.x = MAX(MAX(MAX(v1.x,v2.x),MAX(v3.x,v4.x)), 
		      MAX(MAX(v5.x,v6.x),MAX(v7.x,v8.x)));

    bbox->fll.y = MIN(MIN(MIN(v1.y,v2.y),MIN(v3.y,v4.y)), 
		      MIN(MIN(v5.y,v6.y),MIN(v7.y,v8.y)));
    bbox->nur.y = MAX(MAX(MAX(v1.y,v2.y),MAX(v3.y,v4.y)), 
		      MAX(MAX(v5.y,v6.y),MAX(v7.y,v8.y)));

    bbox->fll.z = MIN(MIN(MIN(v1.z,v2.z),MIN(v3.z,v4.z)), 
		      MIN(MIN(v5.z,v6.z),MIN(v7.z,v8.z)));
    bbox->nur.z = MAX(MAX(MAX(v1.z,v2.z),MAX(v3.z,v4.z)), 
		      MAX(MAX(v5.z,v6.z),MAX(v7.z,v8.z)));



    *numLines = int(ceil(dely/res))+2;

#if 0
    // Shouldn't need to multiply by 2
    *numLines = 2*int(ceil(dely/res))+2;
    if (Warn)
	printf("Using conservative line width in depth map.\n");
#endif

    if (Verbose) {
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll.x, bbox->fll.y, bbox->fll.z, 
	       bbox->nur.x, bbox->nur.y, bbox->nur.z);
    }

    return bbox;
}

