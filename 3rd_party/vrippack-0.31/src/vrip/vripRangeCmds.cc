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
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "vrip.h"
#include "occFunc.h"
#include "DepthMap.h"
#include "vripRangeCmds.h"
#include "vripGUICmds.h"
#include "vripGlobals.h"
#include "vripAux.h"

#include "plyio.h"
#include "DepthMapAux.h"
#include "renderGeom.h"
#include "configure.h"
#include "scan.h"
#include "scanRLE.h"
#include "scanLinePerspRLE.h"
#include "scanPerspRLE.h"
#include "scanNormRLE.h"
#include "Mesh.h"
#include "rangePly.h"

#include "defines.h"
#include "linePersp.h"
#include "perspective.h"

static clock_t tm;
static void start_time();
static void end_time();
static float time_elapsed();

static void parse_transformation(Mesh *mesh, Tcl_Interp *interp, 
				 int argc, const char**argv);

//extern unsigned long TclGetClicks();

int
Vrip_RangeScanCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }

    if (!SuperQuiet)
       printf("Integrating mesh %s...", argv[1]);

    initOccFunc();

    start_time();

    Mesh *mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    if (mesh == NULL)
	return TCL_ERROR;

    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);

    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;

    OrthoShear *shear = computeShear(yuk);
    configureGrid(theGrid, shear);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  theGrid->resolution/sampleSpacing;
    }

    BBox3f *gbbox = getBBox(theGrid);

    prepareRender(mesh, shear, gbbox, theGrid->resolution, 
		  theDepthMap, FALSE);
    softRenderConfidence(mesh);

    /*
    DepthMap *dm = renderDepthMap(mesh, shear, gbbox, 
				  theGrid->resolution, NULL);
				  */

    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails) {
	scanConvertDragTails(theGrid, shear, theDepthMap);
    }
    else {
	scanConvert(theGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
           printf("Time to process: %.2f sec\n", 
		  TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;

    return TCL_OK;    
}

int
Vrip_RangeScanTreeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (theGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    Mesh *mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    if (mesh == NULL)
	return TCL_ERROR;

    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);
    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;


    OrthoShear *shear = computeShear(yuk);
    configureGrid(theGrid, shear);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  theGrid->resolution/sampleSpacing;
    }

    BBox3f *gbbox = getBBox(theGrid);

    prepareRender(mesh, shear, gbbox, theGrid->resolution, 
		  theDepthMap, FALSE);
    softRenderConfidence(mesh);

    /*
    DepthMap *dm = renderDepthMap(mesh, shear, gbbox, 
				  theGrid->resolution, NULL);
				  */

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    scanConvertTree(theGrid, shear, theDepthMap);

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);
    
    delete shear;
    delete gbbox;

    return TCL_OK;    
}


