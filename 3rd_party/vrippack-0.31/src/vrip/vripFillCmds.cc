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
#include "vripFillCmds.h"
#include "vripGlobals.h"
#include "Mesh.h"


int
Vrip_EllipseCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float xlen,ylen,zlen;
    float a,b,c;
    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 4) {
	interp->result = "Usage: vrip_ellipse <xlen> <ylen> <zlen>";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    xlen = atof(argv[1]);
    ylen = atof(argv[2]);
    zlen = atof(argv[3]);

    a = 1/xlen/xlen;
    b = 1/ylen/ylen;
    c = 1/zlen/zlen;

    res = theGrid->resolution;

    buf = theGrid->elems;

    zpos = -theGrid->zdim*theGrid->resolution/2;
    for (zz = 0; zz < theGrid->zdim; zz++, zpos+=res) {
	ypos = -theGrid->ydim*theGrid->resolution/2;	
	for (yy = 0; yy < theGrid->ydim; yy++, ypos+=res) {
	    xpos = -theGrid->xdim*theGrid->resolution/2;
	    for (xx = 0; xx < theGrid->xdim; xx++, xpos+=res, buf++) {
		fxyz = a*xpos*xpos + b*ypos*ypos + c*zpos*zpos - 1;
		if (fxyz > 0) {
		    buf->value = 0;
		    buf->totalWeight = 0;
		} else {
		    buf->value = USHRT_MAX/2;
		    buf->totalWeight = USHRT_MAX/2;
		}
	    }
	}
    }

    return TCL_OK;
}


int
Vrip_EllipseRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float xlen,ylen,zlen;
    float a,b,c;
    float xpos,ypos,zpos;
    float xposBump,yposBump,zposBump;
    float res, fxyz;
    OccElement *buf;
    float amplitude;
    float freq;
    float phase;

    amplitude = 0.517;
    freq = 0.3;
    phase = 0.123;

    if (argc != 4) {
	interp->result = "Usage: vrip_ellipse <xlen> <ylen> <zlen>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    xlen = atof(argv[1]);
    ylen = atof(argv[2]);
    zlen = atof(argv[3]);

    a = 5/xlen/xlen;
    b = 5/ylen/ylen;
    c = 5/zlen/zlen;

    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
	       float amp = amplitude*exp(-float(yy)/frontRLEGrid->ydim*3);
	       xposBump = xpos + 
		  amp*frontRLEGrid->resolution*sin(2*M_PI*freq*xx+0.123);
	       yposBump = ypos + 
		  amp*frontRLEGrid->resolution*sin(2*M_PI*freq*yy+6.788);
	       zposBump = zpos + 
		  amp*frontRLEGrid->resolution*sin(2*M_PI*freq*zz+2.97);
	       fxyz = a*xposBump*xposBump + b*yposBump*yposBump 
		  + c*zposBump*zposBump - 1;
	       fxyz = 2*exp(-(a*xposBump*xposBump + 
			      b*yposBump*yposBump + c*zposBump*zposBump));
	       if (fxyz > 0.99) {
		  buf->value = USHRT_MAX;
		  buf->totalWeight = 0;
	       }
	       else if (fxyz < 0.01) {
		  buf->value = 0;
		  buf->totalWeight = 0;
	       } else {
		  buf->value = ushort(USHRT_MAX*fxyz);
		  buf->totalWeight = USHRT_MAX;
	       }
	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}


