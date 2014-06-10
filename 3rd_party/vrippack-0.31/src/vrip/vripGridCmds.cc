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


#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#include "vrip.h"
#include "vripGridCmds.h"
#include "vripGlobals.h"
#include "vripAux.h"


const int CONSERVATIVE_AVG_DOWN = 0;


int
Vrip_InfoRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    char str[PATH_MAX];

    sprintf(str, "xdim %d ydim %d zdim %d resolution %f axis %d",
	    frontRLEGrid->xdim, frontRLEGrid->ydim, frontRLEGrid->zdim,
	    frontRLEGrid->resolution, frontRLEGrid->axis);

    Tcl_SetResult(interp, str, TCL_VOLATILE);

    return TCL_OK;
}



int
Vrip_InfoNormRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    char str[PATH_MAX];

    sprintf(str, "xdim %d ydim %d zdim %d resolution %f axis %d",
	    frontRLEGridNorm->xdim, frontRLEGridNorm->ydim, frontRLEGridNorm->zdim,
	    frontRLEGridNorm->resolution, frontRLEGridNorm->axis);

    Tcl_SetResult(interp, str, TCL_VOLATILE);

    return TCL_OK;
}


int
Vrip_NewGridCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int dim;
    float res;
    vec3f origin;

    if (argc != 3 && argc != 6) {
	interp->result = "Usage: vrip_newgrid <dim> <res> [<ox> <oy> <oz>]";
	return TCL_ERROR;
    }
    
    dim = atoi(argv[1]);
    res = atof(argv[2]);

    if (argc == 6) {
	origin[0] = atof(argv[3])-dim*res/2;
	origin[1] = atof(argv[4])-dim*res/2;
	origin[2] = atof(argv[5])-dim*res/2;
    } else {
	origin[0] = -dim*res/2;
	origin[1] = 0.15 - dim*res/2;
	origin[2] = -dim*res/2;
    }

    if (theGrid != NULL) {
	delete theGrid;
    }

    theGrid = new OccGrid(dim,dim,dim);
    theGrid->resolution = res;
    theGrid->origin[0] = origin[0];
    theGrid->origin[1] = origin[1];
    theGrid->origin[2] = origin[2];

    theGrid->clear();

    return TCL_OK;
}


int
Vrip_NewGridRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xdim, ydim, zdim, maxDim;
    float res;
    vec3f origin;

    if (argc != 3 && argc != 5 && argc != 6 && argc != 8) {
	interp->result = "Usage: vrip_newgridrle <dim> [<dim> <dim>] <res> [<ox> <oy> <oz>]";
	return TCL_ERROR;
    }
    
    if (argc == 3 || argc == 6) {
	xdim = ydim = zdim = atoi(argv[1]);
	res = atof(argv[2]);
    } else {
	xdim = atoi(argv[1]);	
	ydim = atoi(argv[2]);	
	zdim = atoi(argv[3]);	
	res = atof(argv[4]);
    }

    if (argc == 3 || argc == 5) {
	origin[0] = -xdim*res/2;
	origin[1] = 0.15 - ydim*res/2;
	origin[2] = -zdim*res/2;
    } else if (argc == 6) {
	origin[0] = atof(argv[3])-xdim*res/2;
	origin[1] = atof(argv[4])-ydim*res/2;
	origin[2] = atof(argv[5])-zdim*res/2;
    } else if (argc == 8) {
	origin[0] = atof(argv[5])-xdim*res/2;
	origin[1] = atof(argv[6])-ydim*res/2;
	origin[2] = atof(argv[7])-zdim*res/2;
    }

    if (backRLEGrid != NULL) {
	delete backRLEGrid;
    }

    if (frontRLEGrid != NULL) {
	delete frontRLEGrid;
    }

    if (theDepthMap != NULL) {
	delete theDepthMap;
    }


    backRLEGrid = new OccGridRLE(xdim, ydim, zdim, CHUNK_SIZE);
    backRLEGrid->resolution = res;
    backRLEGrid->origin[0] = origin[0];
    backRLEGrid->origin[1] = origin[1];
    backRLEGrid->origin[2] = origin[2];

    backRLEGrid->clear();

    frontRLEGrid = new OccGridRLE(xdim, ydim, zdim, CHUNK_SIZE);
    frontRLEGrid->resolution = res;
    frontRLEGrid->origin[0] = origin[0];
    frontRLEGrid->origin[1] = origin[1];
    frontRLEGrid->origin[2] = origin[2];

    frontRLEGrid->clear();


    if (UseTails) {
	Tcl_Eval(interp, "vrip_fillrle 1 0");
    }

    // Could this be tighter?
    maxDim = MAX(xdim, MAX(ydim, zdim));
    theDepthMap = new DepthMap(int(maxDim*2.2+20), int(maxDim*2.2+20), 
			       FALSE, DEPTH_TREE_GRANULARITY);

    return TCL_OK;
}


