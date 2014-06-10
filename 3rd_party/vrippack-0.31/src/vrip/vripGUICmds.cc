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

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif


#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#include "vrip.h"
#include "vripGUICmds.h"
#include "vripGlobals.h"

#include <fstream>

// Use c++-ified version!!!
#include <tk.h>


static uchar *sliceBuf = NULL;
static int sliceXdim = 0, sliceYdim = 0;

static char *theRenderPhoto = NULL;
static uchar *renderBuf = NULL;
static int renderXdim = 0, renderYdim = 0;

static int thePhotoSliceArgc;
static const char **thePhotoSliceArgv = NULL;
static Tcl_Interp *theInterp;

// Allocate fog, bog only once, to avoid memory leakage
// This has to be global, so that the multiple functions can all
// use the same front/back buffer.  Otherwise, changes from one
// filter will not be reflected in the others...  -lucas
static OccGrid *fog=NULL;
static OccGrid *bog=NULL;

int
Vrip_PhotoSliceCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{    
    const char *axis;
    OccElement *slice;
    int sliceNum, xdim, ydim;

    if (argc != 4) {
	interp->result = "Usage: vrip_photoslice <photo-widget> <slice-num> <axis>";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    Tk_PhotoHandle handle = Tk_FindPhoto(theInterp, argv[1]);
    if (handle == NULL)
	return TCL_ERROR;

    sliceNum = atoi(argv[2]);
    axis = argv[3];

    slice = theGrid->getSlice(axis, sliceNum, &xdim, &ydim);

    Tk_PhotoSetSize(handle, xdim, ydim);

    Tk_PhotoImageBlock block;
//    block.ptr = (uchar *)(slice + xdim*(ydim-1));
    block.pixelPtr = (uchar *)(slice);
    block.width = xdim;
    block.height = ydim;

    block.pitch = -xdim*4;
    block.pixelSize = 4;

#ifdef LINUX
    block.offset[0] = 1;
    block.offset[1] = 1;
    block.offset[2] = 1;
#else
    block.offset[0] = 0;
    block.offset[1] = 0;
    block.offset[2] = 0;
#endif

    Tk_PhotoPutBlock(handle, &block, 0, 0, xdim, ydim, TK_PHOTO_COMPOSITE_SET);

    return TCL_OK;

}


int
Vrip_SetPhotoSliceCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    theInterp = interp;
    thePhotoSliceArgc = argc;

    thePhotoSliceArgv = argv;

    /*
    if (thePhotoSliceArgv == NULL) {
	thePhotoSliceArgv = new char*[10];
	for (int i = 0; i < 10; i++) {
	    thePhotoSliceArgv[i] = new char[PATH_MAX];
	}
    }

    for (int i = 0; i < argc; i++) {
	strcpy(thePhotoSliceArgv[i], argv[i]);
    }
    */

    return TCL_OK;
}


void
updatePhotoSlice()
{
    Vrip_PhotoSliceRLECmd(NULL, theInterp, thePhotoSliceArgc, 
			 thePhotoSliceArgv);
}

int
Vrip_VolSizeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
  sprintf(interp->result,"%d %d %d",frontRLEGrid->xdim,frontRLEGrid->ydim,frontRLEGrid->zdim);
  return TCL_OK;
}


