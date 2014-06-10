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

int Vrip_RangeScanRLECmd(ClientData, Tcl_Interp *interp, 
			int argc, const char *argv[]);
int Vrip_RangeScanCmd(ClientData, Tcl_Interp *interp, 
		     int argc, const char *argv[]);
int Vrip_RangeScanEdgesRLECmd(ClientData, Tcl_Interp *interp, 
			    int argc, const char *argv[]);
int Vrip_RangeScanNormRLECmd(ClientData, Tcl_Interp *interp, 
			    int argc, const char *argv[]);
int Vrip_RangeScanTreeCmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[]);
int Vrip_RangeScanLinePerspCmd(ClientData, Tcl_Interp *interp, 
			      int argc, const char *argv[]);
int Vrip_RangeScanPerspCmd(ClientData, Tcl_Interp *interp, 
			   int argc, const char *argv[]);
int Vrip_RangeScanNoTreeRLECmd(ClientData, Tcl_Interp *interp, 
			    int argc, const char *argv[]);