int
Vrip_NewGridNormRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xdim, ydim, zdim, maxDim;
    float res;
    vec3f origin;

    if (argc != 3 && argc != 5 && argc != 6 && argc != 8) {
	interp->result = "Usage: vrip_newgridnormrle <dim> [<dim> <dim>] <res> [<ox> <oy> <oz>]";
	return TCL_ERROR;
    }
    
    if (argc == 3 || argc == 6) {
	xdim = ydim = zdim = atoi(argv[1]);
	res = atof(argv[2]);
    } else {
	xdim = atoi(argv[1]);	
	ydim = atoi(argv[2]);	
	zdim = atoi(argv[3]);	
	res = atof(argv[4]);
    }

    if (argc == 3 || argc == 5) {
	origin[0] = -xdim*res/2;
	origin[1] = 0.15 - ydim*res/2;
	origin[2] = -zdim*res/2;
    } else if (argc == 6) {
	origin[0] = atof(argv[3])-xdim*res/2;
	origin[1] = atof(argv[4])-ydim*res/2;
	origin[2] = atof(argv[5])-zdim*res/2;
    } else if (argc == 8) {
	origin[0] = atof(argv[5])-xdim*res/2;
	origin[1] = atof(argv[6])-ydim*res/2;
	origin[2] = atof(argv[7])-zdim*res/2;
    }

    if (backRLEGridNorm != NULL) {
	delete backRLEGridNorm;
    }

    if (frontRLEGridNorm != NULL) {
	delete frontRLEGridNorm;
    }

    if (theDepthMap != NULL) {
	delete theDepthMap;
    }


    backRLEGridNorm = new OccGridNormRLE(xdim, ydim, zdim, CHUNK_SIZE);
    backRLEGridNorm->resolution = res;
    backRLEGridNorm->origin[0] = origin[0];
    backRLEGridNorm->origin[1] = origin[1];
    backRLEGridNorm->origin[2] = origin[2];

    backRLEGridNorm->clear();

    frontRLEGridNorm = new OccGridNormRLE(xdim, ydim, zdim, CHUNK_SIZE);
    frontRLEGridNorm->resolution = res;
    frontRLEGridNorm->origin[0] = origin[0];
    frontRLEGridNorm->origin[1] = origin[1];
    frontRLEGridNorm->origin[2] = origin[2];

    frontRLEGridNorm->clear();


    if (UseTails) {
	Tcl_Eval(interp, "vrip_fillrle 1 0");
    }

    // Could this be tighter?
    maxDim = MAX(xdim, MAX(ydim, zdim));
    theDepthMap = new DepthMap(int(maxDim*2.2+20), int(maxDim*2.2+20), 
			       TRUE, DEPTH_TREE_GRANULARITY);

    return TCL_OK;
}


