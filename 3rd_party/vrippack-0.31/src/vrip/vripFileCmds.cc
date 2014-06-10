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
#include "vripFileCmds.h"
#include "vripGlobals.h"
#include "vripAux.h"


int
Vrip_WriteGridCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3) {
	interp->result = "Usage: vrip_writegrid <filename>";
	return TCL_ERROR;
    }

    // The third argument is supposed to be something that flags us
    //  not to transpose the grid

    if (argc == 2) {	
	// Unflip the grid
	if (frontRLEGrid->flip) {
	    frontRLEGrid->flip = FALSE;
	    frontRLEGrid->origin[2] = -frontRLEGrid->origin[2];
	}

	// Restore to original untransposed coords
	if (frontRLEGrid->axis != Z_AXIS) {
	   if (frontRLEGrid->axis == X_AXIS) {
	      frontRLEGrid->transposeXZ(backRLEGrid);
	      swapgrids();
	   }
	   else {
	      frontRLEGrid->transposeYZ(backRLEGrid);
	      swapgrids();
	   }

	   frontRLEGrid->axis = Z_AXIS;
	}
    }

    if (!frontRLEGrid->write(argv[1])) {
	interp->result = "vrip_writegrid: could not write file";
	return TCL_ERROR;
    }

    return TCL_OK;
}


int
Vrip_WriteGridNormCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2) {
	interp->result = "Usage: vrip_writegridnorm <filename>";
	return TCL_ERROR;
    }


    // Unflip the grid
    if (frontRLEGridNorm->flip) {
	frontRLEGridNorm->flip = FALSE;
	frontRLEGridNorm->origin[2] = -frontRLEGridNorm->origin[2];
    }

    // Restore to original untransposed coords
    if (frontRLEGridNorm->axis != Z_AXIS) {
       if (frontRLEGridNorm->axis == X_AXIS) {
	  frontRLEGridNorm->transposeXZ(backRLEGridNorm);
	  swapgrids();
       }
       else {
	  frontRLEGridNorm->transposeYZ(backRLEGridNorm);
	  swapgrids();
       }

       frontRLEGridNorm->axis = Z_AXIS;
    }


    if (!frontRLEGridNorm->write(argv[1])) {
	interp->result = "vrip_writegridnorm: could not write file";
	return TCL_ERROR;
    }

    return TCL_OK;
}


int
Vrip_WriteDenCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2) {
	interp->result = "Usage: vrip_writeden <filename>";
	return TCL_ERROR;
    }
    
    theGrid->writeDen(argv[1]);

    return TCL_OK;
}


int
Vrip_ReadGridCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int maxDim;

    if (argc != 2) {
	interp->result = "Usage: vrip_readgrid <filename>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	frontRLEGrid = new OccGridRLE(50,50,50,CHUNK_SIZE);
	backRLEGrid = new OccGridRLE(50,50,50,CHUNK_SIZE);
    }
    
    if (!frontRLEGrid->read(argv[1])) {
	interp->result = "vrip_readgrid: could not read file";
	return TCL_ERROR;
    }

    // Don't delete - re-use!!

    delete backRLEGrid;


    backRLEGrid = new OccGridRLE(frontRLEGrid->xdim, frontRLEGrid->ydim,
			       frontRLEGrid->zdim, CHUNK_SIZE);
    backRLEGrid->resolution = frontRLEGrid->resolution;
    backRLEGrid->origin[0] = frontRLEGrid->origin[0];
    backRLEGrid->origin[1] = frontRLEGrid->origin[1];
    backRLEGrid->origin[2] = frontRLEGrid->origin[2];

    if (theDepthMap != NULL) {
	delete theDepthMap;
    }

    maxDim = MAX(frontRLEGrid->xdim, MAX(frontRLEGrid->ydim, 
					 frontRLEGrid->zdim));
    theDepthMap = new DepthMap(int(maxDim*2.2), 
			       int(maxDim*2.2), 
			       FALSE, DEPTH_TREE_GRANULARITY);

    return TCL_OK;
}



