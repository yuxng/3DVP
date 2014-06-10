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


#include <stdlib.h>
#include <signal.h>

#include "vrip.h"
#include "vripGlobals.h"
#include "scan.h"
#include "occFunc.h"

#include "vripInit.h"
#include "vripFillCmds.h"
#include "vripMiscCmds.h"
#include "vripGridCmds.h"
#include "vripFileCmds.h"
#include "vripRangeCmds.h"
#include "vripGUICmds.h"
#include "vripPlyCmds.h"

int
Vrip_Init(Tcl_Interp *interp)
{
    char *occGridDir, *homeDir;
    char occPath[PATH_MAX], rcPath[PATH_MAX];
    int code, rcFileExists;
    Tcl_CmdDeleteProc *nullDeleteProc;
    ClientData nullClientData;

    Tcl_CreateCommand(interp, "pcreate", PcreateCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_newgrid", Vrip_NewGridCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_newgridrle", Vrip_NewGridRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_newgridnormrle", Vrip_NewGridNormRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_writegridnorm", Vrip_WriteGridNormCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_writegrid", Vrip_WriteGridCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_readgrid", Vrip_ReadGridCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_readflatgrid", Vrip_ReadFlatGridCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_writeflatgrid", Vrip_WriteFlatGridCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_writeden", Vrip_WriteDenCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_readrawimages", Vrip_ReadRawImagesCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_avgdown", Vrip_AvgDownCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_avgdownrle", Vrip_AvgDownRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_extract", Vrip_ExtractCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_ellipse", Vrip_EllipseCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_ellipserle", Vrip_EllipseRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_cylinderrle", Vrip_CylinderRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_chair", Vrip_ChairRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_gumdrop_torus", 
		      Vrip_GumdropTorusRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_sinewave", 
		      Vrip_SinewaveRLECmd, NULL, NULL);

    Tcl_CreateCommand(interp, "vrip_fillrle", Vrip_FillRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_fillplygaps", Vrip_FillPlyGapsCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_extendplyedges", Vrip_ExtendPlyEdgesCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_plyconf", Vrip_PlyConfidenceCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangegridtomesh", 
		      Vrip_PlyRangeGridToMeshCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_plycleanrangemesh", Vrip_PlyCleanRangeMeshCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_plycleanmesh", Vrip_PlyCleanMeshCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_cube", Vrip_CubeCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_cuberle", Vrip_CubeRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_transpxz", Vrip_TransposeXZCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_transpyz", Vrip_TransposeYZCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_transpxzrle", Vrip_TransposeXZRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_transpxyrle", Vrip_TransposeXYRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_transpyzrle", Vrip_TransposeYZRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_photoslice", Vrip_PhotoSliceCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_photoslicerle", Vrip_PhotoSliceRLECmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_getvoxelrle", Vrip_GetVoxelRLECmd,
		      NULL, NULL);

    Tcl_CreateCommand(interp, "vrip_volSize", Vrip_VolSizeCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_photoslicenormrle", 
		      Vrip_PhotoSliceNormRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_setrenderphoto", Vrip_SetRenderPhotoCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_setphotoslice", Vrip_SetPhotoSliceCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescan", Vrip_RangeScanCmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescantree", Vrip_RangeScanTreeCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescanrle", Vrip_RangeScanRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescanrlenotree", Vrip_RangeScanNoTreeRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescannormrle", Vrip_RangeScanNormRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescanedgesrle", 
		      Vrip_RangeScanEdgesRLECmd, NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescanlin", Vrip_RangeScanLinePerspCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_rangescanpersp", Vrip_RangeScanPerspCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_param", Vrip_ParamCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_blurvb", Vrip_BlurVisBordersCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_varfromconst", Vrip_ExpandVarToConstCmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_inforle", Vrip_InfoRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_infonormrle", Vrip_InfoNormRLECmd, 
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_copyright", Vrip_CopyrightCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_tess_time", Vrip_TessTimeCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_merge_time", Vrip_MergeTimeCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_resamp_range_time", 
		      Vrip_ResampleRangeTimeCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_plydmap", 
		      Vrip_PlyDepthMapCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_calibraterotation", 
		      Vrip_CalibrateRotationCmd,
		      NULL, NULL);
    Tcl_CreateCommand(interp, "vrip_testvascanner", 
		      Vrip_TestVAScannerCmd,
		      NULL, NULL);

    theGrid = NULL;
    frontRLEGrid = NULL;
    backRLEGrid = NULL;
    theDepthMap = NULL;

    occGridDir = getenv("VRIP_DIR");
    if (occGridDir == NULL)
	occGridDir = ".";

    strcpy(occPath, occGridDir);
    strcat(occPath, "/vrip.tcl");

    code = Tcl_EvalFile(interp, occPath);
    if (code != TCL_OK) {
       // This is an ugly bit of code to get around the fact that the compiler
       //  doesn't like to pass the result of Tcl_GetVar directly to 
       //  Tcl_SetResult.  It would be better if there were a "free" procedure
       //  to pass instead of NULL.  Oh well.
       int length = strlen(Tcl_GetVar (interp, "errorInfo", TCL_GLOBAL_ONLY));
       char *result = (char *)malloc(length*sizeof(char));
       strcpy(result, Tcl_GetVar (interp, "errorInfo", TCL_GLOBAL_ONLY));
       Tcl_SetResult(interp, result, NULL);
      return TCL_ERROR;
    }

    homeDir = getenv("HOME");
    if (homeDir == NULL) {
       fprintf(stderr, "Environment variable HOME not set - will not execute $HOME/.vriprc\n");
       return TCL_OK;
    }

    strcpy(rcPath, homeDir);
    strcat(rcPath, "/.vriprc");
    Tcl_VarEval(interp, "file exists ", rcPath, (char *)NULL);
    rcFileExists = atoi(interp->result);

    if (rcFileExists) {
       code = Tcl_EvalFile(interp, rcPath);
    } else {
       code = TCL_OK;
    }

    // Don't die on FPE's
    signal(SIGFPE, SIG_IGN);

    return code;
}