int
Vrip_AvgDownCmd(ClientData, Tcl_Interp *interp, int argc, const char *[])
{
    int xx, yy, zz, newXDim, newYDim, newZDim ;
    int xok, yok, zok, xinc, zinc, yinc;
    double value, totalWeight;
    OccElement *newBuf, *theBuf;
    OccGrid *newGrid;

    if (argc != 1) {
	interp->result = "Usage: vrip_avgdown";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated yet";
	return TCL_ERROR;
    }
    
    newXDim = int(ceil(theGrid->xdim/2.0));
    newYDim = int(ceil(theGrid->ydim/2.0));
    newZDim = int(ceil(theGrid->zdim/2.0));
    newGrid = new OccGrid(newXDim, newYDim, newZDim);

    xinc = 1;
    yinc = theGrid->xdim;
    zinc = theGrid->xdim*theGrid->ydim;
    newBuf = newGrid->elems;
    for (zz = 0; zz < newGrid->zdim; zz++) {
	zok = (2*zz) < (theGrid->zdim - 1);
	for (yy = 0; yy < newGrid->ydim; yy++) {
	    yok = (2*yy) < (theGrid->ydim - 1);
	    theBuf = theGrid->address(0, 2*yy, 2*zz);
	    for (xx = 0; xx < newGrid->xdim; xx++, newBuf++, theBuf+=2) {
		xok = (2*xx) < (theGrid->xdim - 1);
		value = theBuf->value;
		totalWeight = theBuf->totalWeight;
		if (xok) {
		    value += (theBuf+xinc)->value;
		    totalWeight += (theBuf+xinc)->totalWeight ;
		}
		if (yok) {
		    value += (theBuf+yinc)->value;
		    totalWeight  += (theBuf+yinc)->totalWeight ;
		}
		if (xok&&yok) {
		    value += (theBuf+xinc+yinc)->value;
		    totalWeight  += (theBuf+xinc+yinc)->totalWeight ;
		}
		if (zok) {
		    value += (theBuf+zinc)->value;
		    totalWeight  += (theBuf+zinc)->totalWeight ;
		}
		if (xok&&zok) {
		    value += (theBuf+xinc+zinc)->value;
		    totalWeight  += (theBuf+xinc+zinc)->totalWeight ;
		}
		if (yok&&zok) {
		    value += (theBuf+yinc+zinc)->value;
		    totalWeight  += (theBuf+yinc+zinc)->totalWeight ;
		}
		if (xok&&yok&&zok) {
		    value += (theBuf+xinc+yinc+zinc)->value;
		    totalWeight  += (theBuf+xinc+yinc+zinc)->totalWeight ;
		}
		newBuf->value = ushort(value/8);
		newBuf->totalWeight = ushort(totalWeight/8);
	    }
	}
    }


    // Coordinate systems???

    delete theGrid;
    
    theGrid = newGrid;

    return TCL_OK;
}