int
Vrip_ReadFlatGridCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int maxDim;
    int xdim, ydim, zdim;

    if (argc != 2) {
	interp->result = "Usage: vrip_readflatgrid <filename>";
	return TCL_ERROR;
    }

    if (frontRLEGrid != NULL) {
	delete frontRLEGrid;
    }

    if (backRLEGrid != NULL) {
	delete backRLEGrid;
    }

    FILE *fp = fopen(argv[1], "rb");
    fread(&xdim, sizeof(int), 1, fp);
    fread(&ydim, sizeof(int), 1, fp);
    fread(&zdim, sizeof(int), 1, fp);

    frontRLEGrid = new OccGridRLE(xdim, ydim, zdim, CHUNK_SIZE);

    uchar *ucScanline = new uchar[xdim];
    OccElement *occScanline = new OccElement[xdim];

    for (int zz = 0; zz < zdim; zz++) {
       for (int yy = 0; yy < ydim; yy++) {
	  fread(ucScanline, sizeof(uchar), xdim, fp);
	  for (int xx = 0; xx < xdim; xx++) {
	     occScanline[xx].value = (ushort)(ucScanline[xx]/255.0*USHRT_MAX);
	     occScanline[xx].totalWeight = 1;
	  }
	  frontRLEGrid->putScanline(occScanline, yy, zz);
       }
    }

    delete ucScanline;
    delete occScanline;

    backRLEGrid = new OccGridRLE(frontRLEGrid->xdim, frontRLEGrid->ydim,
			       frontRLEGrid->zdim, CHUNK_SIZE);
    backRLEGrid->resolution = frontRLEGrid->resolution;
    backRLEGrid->origin[0] = frontRLEGrid->origin[0];
    backRLEGrid->origin[1] = frontRLEGrid->origin[1];
    backRLEGrid->origin[2] = frontRLEGrid->origin[2];

    if (theDepthMap != NULL) {
	delete theDepthMap;
    }

    maxDim = MAX(frontRLEGrid->xdim, MAX(frontRLEGrid->ydim, 
					 frontRLEGrid->zdim));
    theDepthMap = new DepthMap(int(maxDim*2.2), 
			       int(maxDim*2.2), 
			       FALSE, DEPTH_TREE_GRANULARITY);

    return TCL_OK;
}


int
Vrip_ReadRawImagesCmd(ClientData, Tcl_Interp *interp, int argc, 
		      const char *argv[])
{
    int maxDim;
    int xdim, ydim, zdim;
    char filename[PATH_MAX], other[PATH_MAX];

    if (argc != 13) {
	interp->result = "Usage: vrip_readrawimages <root-name> <ext> <sig-digits> <image-xdim> <image-ydim> <xmin> <xmax> <ymin> <ymax> <zmin> <zmax> <downsample-rate>";
	return TCL_ERROR;
    }

    // Need to give option to downsample
    // Need to provide min/max thresholding into empty and unknown

    int sigDigits = atoi(argv[3]);
    int startIndex = atoi(argv[4]);
    int oxdim = atoi(argv[4]);
    int oydim = atoi(argv[5]);
    int xmin = atoi(argv[6]);
    int xmax = atoi(argv[7]);
    int ymin = atoi(argv[8]);
    int ymax = atoi(argv[9]);
    int zmin = atoi(argv[10]);
    int zmax = atoi(argv[11]);
    int downsampleRate = atoi(argv[12]);

    short *volStack = new short[oxdim*oydim*downsampleRate];

    xdim = (xmax-xmin)/downsampleRate;
    ydim = (ymax-ymin)/downsampleRate;
    zdim = (zmax-zmin)/downsampleRate;

    if (frontRLEGrid != NULL) {
	delete frontRLEGrid;
    }

    if (backRLEGrid != NULL) {
	delete backRLEGrid;
    }


    frontRLEGrid = new OccGridRLE(xdim, ydim, zdim, CHUNK_SIZE);

    ushort *usScanline = new ushort[xdim];
    OccElement *occScanline = new OccElement[xdim];


    for (int zz = 0; zz < zdim; zz++) {

       // Read slices
       for (int oz = 0; oz < downsampleRate; oz++) {
	  sprintf(filename, "%d", zz*downsampleRate+zmin+oz);
	  strcpy(other, filename);
	  int numDigits = strlen(filename);
	  for (int i=0; i < sigDigits-numDigits; i++) {
	     sprintf(filename, "0%s", other);
	     strcpy(other, filename);
	  }
	  sprintf(filename, "%s%s.%s", argv[1], other, argv[2]);
	  
	  FILE *fp = fopen(filename, "rb");
	  fread(volStack+oxdim*oydim*oz, sizeof(ushort), oxdim*oydim, fp);
	  fclose(fp);
       }

       printf("\rSlice %d of %d...", zz, zdim);
       fflush(stdout);
       for (int yy = 0; yy < ydim; yy++) {
	  for (int xx = 0; xx < xdim; xx++) {
	     uint total = 0;
	     for (int oz = 0; oz < downsampleRate; oz++) {
		for (int oy = yy*downsampleRate+ymin; 
		     oy < (yy+1)*downsampleRate+ymin; oy++) {
		   for (int ox = xx*downsampleRate+xmin; 
			ox < (xx+1)*downsampleRate+xmin; ox++) {
		      int value = volStack[oz*oxdim*oydim+oy*oxdim+ox];
		      value -= SHRT_MIN;
		      total += uint(value);
		   }
		}
	     }
	     occScanline[xx].value = ushort(total/(downsampleRate*downsampleRate*downsampleRate));
	     occScanline[xx].totalWeight = 1;
	  }
	  frontRLEGrid->putScanline(occScanline, yy, zz);
       }
    }

    printf("\n");

    delete volStack;
    delete usScanline;
    delete occScanline;

    backRLEGrid = new OccGridRLE(frontRLEGrid->xdim, frontRLEGrid->ydim,
			       frontRLEGrid->zdim, CHUNK_SIZE);
    backRLEGrid->resolution = frontRLEGrid->resolution;
    backRLEGrid->origin[0] = frontRLEGrid->origin[0];
    backRLEGrid->origin[1] = frontRLEGrid->origin[1];
    backRLEGrid->origin[2] = frontRLEGrid->origin[2];

    if (theDepthMap != NULL) {
	delete theDepthMap;
    }

    maxDim = MAX(frontRLEGrid->xdim, MAX(frontRLEGrid->ydim, 
					 frontRLEGrid->zdim));
    theDepthMap = new DepthMap(int(maxDim*2.2), 
			       int(maxDim*2.2), 
			       FALSE, DEPTH_TREE_GRANULARITY);

    return TCL_OK;
}


