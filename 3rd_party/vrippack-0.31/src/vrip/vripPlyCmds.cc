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
#include "vripGlobals.h"
#include "Mesh.h"
#include "plyio.h"
#include "vripPlyCmds.h"
#include "rangePly.h"

int
Vrip_FillPlyGapsCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 3) {
	interp->result = "Usage: vrip_fillplygaps <file-in> <file-out>";
	return TCL_ERROR;
    }

    Mesh *mesh = readMeshFromPly(argv[1], TRUE, FALSE);

    doConfidence(mesh);

    writePlyFile(argv[2], mesh);

    delete mesh;

    return TCL_OK;
}


int
Vrip_ExtendPlyEdgesCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 3) {
	interp->result = "Usage: vrip_extendplyedges <file-in> <file-out>";
	return TCL_ERROR;
    }

    Mesh *mesh = readMeshFromPly(argv[1], FALSE, TRUE);

    doConfidence(mesh);

    writePlyFile(argv[2], mesh);

    delete mesh;

    return TCL_OK;
}


int
Vrip_PlyConfidenceCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 3) {
	interp->result = "Usage: vrip_plyconf <file-in> <file-out>";
	return TCL_ERROR;
    }

    Mesh *mesh = readMeshFromPly(argv[1], FALSE, FALSE);

    doConfidence(mesh);    

    writePlyFile(argv[2], mesh);

    delete mesh;

    return TCL_OK;
}


int
Vrip_PlyCleanRangeMeshCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 3) {
	interp->result = "Usage: vrip_plycleanrangemesh <file-in> <file-out>";
	return TCL_ERROR;
    }

    Mesh *inMesh = readMeshFromPly(argv[1], FALSE, FALSE);

    Mesh *mesh = cleanMeshFromRangeMesh(inMesh);

    doConfidence(mesh);    
    
    writePlyFile(argv[2], mesh);

    delete mesh;
    delete inMesh;

    return TCL_OK;
}


int
Vrip_PlyCleanMeshCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 3) {
	interp->result = "Usage: vrip_plycleanmesh <file-in> <file-out>";
	return TCL_ERROR;
    }

    Mesh *inMesh = readMeshFromPly(argv[1], FALSE, FALSE);

    Mesh *mesh = cleanMesh(inMesh);

    writePlyFile(argv[2], mesh);

    delete mesh;
    delete inMesh;

    return TCL_OK;
}


int
Vrip_PlyDepthMapCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
   float noiseLevel = 0;

   if (argc != 2 && argc != 3) {
      interp->result = "Usage: vrip_plydmap <file-out> [noise_level]";
      return TCL_ERROR;
   }
   
   if (argc == 3) {
      noiseLevel = atof(argv[2]);
   }
   
   theDepthMap->writePly(argv[1], noiseLevel);
   
   return TCL_OK;
}


int
Vrip_PlyRangeGridToMeshCmd(ClientData, Tcl_Interp *interp, 
			   int argc, const char *argv[])
{
   if (argc >= 2) {
      interp->result = "Usage: vrip_rangegridtomesh [reads from stdin writes to stdout]";
      return TCL_ERROR;
   }
   
   RangeGrid *rangeGrid = readRangeGrid(NULL);

   if (rangeGrid == NULL)
      return TCL_ERROR;

   Mesh *mesh = meshFromGrid(rangeGrid, MeshResolution, FALSE);
   if (mesh == NULL) {
      delete rangeGrid;
      return TCL_ERROR;
   }

   doConfidence(mesh);    

   writePlyFile(NULL, mesh);

   delete mesh;
   delete rangeGrid;

   return TCL_OK;
}