int
Vrip_AvgDownRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *[])
{
    int xx, yy, zz, newXDim, newYDim, newZDim;
    int xok, yok, zok;
    double value, totalWeight, newWeight;
    int zeroWeight;
    OccElementDbl *newBuf;
    OccElement *oldScanline;
    float newRes;
    
    if (argc != 1) {
	interp->result = "Usage: vrip_avgdown";
	return TCL_ERROR;
    }
    
    newXDim = int(ceil(frontRLEGrid->xdim/2.0));
    newYDim = int(ceil(frontRLEGrid->ydim/2.0));
    newZDim = int(ceil(frontRLEGrid->zdim/2.0));
    delete backRLEGrid;
    backRLEGrid = new OccGridRLE(newXDim, newYDim, newZDim, CHUNK_SIZE);
    newRes = backRLEGrid->resolution = frontRLEGrid->resolution * 2;

    backRLEGrid->reset();

    OccElementDbl *newScanlineDbl = new OccElementDbl[newXDim];
    OccElement *newScanline = new OccElement[newXDim];
    int *counts = new int[newXDim];
    int *countPtr;

    if (CONSERVATIVE_AVG_DOWN)
	printf("Using conservative averaging down...\n");

    printf("\n");
    for (zz = 0; zz < backRLEGrid->zdim; zz++) {
       printf("\rProcessing slice %d of %d...", zz+1, backRLEGrid->zdim);
       fflush(stdout);
	zok = (2*zz) < (frontRLEGrid->zdim - 1);
	for (yy = 0; yy < backRLEGrid->ydim; yy++) {
	    yok = (2*yy) < (frontRLEGrid->ydim - 1);
	    oldScanline = frontRLEGrid->getScanline(2*yy,2*zz);
	    newBuf = newScanlineDbl;
	    countPtr = counts;
	    for (xx = 0; xx < backRLEGrid->xdim; 
			 xx++, newBuf++, countPtr++, oldScanline+=2) {

		xok = (2*xx) < (frontRLEGrid->xdim - 1);

		*countPtr = 1;
		newBuf->value = oldScanline->value;
		newBuf->totalWeight = oldScanline->totalWeight;
		if (xok) {
		    newWeight = (oldScanline+1)->totalWeight;
		    if (newWeight > 0) {
		       if (newBuf->totalWeight == 0) {
			  newBuf->value = (oldScanline+1)->value;
			  newBuf->totalWeight = newWeight;
		       } else {
			  newBuf->value += (oldScanline+1)->value;
			  newBuf->totalWeight += newWeight;
			  *countPtr = *countPtr + 1;
		       }
		    }
		}
	    }

	    if (yok) {
		oldScanline = frontRLEGrid->getScanline(2*yy+1,2*zz);
		newBuf = newScanlineDbl;
		countPtr = counts;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, countPtr++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);


		    newWeight = oldScanline->totalWeight;
		    if (newWeight > 0) {
		       if (newBuf->totalWeight == 0) {
			  newBuf->value = oldScanline->value;
			  newBuf->totalWeight = newWeight;
		       } else {
			  newBuf->value += oldScanline->value;
			  newBuf->totalWeight += newWeight;
			  *countPtr = *countPtr + 1;
		       }
		    }
		    if (xok) {
		       newWeight = (oldScanline+1)->totalWeight;
		       if (newWeight > 0) {
			  if (newBuf->totalWeight == 0) {
			     newBuf->value = (oldScanline+1)->value;
			     newBuf->totalWeight = newWeight;
			  } else {
			     newBuf->value += (oldScanline+1)->value;
			     newBuf->totalWeight += newWeight;
			     *countPtr = *countPtr + 1;
			  }
		       }
		    }
		}
	    }

	    if (zok) {
		oldScanline = frontRLEGrid->getScanline(2*yy,2*zz+1);
		newBuf = newScanlineDbl;
		countPtr = counts;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, countPtr++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);


		    newWeight = oldScanline->totalWeight;
		    if (newWeight > 0) {
		       if (newBuf->totalWeight == 0) {
			  newBuf->value = oldScanline->value;
			  newBuf->totalWeight = newWeight;
		       } else {
			  newBuf->value += oldScanline->value;
			  newBuf->totalWeight += newWeight;
			  *countPtr = *countPtr + 1;
		       }
		    }
		    if (xok) {
		       newWeight = (oldScanline+1)->totalWeight;
		       if (newWeight > 0) {
			  if (newBuf->totalWeight == 0) {
			     newBuf->value = (oldScanline+1)->value;
			     newBuf->totalWeight = newWeight;
			  } else {
			     newBuf->value += (oldScanline+1)->value;
			     newBuf->totalWeight += newWeight;
			     *countPtr = *countPtr + 1;
			  }
		       }
		    }
		}
	    }

	    if (yok&&zok) {
		oldScanline = frontRLEGrid->getScanline(2*yy+1,2*zz+1);
		newBuf = newScanlineDbl;
		countPtr = counts;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, countPtr++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);


		    newWeight = oldScanline->totalWeight;
		    if (newWeight > 0) {
		       if (newBuf->totalWeight == 0) {
			  newBuf->value = oldScanline->value;
			  newBuf->totalWeight = newWeight;
		       } else {
			  newBuf->value += oldScanline->value;
			  newBuf->totalWeight += newWeight;
			  *countPtr = *countPtr + 1;
		       }
		    }
		    if (xok) {
		       newWeight = (oldScanline+1)->totalWeight;
		       if (newWeight > 0) {
			  if (newBuf->totalWeight == 0) {
			     newBuf->value = (oldScanline+1)->value;
			     newBuf->totalWeight = newWeight;
			  } else {
			     newBuf->value += (oldScanline+1)->value;
			     newBuf->totalWeight += newWeight;
			     *countPtr = *countPtr + 1;
			  }
		       }
		    }
		}
	    }

	    for (xx = 0; xx < backRLEGrid->xdim; xx++) {
		newScanline[xx].value = ushort(newScanlineDbl[xx].value/
					       counts[xx]);
		newScanline[xx].totalWeight = 
		    ushort(newScanlineDbl[xx].totalWeight/counts[xx]);
	    }

	    backRLEGrid->putScanline(newScanline, yy, zz);
	}
    }
    printf("\n");



    // Coordinate systems???

    delete frontRLEGrid;
    frontRLEGrid = new OccGridRLE(newXDim, newYDim, newZDim, CHUNK_SIZE);
    frontRLEGrid->resolution = newRes;
    swapgrids();
    
    return TCL_OK;
}