int
Vrip_WriteFlatGridCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xdim, ydim, zdim, xx, yy, zz;
    OccElement *occScanline;

    /*
    frontRLEGrid = new OccGridRLE(11,11,11,CHUNK_SIZE);

    xdim = frontRLEGrid->xdim;
    ydim = frontRLEGrid->ydim;
    zdim = frontRLEGrid->zdim;

    occScanline = new OccElement[xdim];

    for (zz = 0; zz < zdim; zz++) {
       for (yy = 0; yy < ydim; yy++) {
	  for (xx = 0; xx < xdim; xx++) {
	     if (((xx-xdim/2)*(xx-xdim/2)+(yy-ydim/2)*(yy-ydim/2)+(zz-zdim/2)*(zz-zdim/2)) > (xdim/4 * xdim/4)) {
		occScanline[xx].value = 0;
		occScanline[xx].totalWeight = 1;
	     } else {
		occScanline[xx].value = 128<<8;
		occScanline[xx].totalWeight = 1;
	     }
	  }
	  frontRLEGrid->putScanline(occScanline, yy, zz);
       }
    }
    delete occScanline;
    */

    if (argc != 2) {
	interp->result = "Usage: vrip_writeflatgrid <filename>";
	return TCL_ERROR;
    }

    xdim = frontRLEGrid->xdim;
    ydim = frontRLEGrid->ydim;
    zdim = frontRLEGrid->zdim;

    FILE *fp = fopen(argv[1], "wb");
    fwrite(&xdim, sizeof(int), 1, fp);
    fwrite(&ydim, sizeof(int), 1, fp);
    fwrite(&zdim, sizeof(int), 1, fp);

    uchar *ucScanline = new uchar[xdim];

    for (zz = 0; zz < zdim; zz++) {
       for (yy = 0; yy < ydim; yy++) {
	  occScanline = frontRLEGrid->getScanline(yy, zz);
	  for (xx = 0; xx < xdim; xx++) {
	     ucScanline[xx] = uchar(occScanline[xx].value >> 8);
	  }
	  fwrite(ucScanline, sizeof(uchar), xdim, fp);
       }
    }

    delete ucScanline;

    return TCL_OK;
}

