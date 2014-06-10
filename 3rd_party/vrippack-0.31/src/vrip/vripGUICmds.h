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


#include <tcl.h>
#include <DepthMap.h>

int Vrip_PhotoSliceCmd(ClientData, Tcl_Interp *interp, 
		      int argc, const char *argv[]);
int Vrip_PhotoSliceRLECmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[]);
int Vrip_PhotoSliceNormRLECmd(ClientData, Tcl_Interp *interp, 
			     int argc, const char *argv[]);
int Vrip_SetRenderPhotoCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_SetPhotoSliceCmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[]);

int Vrip_GetVoxelRLECmd(ClientData, Tcl_Interp *interp,
			int argc, const char *argv[]);

int updateRenderPhoto(DepthMap *dm);
void updatePhotoSlice();

// Commands for doing volumetric hole filling
// James & Lucas. 2/00
int Vrip_MuckWithVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);
int Vrip_IsoGradBlurVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);

int Vrip_QuantizeVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);

int Vrip_GaussBlurVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);
int Vrip_SharpenVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);
int Vrip_RemoveBWVolCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);


int Vrip_VolSizeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[]);