int
Vrip_AvgDownRLE_oldCmd(ClientData, Tcl_Interp *interp, int argc, const char *[])
{
    int xx, yy, zz, newXDim, newYDim, newZDim;
    int xok, yok, zok;
    double value, totalWeight, newWeight;
    int zeroWeight;
    OccElementDbl *newBuf;
    OccElement *oldScanline;
    float newRes;
    
    if (argc != 1) {
	interp->result = "Usage: vrip_avgdown";
	return TCL_ERROR;
    }
    
    newXDim = int(ceil(frontRLEGrid->xdim/2.0));
    newYDim = int(ceil(frontRLEGrid->ydim/2.0));
    newZDim = int(ceil(frontRLEGrid->zdim/2.0));
    newRes = frontRLEGrid->resolution/2;
    delete backRLEGrid;
    backRLEGrid = new OccGridRLE(newXDim, newYDim, newZDim, CHUNK_SIZE);
    backRLEGrid->resolution = newRes;

    backRLEGrid->reset();

    OccElementDbl *newScanlineDbl = new OccElementDbl[newXDim];
    OccElement *newScanline = new OccElement[newXDim];

    if (CONSERVATIVE_AVG_DOWN)
	printf("Using conservative averaging down...\n");

    for (zz = 0; zz < backRLEGrid->zdim; zz++) {
	zok = (2*zz) < (frontRLEGrid->zdim - 1);
	for (yy = 0; yy < backRLEGrid->ydim; yy++) {
	    yok = (2*yy) < (frontRLEGrid->ydim - 1);
	    oldScanline = frontRLEGrid->getScanline(2*yy,2*zz);
	    newBuf = newScanlineDbl;
	    for (xx = 0; xx < backRLEGrid->xdim; 
			 xx++, newBuf++, oldScanline+=2) {
		xok = (2*xx) < (frontRLEGrid->xdim - 1);
		value = oldScanline->value;
		newWeight = oldScanline->totalWeight;
		zeroWeight = newWeight == 0;
		totalWeight = newWeight;
		if (xok) {
		    value += (oldScanline+1)->value;
		    newWeight = (oldScanline+1)->totalWeight;
		    zeroWeight = (newWeight == 0) || zeroWeight;
		    totalWeight += newWeight;
		}
		newBuf->value = value;
		if (zeroWeight)
		    newBuf->totalWeight = 0;
		else
		    newBuf->totalWeight = totalWeight;
	    }

	    if (yok) {
		oldScanline = frontRLEGrid->getScanline(2*yy+1,2*zz);
		newBuf = newScanlineDbl;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);
		    value = oldScanline->value;
		    newWeight = oldScanline->totalWeight;
		    zeroWeight = newWeight == 0;
		    totalWeight = newWeight;
		    if (xok) {
			value += (oldScanline+1)->value;
			newWeight = (oldScanline+1)->totalWeight;
			zeroWeight = (newWeight == 0) || zeroWeight;
			totalWeight += newWeight;
		    }
		    newBuf->value += value;
		    if (CONSERVATIVE_AVG_DOWN && 
			(zeroWeight || newBuf->totalWeight == 0))
			newBuf->totalWeight = 0;
		    else
			newBuf->totalWeight += totalWeight;
		}
	    }

	    if (zok) {
		oldScanline = frontRLEGrid->getScanline(2*yy,2*zz+1);
		newBuf = newScanlineDbl;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);
		    value = oldScanline->value;
		    newWeight = oldScanline->totalWeight;
		    zeroWeight = newWeight == 0;
		    totalWeight = newWeight;
		    if (xok) {
			value += (oldScanline+1)->value;
			newWeight = (oldScanline+1)->totalWeight;
			zeroWeight = (newWeight == 0) || zeroWeight;
			totalWeight += newWeight;
		    }
		    newBuf->value += value;
		    if (CONSERVATIVE_AVG_DOWN && 
			(zeroWeight || newBuf->totalWeight == 0))
			newBuf->totalWeight = 0;
		    else
			newBuf->totalWeight += totalWeight;
		}
	    }

	    if (yok&&zok) {
		oldScanline = frontRLEGrid->getScanline(2*yy+1,2*zz+1);
		newBuf = newScanlineDbl;
		for (xx = 0; xx < backRLEGrid->xdim; 
			     xx++, newBuf++, oldScanline+=2) {
		    xok = (2*xx) < (frontRLEGrid->xdim - 1);
		    value = oldScanline->value;
		    newWeight = oldScanline->totalWeight;
		    zeroWeight = newWeight == 0;
		    totalWeight = newWeight;
		    if (xok) {
			value += (oldScanline+1)->value;
			newWeight = (oldScanline+1)->totalWeight;
			zeroWeight = (newWeight == 0) || zeroWeight;
			totalWeight += newWeight;
		    }
		    newBuf->value += value;
		    if (CONSERVATIVE_AVG_DOWN && 
			(zeroWeight || newBuf->totalWeight == 0))
			newBuf->totalWeight = 0;
		    else
			newBuf->totalWeight += totalWeight;
		}
	    }

	    for (xx = 0; xx < backRLEGrid->xdim; xx++) {
		newScanline[xx].value = ushort(newScanlineDbl[xx].value/8);
		newScanline[xx].totalWeight = 
		    ushort(newScanlineDbl[xx].totalWeight/8);
	    }

	    backRLEGrid->putScanline(newScanline, yy, zz);
	}
    }


    // Coordinate systems???

    delete frontRLEGrid;
    frontRLEGrid = new OccGridRLE(newXDim, newYDim, newZDim, CHUNK_SIZE);
    frontRLEGrid->resolution = newRes;
    swapgrids();

    return TCL_OK;
}