int
Vrip_ChairRLECmd(ClientData, Tcl_Interp *interp, 
		 int argc, const char *argv[])
{
    int xx,yy,zz;
    float a,b,k;
    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 4) {
	interp->result = "Usage: vrip_chair <a> <b> <k>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    a = atof(argv[1]);
    b = atof(argv[2]);
    k = atof(argv[3]);

    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
	       fxyz = -(pow((xpos*xpos+ypos*ypos+zpos*zpos-a*k*k),2) - 
		  b*((zpos-k)*(zpos-k)-2*xpos*xpos)*
		  ((zpos+k)*(zpos+k)-2*ypos*ypos))*0.01+0.5;
	       if (fxyz > 1) {
		  buf->value = USHRT_MAX;
		  buf->totalWeight = 0;
	       }
	       else if (fxyz < 0) {
		  buf->value = 0;
		  buf->totalWeight = 0;
	       } else {
		  buf->value = ushort(USHRT_MAX*fxyz);
		  buf->totalWeight = USHRT_MAX;
	       }
	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}


int
Vrip_SinewaveRLECmd(ClientData, Tcl_Interp *interp, 
		    int argc, const char *argv[])
{
    int xx,yy,zz;
    float frequency,amplitude;
    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 3) {
	interp->result = "Usage: vrip_sinewave <amplitude> <frequency>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    amplitude = atof(argv[1]);
    frequency = atof(argv[2]);

    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
	       fxyz = -zpos + amplitude*cos(2*M_PI*frequency*xpos) + 0.5;
	       if (fxyz > 1) {
		  buf->value = USHRT_MAX;
		  buf->totalWeight = 0;
	       }
	       else if (fxyz < 0) {
		  buf->value = 0;
		  buf->totalWeight = 0;
	       } else {
		  buf->value = ushort(USHRT_MAX*fxyz);
		  buf->totalWeight = USHRT_MAX;
	       }
	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}


int
Vrip_GumdropTorusRLECmd(ClientData, Tcl_Interp *interp, 
			int argc, const char *argv[])
{
    int xx,yy,zz;
    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 1) {
	interp->result = "Usage: vrip_gumdrop_torus";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
	       fxyz = 4*pow(xpos,4)+4*pow(ypos,4)+8*ypos*ypos*zpos*zpos
		  +4*pow(zpos,4)+17*xpos*xpos*ypos*ypos+17*xpos*xpos*zpos*zpos
		  -20*xpos*xpos-20*ypos*ypos-20*zpos*zpos+17+0.5;
	       fxyz = (1-fxyz)*0.1;
	       if (fxyz > 1) {
		  buf->value = USHRT_MAX;
		  buf->totalWeight = 0;
	       }
	       else if (fxyz < 0) {
		  buf->value = 0;
		  buf->totalWeight = 0;
	       } else {
		  buf->value = ushort(USHRT_MAX*fxyz);
		  buf->totalWeight = USHRT_MAX;
	       }
	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}


int
Vrip_CylinderRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float radius, c;
    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 2) {
	interp->result = "Usage: vrip_cylinder <radius>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    radius = atof(argv[1]);
    c = 1/radius/radius;

    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
		fxyz = exp(-(c*(xpos*xpos+zpos*zpos)));
		if (fxyz > 0.75 || fxyz < 0.25) {
		    buf->value = 0;
		    buf->totalWeight = 0;
		} else {
		    buf->value = ushort(USHRT_MAX*fxyz);
		    buf->totalWeight = USHRT_MAX;
		}
	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}