int
Vrip_RangeScanNoTreeRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGrid == NULL || frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");

    fflush(stdout);

    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    }

    if (mesh == NULL)
	return TCL_ERROR;

    //fprintf(stderr, "Not assigning confidence!!\n");
    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);
    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;


    OrthoShear *shear = computeShear(yuk);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGrid(&frontRLEGrid, &backRLEGrid, shear);

    BBox3f *gbbox = getBBox(backRLEGrid);


    prepareRender(mesh, shear, gbbox, backRLEGrid->resolution, 
		  theDepthMap, FALSE);
    softRenderConfidence(mesh);

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");

    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails) {
	scanConvertDragTails(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }
    else {
	scanConvert(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}


int
Vrip_RangeScanRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGrid == NULL || frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");

    fflush(stdout);

    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    }

    if (mesh == NULL)
	return TCL_ERROR;

    //fprintf(stderr, "Not assigning confidence!!\n");
    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);
    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;


    OrthoShear *shear = computeShear(yuk);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGrid(&frontRLEGrid, &backRLEGrid, shear);

    BBox3f *gbbox = getBBox(backRLEGrid);

    prepareRender(mesh, shear, gbbox, backRLEGrid->resolution, 
		  theDepthMap, FALSE);
    softRenderConfidence(mesh);

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");

    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    theDepthMap->tagCellsForResampling();
    theDepthMap->fillTree(2);

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails) {
	if (OneLineAtATime) {
	    for (int zz = 0; zz < backRLEGrid->zdim; zz++) {
		printf("\rProcessing slice %d of %d...", 
		       zz+1, backRLEGrid->zdim);
		fflush(stdout);
		int somethingNew =
		    scanConvertTreeDragTailsOneLine(backRLEGrid, frontRLEGrid, 
						    shear, theDepthMap, zz);
		if (somethingNew) {
		    updatePhotoSlice();
		    Tcl_Eval(interp, "update");
		}
		swapgrids();
	    }
	    printf("\n");
	} else {
	    scanConvertTreeDragTails(backRLEGrid, frontRLEGrid, shear, theDepthMap);
	}
    }
    else {
	//scanConvert(backRLEGrid, frontRLEGrid, shear, theDepthMap);
	//scanConvertTree(backRLEGrid, frontRLEGrid, shear, theDepthMap);
	scanConvertTreeFast(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}


int
Vrip_RangeScanEdgesRLECmd(ClientData, Tcl_Interp *interp, 
			 int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGrid == NULL || frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");

    fflush(stdout);

    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, TRUE);
    }

    if (mesh == NULL)
	return TCL_ERROR;

    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);
    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;


    OrthoShear *shear = computeShear(yuk);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGrid(&frontRLEGrid, &backRLEGrid, shear);

    BBox3f *gbbox = getBBox(backRLEGrid);


    prepareRender(mesh, shear, gbbox, backRLEGrid->resolution, 
		  theDepthMap, FALSE);

    softRenderEdgeSteps(mesh);
    theDepthMap->updateEdgeSteps();

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");

//    sleep(5);

    softRenderConfidence(mesh);

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");

    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    theDepthMap->fillTree(2);

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails) {
	scanConvertTreeDragTails(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }
    else {
	//scanConvert(backRLEGrid, frontRLEGrid, shear, theDepthMap);
	//scanConvertTree(backRLEGrid, frontRLEGrid, shear, theDepthMap);
	scanConvertTreeBiasEdgesEmpty(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}



int
Vrip_RangeScanNormRLECmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGridNorm == NULL || frontRLEGridNorm == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }
    
    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");
    fflush(stdout);

    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    }

    if (mesh == NULL)
	return TCL_ERROR;

    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    Vec3f newdir;
    float angle;
    if (!mesh->isWarped)
	angle = 0;
    else if (mesh->isRightMirrorOpen)
	angle = 30;
    else
	angle = -30;

    Vec3f dir(-sin(RAD(angle)), 0, -cos(RAD(angle)));

    Matrix4f rot;
    mesh->quat.toMatrix(rot);

    rot.multVec(dir, newdir);

    if (Verbose)
	printf("View dir: [%f, %f, %f]\n", newdir.x, newdir.y, newdir.z);
    vec3f yuk;
    yuk[0] = newdir.x;
    yuk[1] = newdir.y;
    yuk[2] = newdir.z;


    OrthoShear *shear = computeShear(yuk);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGrid(&frontRLEGridNorm, &backRLEGridNorm, shear);

    BBox3f *gbbox = getBBox(backRLEGridNorm);

    prepareRender(mesh, shear, gbbox, backRLEGridNorm->resolution, 
		  theDepthMap, TRUE);
    softRenderNx(mesh);
    theDepthMap->updateNx();
    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");
    sleep(1);

    softRenderNy(mesh);
    theDepthMap->updateNy();
    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");
    sleep(1);

    softRenderNz(mesh);
    theDepthMap->updateNz();
    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");
    sleep(1);


    softRenderConfidence(mesh);
    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");

    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    theDepthMap->fillTree(2);

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