int
Vrip_TransposeXZCmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_transpxz";
	return TCL_ERROR;
    }

    theGrid->transposeXZ();

    return TCL_OK;
}


int
Vrip_TransposeYZCmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_transpyz";
	return TCL_ERROR;
    }

    theGrid->transposeYZ();

    return TCL_OK;
}


int
Vrip_ExpandVarToConstCmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_varfromconst";
	return TCL_ERROR;
    }

    printf("Doing first expansion of varying regions...\n");
    expandVaryingFromConstant(frontRLEGrid, backRLEGrid);
    swapgrids();

    printf("Transposing X <-> Z...\n");
    frontRLEGrid->transposeXZ(backRLEGrid);
    swapgrids();

    printf("Doing second expansion of varying regions...\n");
    expandVaryingFromConstant(frontRLEGrid, backRLEGrid);
    swapgrids();
    
    printf("Transposing X <-> Z...\n");
    frontRLEGrid->transposeXZ(backRLEGrid);
    swapgrids();

    printf("Transposing X <-> Y...\n");
    frontRLEGrid->transposeXY(backRLEGrid);
    swapgrids();

    printf("Doing final expansion of varying regions...\n");
    expandVaryingFromConstant(frontRLEGrid, backRLEGrid);
    swapgrids();
    
    printf("Transposing X <-> Y...\n");
    frontRLEGrid->transposeXY(backRLEGrid);
    swapgrids();

    return TCL_OK;
}