int
Vrip_FillRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float value, weight;

    if (argc != 3) {
	interp->result = "Usage: vrip_fillrle <value> <weight>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    value = atof(argv[1]);
    weight = atof(argv[2]);

    OccElement *scanline = new OccElement[frontRLEGrid->xdim]; 
    for (xx = 0; xx < frontRLEGrid->xdim; xx++) {
	scanline[xx].value = ushort(USHRT_MAX*value);
	scanline[xx].totalWeight = ushort(UCHAR_MAX*weight);
    }

/*
    scanline[0].value = scanline[0].totalWeight = 0;
    scanline[frontRLEGrid->xdim-1].value = 
	scanline[frontRLEGrid->xdim-1].totalWeight = 0;
*/

    frontRLEGrid->reset();
    
    for (zz = 0; zz < frontRLEGrid->zdim; zz++) {
	for (yy = 0; yy < frontRLEGrid->ydim; yy++) {
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }


/*

    for (xx = 0; xx < frontRLEGrid->xdim; xx++) {
	scanline[xx].value = 0;
	scanline[xx].totalWeight = 0;
    }

    zz = 0;
    for (yy = 0; yy < frontRLEGrid->ydim; yy++) {
	    frontRLEGrid->putScanline(scanline, yy, zz);
    }
    zz = frontRLEGrid->zdim-1;
    for (yy = 0; yy < frontRLEGrid->ydim; yy++) {
	    frontRLEGrid->putScanline(scanline, yy, zz);
    }

    yy = 0;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++) {
	    frontRLEGrid->putScanline(scanline, yy, zz);
    }
    yy = frontRLEGrid->ydim-1;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++) {
	    frontRLEGrid->putScanline(scanline, yy, zz);
    }
*/


    delete [] scanline;

    return TCL_OK;
}


int
Vrip_CubeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float xlen,ylen,zlen;
    float xpos,ypos,zpos;
    float res;
    OccElement *buf;

    if (argc != 4) {
	interp->result = "Usage: vrip_cube <xlen> <ylen> <zlen>";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    xlen = atof(argv[1]);
    ylen = atof(argv[2]);
    zlen = atof(argv[3]);

    res = theGrid->resolution;

    buf = theGrid->elems;

    zpos = -theGrid->zdim*theGrid->resolution/2;

    for (zz = 0; zz < theGrid->zdim; zz++, zpos+=res) {

	ypos = -theGrid->ydim*theGrid->resolution/2;
	
	for (yy = 0; yy < theGrid->ydim; yy++, ypos+=res) {

	    xpos = -theGrid->xdim*theGrid->resolution/2;

	    for (xx = 0; xx < theGrid->xdim; xx++, xpos+=res, buf++) {

		if (fabs(xpos) < xlen/2 && fabs(ypos) < ylen/2 
		    && fabs(zpos) < zlen/2) {
		    buf->value = USHRT_MAX;
		    buf->totalWeight = USHRT_MAX;		    
		} else {
		    buf->value = 0;
		    buf->totalWeight = USHRT_MAX;
		}

	    }
	}
    }

    return TCL_OK;
}


int
Vrip_CubeRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int xx,yy,zz;
    float xlen,ylen,zlen;

    float xpos,ypos,zpos;
    float res, fxyz;
    OccElement *buf;

    if (argc != 4) {
	interp->result = "Usage: vrip_cuberle <xlen> <ylen> <zlen>";
	return TCL_ERROR;
    }

    if (frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    xlen = atof(argv[1]);
    ylen = atof(argv[2]);
    zlen = atof(argv[3]);

    res = frontRLEGrid->resolution;

    OccElement *scanline = new OccElement[frontRLEGrid->xdim];
    frontRLEGrid->reset();

    zpos = -frontRLEGrid->zdim*frontRLEGrid->resolution/2;
    for (zz = 0; zz < frontRLEGrid->zdim; zz++, zpos+=res) {
	ypos = -frontRLEGrid->ydim*frontRLEGrid->resolution/2;	
	for (yy = 0; yy < frontRLEGrid->ydim; yy++, ypos+=res) {
	    xpos = -frontRLEGrid->xdim*frontRLEGrid->resolution/2;
	    buf = scanline;
	    for (xx = 0; xx < frontRLEGrid->xdim; xx++, xpos+=res, buf++) {
		if (fabs(xpos) < xlen/2 && fabs(ypos) < ylen/2 
		    && fabs(zpos) < zlen/2) {
		    buf->value = USHRT_MAX;
		    buf->totalWeight = USHRT_MAX;		    
		} else {
		    buf->value = 0;
		    buf->totalWeight = USHRT_MAX;
		}

	    }
	    frontRLEGrid->putScanline(scanline, yy, zz);
	}
    }

    delete [] scanline;

    return TCL_OK;
}
