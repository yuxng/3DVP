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

int Vrip_NewGridCmd(ClientData dummy, Tcl_Interp *interp, 
		  int argc, const char *argv[]);
int Vrip_NewGridRLECmd(ClientData, Tcl_Interp *interp, 
		       int argc, const char *argv[]);
int Vrip_NewGridNormRLECmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_InfoRLECmd(ClientData, Tcl_Interp *interp, 
		    int argc, const char *argv[]);
int Vrip_InfoNormRLECmd(ClientData, Tcl_Interp *interp, 
			int argc, const char *argv[]);
int Vrip_AvgDownCmd(ClientData dummy, Tcl_Interp *interp, 
		  int argc, const char *argv[]);
int Vrip_AvgDownRLECmd(ClientData dummy, Tcl_Interp *interp, 
		      int argc, const char *argv[]);
int Vrip_AvgDownRLE_oldCmd(ClientData dummy, Tcl_Interp *interp, 
		      int argc, const char *argv[]);
int Vrip_TransposeXZCmd(ClientData, Tcl_Interp *interp, 
		       int argc, const char *argv[]);
int Vrip_TransposeYZCmd(ClientData, Tcl_Interp *interp, 
		       int argc, const char *argv[]);
int Vrip_TransposeXZRLECmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_TransposeXYRLECmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_TransposeYZRLECmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_ExtractCmd(ClientData, Tcl_Interp *interp, 
		   int argc, const char **);
int Vrip_BlurVisBordersCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[]);
int Vrip_ExpandVarToConstCmd(ClientData, Tcl_Interp *interp, 
			    int argc, const char **);