int
Vrip_TransposeXZRLECmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_transpxz";
	return TCL_ERROR;
    }

    frontRLEGrid->transposeXZ(backRLEGrid);
    swapgrids();
    
    return TCL_OK;
}


int
Vrip_TransposeXYRLECmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_transpxy";
	return TCL_ERROR;
    }

    frontRLEGrid->transposeXY(backRLEGrid);
    swapgrids();
    
    return TCL_OK;
}


int
Vrip_TransposeYZRLECmd(ClientData, Tcl_Interp *interp, int argc, const char **)
{
    if (argc != 1) {
	interp->result = "Usage: vrip_transpyz";
	return TCL_ERROR;
    }

    frontRLEGrid->transposeYZ(backRLEGrid);
    swapgrids();

    return TCL_OK;
}


int
Vrip_ExtractCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xmin, xmax, ymin, ymax, zmin, zmax;
    int yy, zz;
    int newXdim, newYdim, newZdim, xdim, ydim;
    OccElement *slice, *scanline;

    if (argc != 7) {
	interp->result = 
	    "Usage: vrip_extract <xmin> <xmax> <ymin> <ymax> <zmin> <zmax>";
	return TCL_ERROR;
    }

    xmin = atoi(argv[1]);
    xmax = atoi(argv[2]);
    ymin = atoi(argv[3]);
    ymax = atoi(argv[4]);
    zmin = atoi(argv[5]);
    zmax = atoi(argv[6]);

    if (xmin < 0 || xmax >= frontRLEGrid->xdim ||
	ymin < 0 || ymax >= frontRLEGrid->ydim ||
	zmin < 0 || zmax >= frontRLEGrid->zdim) {
	
	interp->result = "Out of range!";
	return TCL_ERROR;
    }

    newXdim = xmax - xmin;
    newYdim = ymax - ymin;
    newZdim = zmax - zmin;

    backRLEGrid->init(newXdim, newYdim, newZdim, CHUNK_SIZE);

    for (zz = zmin; zz < zmax; zz++) {
	slice = frontRLEGrid->getSlice("z", zz, &xdim, &ydim);
	for (yy = ymin; yy < ymax; yy++) {
	    scanline = slice + yy*xdim + xmin;
	    backRLEGrid->putScanline(scanline, yy-ymin, zz-zmin);
	}
    }

    swapgrids();

    return TCL_OK;
}


int
Vrip_BlurVisBordersCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx, yy, zz, i, j, k;
    OccElement *scanline, *elem, *buf;
    ushort totalWeight;
    double value, weight, sumWeight;

    if (argc != 1) {
	interp->result = "Usage: vrip_blurvb";
	return TCL_ERROR;
    }
    

    scanline = new OccElement[frontRLEGrid->xdim];

    backRLEGrid->reset();
    for (zz = 0; zz < frontRLEGrid->zdim; zz++) {
	for (yy = 0; yy < frontRLEGrid->ydim; yy++) {
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, buf++) {
		elem = frontRLEGrid->getElement(xx,yy,zz);
		if (xx == 0 || xx == frontRLEGrid->xdim - 1 ||
		    yy == 0 || yy == frontRLEGrid->ydim - 1 ||
		    zz == 0 || zz == frontRLEGrid->zdim - 1) {

		    buf->value = elem->value;
		    buf->totalWeight = elem->totalWeight;
		}
		else {
		    totalWeight = elem->totalWeight;
		    buf->totalWeight = totalWeight;
		    value = 0;
		    sumWeight = 0;
		    for (i = -1; i <= 1; i++) {
			for (j = -1; j <= 1; j++) {
			    for (k = -1; k <= 1; k++) {
				weight = exp(-(i*i + j*j + k*k)*totalWeight/10);
				value += 
				    weight*frontRLEGrid->getElement(xx+k,yy+j,zz+i)->value;
				sumWeight += weight;
			    }
			}
		    }
		    value /= sumWeight;
		    buf->value = ushort(value);
		}		
	    }
	    backRLEGrid->putScanline(scanline, yy, zz);
	}	
    }
    
    delete [] scanline;

    swapgrids();

    return TCL_OK;
}

