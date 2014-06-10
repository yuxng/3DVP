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


int Vrip_WriteDenCmd(ClientData dummy, Tcl_Interp *interp, 
		  int argc, const char *argv[]);
int Vrip_WriteGridCmd(ClientData dummy, Tcl_Interp *interp, 
		  int argc, const char *argv[]);
int Vrip_WriteGridNormCmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[]);
int Vrip_ReadGridCmd(ClientData dummy, Tcl_Interp *interp, 
		  int argc, const char *argv[]);
int Vrip_ReadFlatGridCmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[]);
int Vrip_WriteFlatGridCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_ReadRawImagesCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);