#if 0
    if (UseTails)
	scanConvertTreeDragTails(backRLEGridNorm, frontRLEGridNorm, 
				 shear, theDepthMap);
    else
	scanConvertTree(backRLEGridNorm, frontRLEGridNorm, shear, theDepthMap);
#else
	scanConvertTree(backRLEGridNorm, frontRLEGridNorm, shear, theDepthMap);
#endif


    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}



int
Vrip_RangeScanLinePerspCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[])
{
    Vec3f vec;

    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGrid == NULL || frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }    

    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");
    fflush(stdout);


    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    }

    if (!mesh->hasConfidence)
       doConfidence(mesh);

    end_time();

    TesselationTime = time_elapsed();

    start_time();

    parse_transformation(mesh, interp, argc, argv);


    float temp;
    int axis, flip;
    Vec3f center;
    center.setValue(frontRLEGrid->origin);

    if (frontRLEGrid->flip)
	center.z = -center.z;

    if (frontRLEGrid->axis == X_AXIS) {
	SWAP(center.x, center.z, temp);
    }
    else if (frontRLEGrid->axis == Y_AXIS) {
	SWAP(center.y, center.z, temp);
    }

    center.x += frontRLEGrid->resolution/2*frontRLEGrid->xdim;
    center.y += frontRLEGrid->resolution/2*frontRLEGrid->ydim;
    center.z += frontRLEGrid->resolution/2*frontRLEGrid->zdim;

    if (Verbose) {
	printf("Origin: (%f, %f, %f)\n", frontRLEGrid->origin[0], 
	       frontRLEGrid->origin[1], frontRLEGrid->origin[2]);
	printf("Center: (%f, %f, %f)\n", center.x, center.y, center.z);
    }

    OrthoShear *shear = initLinePersp(mesh->quat, mesh->trans, center, 
				      frontRLEGrid->resolution);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGridLinePersp(&frontRLEGrid, &backRLEGrid, shear);

    if (Verbose) {
	applyLinePersp(center, vec);
	printf("Transformed center: (%f, %f, %f)\n", vec.x, vec.y, vec.z);	
    }

    int lineWidth;
    BBox3f *gbbox = getBBoxLinePersp(backRLEGrid, &lineWidth);
    if (Verbose) {
	printf("Line width = %d\n", lineWidth);
    }

    prepareRenderLinePersp(mesh, shear, gbbox, 
			   backRLEGrid->resolution, theDepthMap, FALSE);
    softRenderConfidence(mesh);

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");


    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    theDepthMap->fillTree(lineWidth);

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails && !TailsOnly) {
	scanConvertLinePerspTreeDragTails(backRLEGrid, frontRLEGrid, 
					  shear, theDepthMap);
    } else if (TailsOnly) {
#if 1
	scanConvertLinePerspTreeTailsOnly(backRLEGrid, frontRLEGrid, 
					  shear, theDepthMap);
#else
	printf("Using fast carver...\n");
	scanConvertLinePerspTreeTailsOnlyFast(backRLEGrid, frontRLEGrid, 
					      shear, theDepthMap);
#endif
    }
    else {
	scanConvertLinePerspTree(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}


int
Vrip_RangeScanPerspCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[])
{
    Vec3f vec;

    if (argc != 2 && argc != 3 && argc != 6 && argc != 9 ) {
	interp->result = "wrong number of args";
	return TCL_ERROR;
    }

    if (backRLEGrid == NULL || frontRLEGrid == NULL) {
	interp->result = "Grid not allocated.";
	return TCL_ERROR;
    }    

    if (!SuperQuiet)
       printf("Integrating mesh %s...\n", argv[1]);

    initOccFunc();

    start_time();

    if (!Quiet)
	printf("Assiging confidences...\n");
    fflush(stdout);

    Mesh *mesh;
    if (UseTails && FillGaps) {
	mesh = readMeshFromPly(argv[1], TRUE, FALSE);
    }
    else  {
	mesh = readMeshFromPly(argv[1], FALSE, FALSE);
    }

    if (!mesh->hasConfidence)
       doConfidence(mesh, TRUE);

    end_time();

    TesselationTime = time_elapsed();

    if (!UsePerspectiveDir) {
       mesh->computeBBox();
       Vec3f bboxCenter = mesh->bbox.fll + mesh->bbox.nur;
       bboxCenter /= 2.0;
       PerspectiveDir = bboxCenter - PerspectiveCOP;
       PerspectiveDir.normalize();
       printf("Persp Dir: ");
       PerspectiveDir.print();
    }

    start_time();

    parse_transformation(mesh, interp, argc, argv);

    float temp;
    int axis, flip;
    Vec3f center;
    center.setValue(frontRLEGrid->origin);

    if (frontRLEGrid->flip)
	center.z = -center.z;

    if (frontRLEGrid->axis == X_AXIS) {
	SWAP(center.x, center.z, temp);
    }
    else if (frontRLEGrid->axis == Y_AXIS) {
	SWAP(center.y, center.z, temp);
    }

    center.x += frontRLEGrid->resolution/2*frontRLEGrid->xdim;
    center.y += frontRLEGrid->resolution/2*frontRLEGrid->ydim;
    center.z += frontRLEGrid->resolution/2*frontRLEGrid->zdim;

    if (Verbose) {
	printf("Origin: (%f, %f, %f)\n", frontRLEGrid->origin[0], 
	       frontRLEGrid->origin[1], frontRLEGrid->origin[2]);
	printf("Center: (%f, %f, %f)\n", center.x, center.y, center.z);
    }

    OrthoShear *shear = initPersp(mesh->quat, mesh->trans, center, 
				  frontRLEGrid->resolution);

    float sampleSpacing = sqrt(1 + shear->sx*shear->sx + shear->sy*shear->sy);

    if (UseEdgeLength) {
       MAX_DEPTH_DIFFERENCE = MaxEdgeLength/sampleSpacing;
    } 
    else {
       MAX_DEPTH_DIFFERENCE = tan(acos(MinViewDot))*
	  backRLEGrid->resolution/sampleSpacing;
    }

    configureGridPersp(&frontRLEGrid, &backRLEGrid, shear);

    if (Verbose) {
	applyPersp(center, vec);
	printf("Transformed center: (%f, %f, %f)\n", vec.x, vec.y, vec.z);	
    }

    int lineWidth;
    BBox3f *gbbox = getBBoxPersp(backRLEGrid, &lineWidth);
    if (Verbose) {
	printf("Line width = %d\n", lineWidth);
    }

    prepareRenderPersp(mesh, shear, gbbox, 
		       backRLEGrid->resolution, theDepthMap, FALSE);
    softRenderConfidence(mesh);

    updateRenderPhoto(theDepthMap);
    Tcl_Eval(interp, "update");


    if (UseTails) {
	if (FillBackground) {
	    fillBackground(theDepthMap);
	} else if (DoSilhouette) {
	   makeSilhouette(theDepthMap);
	}
    }

    theDepthMap->fillTree(lineWidth);

    end_time();

    ResampleRangeTime = time_elapsed();

    start_time();

    if (UseTails && !TailsOnly) {
	scanConvertPerspTreeDragTails(backRLEGrid, frontRLEGrid, 
					  shear, theDepthMap);
    } else if (TailsOnly) {
#if 1
	scanConvertPerspTreeTailsOnly(backRLEGrid, frontRLEGrid, 
					  shear, theDepthMap);
#else
	printf("Using fast carver...\n");
	scanConvertPerspTreeTailsOnlyFast(backRLEGrid, frontRLEGrid, 
					      shear, theDepthMap);
#endif
    }
    else {
	scanConvertPerspTree(backRLEGrid, frontRLEGrid, shear, theDepthMap);
    }

    end_time();

    MergeTime = time_elapsed();
    if (!SuperQuiet)
       printf("Time to process: %.2f sec\n", 
	      TesselationTime + ResampleRangeTime + MergeTime);

    delete shear;
    delete gbbox;
    delete mesh;

    return TCL_OK;    
}