int
Vrip_PhotoSliceRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{    
    const char *axis;
    OccElement *slice;
    int sliceNum, xdim, ydim;
    float weight, weightScale, value;
    int offset, divisor;

    if (argc != 7) {
	interp->result = "Usage: vrip_photoslicerle <photo-widget> <slice-num> <axis> <weight-scale> <offset> <divisor>";
	return TCL_ERROR;
    }    

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    Tk_PhotoHandle handle = Tk_FindPhoto(theInterp, argv[1]);
    if (handle == NULL)
	return TCL_ERROR;

    sliceNum = atoi(argv[2]);
    axis = argv[3];

    weightScale = atof(argv[4]);
    if (weightScale <= 0) {
	interp->result = "weight scale out of range";
	return TCL_ERROR;
    }

    offset = atoi(argv[5]);
    divisor = atoi(argv[6]);

    slice = frontRLEGrid->getSlice(axis, sliceNum, &xdim, &ydim);
    
    if (sliceBuf == NULL) {
	sliceBuf = new uchar[3*xdim*ydim];
	sliceXdim = xdim;
	sliceYdim = ydim;
    } 
    else if (xdim*ydim > sliceXdim*sliceYdim) {
	delete sliceBuf;
	sliceBuf = new uchar[3*xdim*ydim];
	sliceXdim = xdim;
	sliceYdim = ydim;
    }

    for (int i = 0; i < xdim*ydim; i++) {
	if (ShowConfSlice) {
	    value = slice[i].value/255.0;
	    if (value >= 255.5) value = 255;
	    sliceBuf[3*i] = uchar(value+0.5);
	    sliceBuf[3*i+1] = uchar(value+0.5);
	    weight = (slice[i].totalWeight & ~OccGridRLE::FALSE_DATA_BIT)
		/weightScale;
	    if (weight > 255.5) weight = 255;
	    sliceBuf[3*i+2] = uchar(weight+0.5);
#if 0
	    value = slice[i].value/255.0;
	    if (value >= 255.5) value = 255;
	    sliceBuf[3*i] = uchar(value+0.5);
	    sliceBuf[3*i+1] = uchar(value+0.5);
	    sliceBuf[3*i+2] = uchar(value+0.5);
#elif 0
	    weight = 8*(slice[i].totalWeight & ~OccGridRLE::FALSE_DATA_BIT)
		/weightScale;
	    if (weight > 255.5) weight = 255;
	    sliceBuf[3*i] = uchar(weight+0.5);
	    sliceBuf[3*i+1] = uchar(weight+0.5);
	    sliceBuf[3*i+2] = 255;
#endif
	}
	else if (ShowValueWeight) {
	    if (slice[i].value == USHRT_MAX && 
		(slice[i].totalWeight & ~OccGridRLE::FALSE_DATA_BIT) == 0) {
#if 1
		sliceBuf[3*i] = 145;
		sliceBuf[3*i+1] = 117;
		sliceBuf[3*i+2] = 74;
#else
		sliceBuf[3*i] = 0;
		sliceBuf[3*i+1] = 0;
		sliceBuf[3*i+2] = 0;
#endif
	    } else {
		weight = (slice[i].totalWeight & ~OccGridRLE::FALSE_DATA_BIT)
		    /weightScale;
		value = slice[i].value/256 - 128;
		value = value*weight/256 + 128;
		if (value >= 255.5)
		   value = 255;
		else if (value < -0.5)
		   value = 0;
		sliceBuf[3*i] = sliceBuf[3*i+1] = sliceBuf[3*i+2] = uchar(value+0.5);
		//sliceBuf[3*i] = sliceBuf[3*i+1] = sliceBuf[3*i+2] = uchar(weight/1.5+0.5);
	    }
	} else {
	    if (slice[i].value == USHRT_MAX && 
		(slice[i].totalWeight & ~OccGridRLE::FALSE_DATA_BIT) == 0) {
#if 1
		sliceBuf[3*i] = 145;
		sliceBuf[3*i+1] = 117;
		sliceBuf[3*i+2] = 74;
#else
		sliceBuf[3*i] = 0;
		sliceBuf[3*i+1] = 0;
		sliceBuf[3*i+2] = 0;
#endif
	    } else {

	       value = (float(slice[i].value)-offset)/divisor+128;
	       if (value <= 0.5) value = 0;

	       //value = slice[i].value/255.0;
		if (value >= 255.5) value = 255;
		sliceBuf[3*i] = sliceBuf[3*i+1] = sliceBuf[3*i+2] = uchar(value+0.5);
		//sliceBuf[3*i] = sliceBuf[3*i+1] = sliceBuf[3*i+2] = uchar(weight/1.5+0.5);
	    }
	}
    }

    Tk_PhotoSetSize(handle, xdim, ydim);

    Tk_PhotoImageBlock block;
    
    if (EQSTR(axis, "x")) {
	block.pixelPtr = sliceBuf + 3*xdim*(ydim-1);
	block.pitch = -xdim*3;
    } else if (EQSTR(axis, "y")) {
	block.pixelPtr = sliceBuf;
	block.pitch = xdim*3;
    } else if (EQSTR(axis, "z")) {
	block.pixelPtr = sliceBuf + 3*xdim*(ydim-1);
	block.pitch = -xdim*3;
    }

    block.pixelSize = 3;
    block.offset[0] = 0;
    block.offset[1] = 1;
    block.offset[2] = 2;
    block.width = xdim;
    block.height = ydim;

    Tk_PhotoPutBlock(handle, &block, 0, 0, xdim, ydim, TK_PHOTO_COMPOSITE_SET);

    return TCL_OK;

}