static void
parse_transformation(Mesh *mesh, Tcl_Interp *interp, int argc, const char **argv)
{
    Quaternion quat;
    double xt,yt,zt;
    double rot;
    double q0,q1,q2,q3;

    /* parse the transformation info */
    
    /* argc == 2, no transformation */
    /* argc == 3, <rot-about-y-axis> */
    /* argc == 6, <tx> <ty> <tz> <rot-about-y-axis> */
    /* argc == 9, <tx> <ty> <tz> <q0> <q1> <q2> <q3> */
    

    // First decide if a .xf file exists
    int xfFileExists;

    // Awful code to create .xf filename
    char filename[PATH_MAX];
    strcpy(filename, argv[1]);
    char *dot = strrchr(filename, '.');
    *dot = 0;
    strcat(filename, ".xf");
    struct stat sts;
    if (stat(filename, &sts) == -1 && errno == ENOENT) {
       xfFileExists = 0;
    } else {
       xfFileExists = 1;
    }

    // If the .xf file exists and there are no overriding
    // xform arguments, then go ahead and use the .xf file
    if (argc == 2 && xfFileExists) {
       float r1, r2, r3, t1, t2, t3;
       Matrix4f mat;
       mat.makeIdentity();
       FILE *fp = fopen(filename, "r");
       mat.read(fp);
       t1 = mat.elem(0,3);
       t2 = mat.elem(1,3);
       t3 = mat.elem(2,3);

       /*
       fscanf(fp, "%f %f %f %f", &r1, &r2, &r3, &t1);
       mrot.setElem(0,0,r1);
       mrot.setElem(1,0,r2);
       mrot.setElem(2,0,r3);
       fscanf(fp, "%f %f %f %f", &r1, &r2, &r3, &t2);
       mrot.setElem(0,1,r1);
       mrot.setElem(1,1,r2);
       mrot.setElem(2,1,r3);
       fscanf(fp, "%f %f %f %f", &r1, &r2, &r3, &t3);
       mrot.setElem(0,2,r1);
       mrot.setElem(1,2,r2);
       mrot.setElem(2,2,r3);
       */

       mesh->quat.fromMatrix(mat);
       mesh->trans.setValue(t1,t2,t3);

       return;
    }

    /* translation */
    if (argc == 2 || argc == 3) {
	mesh->trans.setValue(0,0,0);
    }
    else {
	Tcl_GetDouble (interp, argv[2], &xt);
	Tcl_GetDouble (interp, argv[3], &yt);
	Tcl_GetDouble (interp, argv[4], &zt);
	mesh->trans.setValue(xt, yt, zt);
    }
    
    /* rotation */
    Matrix4f mrot;
    mrot.makeIdentity();
    if (argc == 9) {
	Tcl_GetDouble (interp, argv[5], &q0);
	Tcl_GetDouble (interp, argv[6], &q1);
	Tcl_GetDouble (interp, argv[7], &q2);
	Tcl_GetDouble (interp, argv[8], &q3);
	mesh->quat.q[0] = q0;
	mesh->quat.q[1] = q1;
	mesh->quat.q[2] = q2;
	mesh->quat.q[3] = q3;
	mesh->quat.toMatrix(mrot);
	mrot.transpose();
	mesh->quat.fromMatrix(mrot);
    }
    else if (argc == 6) {
	Tcl_GetDouble (interp, argv[5], &rot);
	mrot.rotateY(RAD(rot));
	mesh->quat.fromMatrix(mrot);
    }
    else if (argc == 3) {
	Tcl_GetDouble (interp, argv[2], &rot);
	mrot.rotateY(RAD(rot));
	mesh->quat.fromMatrix(mrot);
    }
    else {
	mesh->quat.fromMatrix(mrot);
    }
}


static void
start_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime;
}

static void
end_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime - tm;
}

static float
time_elapsed()
{
  return (double) tm / (double) CLOCKS_PER_SEC;
}