int
Vrip_PhotoSliceNormRLECmd(ClientData, Tcl_Interp *interp, int argc, 
			 const char *argv[])
{    
    const char *axis;
    OccNormElement *slice;
    int sliceNum, xdim, ydim;
    float weight, weightScale, value;

    if (argc != 5) {
	interp->result = "Usage: vrip_photoslicenormrle <photo-widget> <slice-num> <axis> <weight-scale>";
	return TCL_ERROR;
    }    

    if (frontRLEGridNorm == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    Tk_PhotoHandle handle = Tk_FindPhoto(theInterp, argv[1]);
    if (handle == NULL)
	return TCL_ERROR;

    sliceNum = atoi(argv[2]);
    axis = argv[3];

    weightScale = atof(argv[4]);
    if (weightScale <= 0) {
	interp->result = "weight scale out of range";
	return TCL_ERROR;
    }

    slice = frontRLEGridNorm->getSlice(axis, sliceNum, &xdim, &ydim);
    
    if (sliceBuf == NULL) {
	sliceBuf = new uchar[3*xdim*ydim];
	sliceXdim = xdim;
	sliceYdim = ydim;
    } 
    else if (xdim*ydim > sliceXdim*sliceYdim) {
	delete sliceBuf;
	sliceBuf = new uchar[3*xdim*ydim];
	sliceXdim = xdim;
	sliceYdim = ydim;
    }

    if (!ShowNormals) {
	for (int i = 0; i < xdim*ydim; i++) {
	    value = slice[i].value/255.0;
	    if (value >= 255.5) value = 255;
	    sliceBuf[3*i] = uchar(value+0.5);
	    weight = slice[i].totalWeight/weightScale;
	    if (weight > 255.5) weight = 255;
	    sliceBuf[3*i+1] = uchar(weight+0.5);
	}
    } else {
	for (int i = 0; i < xdim*ydim; i++) {
	    sliceBuf[3*i] = uchar(int(slice[i].nx)+127);
	    sliceBuf[3*i+1] = uchar(int(slice[i].ny)+127);
	    sliceBuf[3*i+2] = uchar(int(slice[i].nz)+127);
	}
    }

    Tk_PhotoSetSize(handle, xdim, ydim);

    Tk_PhotoImageBlock block;
    
    if (EQSTR(axis, "x")) {
	block.pixelPtr = sliceBuf + 3*xdim*(ydim-1);
	block.pitch = -xdim*3;
    } else if (EQSTR(axis, "y")) {
	block.pixelPtr = sliceBuf;
	block.pitch = xdim*3;
    } else if (EQSTR(axis, "z")) {
	block.pixelPtr = sliceBuf + 3*xdim*(ydim-1);
	block.pitch = -xdim*3;
    }

    block.pixelSize = 3;

    if (!ShowNormals) {
	block.offset[0] = 0;
	block.offset[1] = 0;
	block.offset[2] = 1;
    } else {
	block.offset[0] = 0;
	block.offset[1] = 1;
	block.offset[2] = 2;
    }

    block.width = xdim;
    block.height = ydim;

    Tk_PhotoPutBlock(handle, &block, 0, 0, xdim, ydim, TK_PHOTO_COMPOSITE_SET);

    return TCL_OK;

}



int
Vrip_GetVoxelRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]) 
{
    int ix, iy, iz;
    char buf[512];

    if (argc != 4) {
        interp->result = "Usage: vrip_getvoxelrle <ix> <iy> <iz>";
        return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }

    ix = atoi(argv[1]);
    iy = atoi(argv[2]);
    iz = atoi(argv[3]);

    if (ix >= 0 && ix < frontRLEGrid->xdim &&
	iy >= 0 && iy < frontRLEGrid->ydim &&
	iz >= 0 && iz < frontRLEGrid->zdim) {
       OccElement *el = frontRLEGrid->getElement(ix, iy, iz);
       sprintf(buf, "(%.3d, %.3d, %.3d): v = %+.5d, wt = %.5d",
	       ix, iy, iz, (int) el->value + SHRT_MIN, el->totalWeight);
       }
    else
       sprintf(buf, "<index out of range>");

    Tcl_SetResult(interp, buf, TCL_VOLATILE);

    return TCL_OK;
}



int
Vrip_SetRenderPhotoCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (theRenderPhoto != NULL)
	delete [] theRenderPhoto;

    if (argc == 1) {
	theRenderPhoto = NULL;
	return TCL_OK;
    }
    else {
	theRenderPhoto = new char[strlen(argv[1])+1];
	strcpy(theRenderPhoto, argv[1]);
	return TCL_OK;
    }
}



int
updateRenderPhoto(DepthMap *dm)
{
    int xdim, ydim;

    if (theRenderPhoto == NULL)
	return 0;

    Tk_PhotoHandle handle = Tk_FindPhoto(theInterp, theRenderPhoto);
    if (handle == NULL)
	return 0;

    xdim = dm->xdim;
    ydim = dm->ydim;

    if (renderBuf == NULL) {
	renderBuf = new uchar[xdim*ydim];
	renderXdim = xdim;
	renderYdim = ydim;
    } 
    else if (xdim*ydim > renderXdim*renderYdim) {
	delete renderBuf;
	renderBuf = new uchar[xdim*ydim];
	renderXdim = xdim;
	renderYdim = ydim;
    }

    for (int i = 0; i < xdim*ydim; i++) {
	renderBuf[i] = uchar(fabs(dm->elems[i].conf*255+0.5));
    }

    Tk_PhotoSetSize(handle, xdim, ydim);

    Tk_PhotoImageBlock block;
    
    block.pixelPtr = renderBuf + xdim*(ydim-1);
    block.pitch = -xdim;
    block.pixelSize = 1;
    block.offset[0] = 0;
    block.offset[1] = 0;
    block.offset[2] = 0;
    block.width = xdim;
    block.height = ydim;

    Tk_PhotoPutBlock(handle, &block, 0, 0, xdim, ydim, TK_PHOTO_COMPOSITE_SET);

    return 1;

}


