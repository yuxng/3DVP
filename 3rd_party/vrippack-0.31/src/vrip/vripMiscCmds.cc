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
#include "vripMiscCmds.h"
#include "vripGlobals.h"
#include "configure.h"
#include "occFunc.h"



int
Vrip_ParamCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
    int ival, count;
    uchar ucval;
    float fval;

    if (argc == 1) {
        printf("Command: vrip_param [-option value]\n");
        printf("  -verbose <boolean> (%d)\n", Verbose);
        printf("  -warn <boolean> (%d)\n", Warn);
        printf("  -quiet <boolean> (%d)\n", Quiet);

        printf("  -show_norm <boolean> (%d)\n", ShowNormals);
        printf("  -show_weight_slice <boolean> (%d)\n", ShowConfSlice);
        printf("  -show_value_weight <boolean> (%d)\n", ShowValueWeight);

        printf("  -use_tails <boolean> (%d)\n", UseTails);
        printf("  -tails_only <boolean> (%d)\n", TailsOnly);

        printf("  -fill_gaps <boolean> (%d)\n", FillGaps);
        printf("  -fill_bg <boolean> (%d)\n", FillBackground);
        printf("  -do_silhouette <boolean> (%d)\n", DoSilhouette);
        printf("  -extend_boundary_steps <byte> (%d)\n", EdgeExtensionSamples);
        //printf("  -mesh_resolution <int> (%d)\n", MeshResolution);

        printf("  -line_persp_center <float> <float> <float> (%f, %f, %f)\n", 
	       LASER_LINE_AT_T0_X, LASER_LINE_AT_T0_Y, LASER_LINE_AT_T0_Z);
        printf("  -line_persp_dir <float> <float> <float> (%f, %f, %f)\n", 
	       LASER_LINE_DIR_X, LASER_LINE_DIR_Y, LASER_LINE_DIR_Z);

	printf("  -persp_center <float> <float> <float> (%f, %f, %f)\n", 
	       PerspectiveCOP.x, PerspectiveCOP.y, PerspectiveCOP.z);
        printf("  -persp_dir <float> <float> <float> (%f, %f, %f)\n", 
	       PerspectiveDir.x, PerspectiveDir.y, PerspectiveDir.z);
        printf("  -use_persp_dir <boolean> (%d)\n", UsePerspectiveDir);

	printf("  -use_length <boolean> (%d)\n", UseEdgeLength);
        printf("  -min_view_dot <float> (%f)\n", MinViewDot);
        printf("  -max_edge_length <float> (%f)\n", MaxEdgeLength);
        printf("  -min_color <char> (%d)\n", MinColor);

        printf("  -vert_weight_bias <byte> (%d)\n", ConfidenceBias);

        printf("  -view_weight_exp <float> (%f)\n", ConfidenceExponent);

        printf("  -boundary_weight_exp <float> (%f)\n", EdgeConfExponent);
        printf("  -max_boundary_steps <int> (%d)\n", EdgeConfSteps);
        printf("  -min_boundary_weight <byte> (%d)\n", MinEdgeConfidence);
        printf("  -min_vertex_confidence float (%f)\n", MinVertexConfidence);

        printf("  -ramp_width <float> (%f)\n", OccupancyRampWidth);
        printf("  -w1 <float> (%f)\n", WeightPos1);
        printf("  -w2 <float> (%f)\n", WeightPos2);
        printf("  -w3 <float> (%f)\n", WeightPos3);
        printf("  -w4 <float> (%f)\n", WeightPos4);
        printf("  -w5 <float> (%f)\n", WeightPos5);
        printf("  -min_voxel_weight <byte> (%d)\n", MinWeight);

	return TCL_OK;
    }

    count = 1;
    while (count < argc) {
        if (EQSTR(argv[count], "-verbose")) {
            count++;
            ival = atoi(argv[count]);
	    Verbose = ival != 0;
        } 
	else if (EQSTR(argv[count], "-warn")) {
            count++;
            ival = atoi(argv[count]);
	    Warn = ival != 0;
        }
	else if (EQSTR(argv[count], "-line_persp_center")) {
            count++;
            fval = atof(argv[count]);
	    LASER_LINE_AT_T0_X = fval;
	    count++;
            fval = atof(argv[count]);
	    LASER_LINE_AT_T0_Y = fval;
	    count++;
            fval = atof(argv[count]);
	    LASER_LINE_AT_T0_Z = fval;
        }
	else if (EQSTR(argv[count], "-line_persp_dir")) {
            count++;
            fval = atof(argv[count]);
	    LASER_LINE_DIR_X = fval;
	    count++;
            fval = atof(argv[count]);
	    LASER_LINE_DIR_Y = fval;
	    count++;
            fval = atof(argv[count]);
	    LASER_LINE_DIR_Z = fval;
        }
	else if (EQSTR(argv[count], "-persp_center")) {
            count++;
            fval = atof(argv[count]);
	    PerspectiveCOP.x = fval;
	    count++;
            fval = atof(argv[count]);
	    PerspectiveCOP.y = fval;
	    count++;
            fval = atof(argv[count]);
	    PerspectiveCOP.z = fval;
        }
	else if (EQSTR(argv[count], "-persp_dir")) {
            count++;
            fval = atof(argv[count]);
	    PerspectiveDir.x = fval;
	    count++;
            fval = atof(argv[count]);
	    PerspectiveDir.y = fval;
	    count++;
            fval = atof(argv[count]);
	    PerspectiveDir.z = fval;
        }
	else if (EQSTR(argv[count], "-use_tails")) {
            count++;
            ival = atoi(argv[count]);
	    UseTails = ival != 0;
        }
	else if (EQSTR(argv[count], "-tails_only")) {
            count++;
            ival = atoi(argv[count]);
	    TailsOnly = ival != 0;
        }
	else if (EQSTR(argv[count], "-fill_gaps")) {
            count++;
            ival = atoi(argv[count]);
	    FillGaps = ival != 0;
        }
	else if (EQSTR(argv[count], "-fill_bg")) {
            count++;
            ival = atoi(argv[count]);
	    FillBackground = ival != 0;
        }
	else if (EQSTR(argv[count], "-do_silhouette")) {
            count++;
            ival = atoi(argv[count]);
	    DoSilhouette = ival != 0;
        }
	else if (EQSTR(argv[count], "-show_norm")) {
            count++;
            ival = atoi(argv[count]);
	    ShowNormals = ival != 0;
        }
	else if (EQSTR(argv[count], "-show_value_weight")) {
            count++;
            ival = atoi(argv[count]);
	    ShowValueWeight = ival != 0;
        }
	else if (EQSTR(argv[count], "-show_weight_slice") ||
	   EQSTR(argv[count], "-show_conf_slice")) {
            count++;
            ival = atoi(argv[count]);
	    ShowConfSlice = ival != 0;
        }
	else if (EQSTR(argv[count], "-quiet")) {
            count++;
            ival = atoi(argv[count]);
	    Quiet = ival != 0;
        }
	else if (EQSTR(argv[count], "-superQuiet")) {
            count++;
            ival = atoi(argv[count]);
	    SuperQuiet = ival != 0;
        }
	else if (EQSTR(argv[count], "-mesh_resolution")) {
            count++;
            ival = atoi(argv[count]);
	    MeshResolution = ival;
        }
	else if (EQSTR(argv[count], "-vert_weight_bias") ||
	   EQSTR(argv[count], "-confbias")) {
            count++;
            ival = atoi(argv[count]);
	    ConfidenceBias = ival;
        }
	else if (EQSTR(argv[count], "-min_boundary_weight") ||
	   EQSTR(argv[count], "-minedgeconf")) {
            count++;
            ival = atoi(argv[count]);
	    MinEdgeConfidence = ival;
        }
	else if (EQSTR(argv[count], "-min_vertex_confidence")) {
            count++;
            fval = atof(argv[count]);
	    MinVertexConfidence = fval;
        }
	else if (EQSTR(argv[count], "-max_boundary_steps") ||
	   EQSTR(argv[count], "-edgeconfsteps")) {
            count++;
            ival = atoi(argv[count]);
	    EdgeConfSteps = ival;
        }
	else if (EQSTR(argv[count], "-min_voxel_weight") ||
	   EQSTR(argv[count], "-minweight")) {
            count++;
            ival = atoi(argv[count]);
	    MinWeight = ival;
        }
	else if (EQSTR(argv[count], "-extend_boundary_steps") ||
	   EQSTR(argv[count], "-extedge")) {
            count++;
            ival = atoi(argv[count]);
	    EdgeExtensionSamples = ival;
        }
	else if (EQSTR(argv[count], "-max_edge_length") ||
	   EQSTR(argv[count], "-maxedge")) {
            count++;
            fval = atof(argv[count]);
	    MaxEdgeLength = fval;
        }
	else if (EQSTR(argv[count], "-min_color") ||
	   EQSTR(argv[count], "-mincolor")) {
            count++;
            ucval = atoi(argv[count]);
	    MinColor = ucval;
        }
	else if (EQSTR(argv[count], "-use_length") ||
	   EQSTR(argv[count], "-uselength")) {
            count++;
            ival = atoi(argv[count]);
	    UseEdgeLength = ival;
        }
	else if (EQSTR(argv[count], "-min_view_dot") ||
	   EQSTR(argv[count], "-minviewdot")) {
            count++;
            fval = atof(argv[count]);
	    MinViewDot = fval;
        }
	else if (EQSTR(argv[count], "-ramp_width") ||
	   EQSTR(argv[count], "-rampwidth")) {
            count++;
            fval = atof(argv[count]);
	    OccupancyRampWidth = fval;
        }
	else if (EQSTR(argv[count], "-w1")) {
            count++;
            fval = atof(argv[count]);
	    WeightPos1 = fval;
        }
	else if (EQSTR(argv[count], "-w2")) {
            count++;
            fval = atof(argv[count]);
	    WeightPos2 = fval;
        }
	else if (EQSTR(argv[count], "-w3")) {
            count++;
            fval = atof(argv[count]);
	    WeightPos3 = fval;
        }
	else if (EQSTR(argv[count], "-w4")) {
            count++;
            fval = atof(argv[count]);
	    WeightPos4 = fval;
        }
	else if (EQSTR(argv[count], "-w5")) {
            count++;
            fval = atof(argv[count]);
	    WeightPos5 = fval;
        }
	else if (EQSTR(argv[count], "-boundary_weight_exp") ||
	   EQSTR(argv[count], "-edgeconfexp")) {
            count++;
            fval = atof(argv[count]);
	    EdgeConfExponent = fval;
        }
	else if (EQSTR(argv[count], "-view_weight_exp") ||
	   EQSTR(argv[count], "-confexp")) {
            count++;
            fval = atof(argv[count]);
	    ConfidenceExponent = fval;
        } else {
	   Tcl_AppendResult(interp, "vrip_param: No such parameter, `", 
			    argv[count], "'", (char *)NULL);
	   return TCL_ERROR;
	}
	count++;
    }

    return TCL_OK;
}



int
Vrip_MergeTimeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
   char time[PATH_MAX];

   sprintf(time, "%f", MergeTime);
   Tcl_SetResult(interp, time, TCL_VOLATILE);

   return TCL_OK;
}


int
Vrip_TessTimeCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
   char time[PATH_MAX];

   sprintf(time, "%f", TesselationTime);
   Tcl_SetResult(interp, time, TCL_VOLATILE);

   return TCL_OK;
}


int
Vrip_ResampleRangeTimeCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[])
{
   char time[PATH_MAX];

   sprintf(time, "%f", ResampleRangeTime);
   Tcl_SetResult(interp, time, TCL_VOLATILE);

   return TCL_OK;
}


int
Vrip_CopyrightCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
   printf("\n");
   printf("Copyright (2001)\n");
   printf("\n");
   printf("The Board of Regents of the University of Washington (UW)\n");
   printf("The Board of Trustees of the Leland Stanford Junior University (Stanford)\n");
   printf("\n");
   printf("Except for commercial resale, lease, license or other\n");
   printf("commercial transactions, permission is hereby given to use, copy,\n");
   printf("modify this software for academic purposes only.  No part of this\n");
   printf("software or any derivatives thereof may be used in the production of\n");
   printf("computer models for resale or for use in a commercial product.\n");
   printf("UW AND STANFORD MAKE NO REPRESENTATIONS OR WARRANTIES OF ANY\n");
   printf("KIND CONCERNING THIS SOFTWARE.  No support is implied or provided.\n");
   printf("\n");

   return TCL_OK;
}


int
PcreateCmd(ClientData, Tcl_Interp *interp, int argc, const char *argv[])
{
   // This used to mean something under IRIX...

   /*   char newArgv[100][PATH_MAX];
    char path[PATH_MAX];
    int result;

    if (argc == 1) {
	interp->result = "wrong # of args";
	return TCL_ERROR;
    }

    strcpy(path,argv[1]);
    for (int i = 1; i < argc; i++) {
	strcpy(newArgv[i-1],argv[i]);
    }
    newArgv[argc-1] = NULL;

#ifndef LINUX
    result = pcreatevp(path, newArgv);

    wait(NULL);
#endif
   */

    return TCL_OK;
}

int
Vrip_CalibrateRotationCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[])
{
   FILE *fp = fopen(argv[1], "r");
   
   int i, count = 0;
   float t0, t1, t2, q0, q1, q2, q3, norm;
   float inAngles[1000], angles[1000], angle;
   Quaternion quats[1000];
   Vec3f trans[1000], axes[1000], center[1000], axis;
   while (fscanf(fp, "%f %f %f %f %f %f %f %f", 
		&angle, &t0, &t1, &t2, &q0, &q1, &q2, &q3) > 0) {
      inAngles[count] = angle;
      quats[count].setValue(q0, q1, q2, q3);
      quats[count].normalize();
      trans[count].setValue(t0,t1,t2);
      count++;
   }

   fclose(fp);

   Matrix4f mbase, mbaseinv, mrot, newMat, mat;
   Quaternion quat, quatAccum;
   Vec3f transAccum;

   printf("\nXforms relative to origin:\n\n");

   mbase.makeIdentity();
   quats[0].toMatrix(mbase);
   mbase.setTranslate(trans[0]);
   mbaseinv = mbase.inverse();

   for (i = 1; i < count; i++) {
      mat.makeIdentity();
      quats[i].toMatrix(mat);
      mat.setTranslate(trans[i]);
      mat.multLeft(mbaseinv);
      quat.fromMatrix(mat);
      quat.toAxisAngle(angles[i], axes[i]);
      
      if (axes[i][1] > 0) {
	 angles[i] = 360 - angles[i];
	 axes[i].negate();
      }
      printf("Angle = %f, Axis = (%f, %f, %f)\n", angles[i],
	     axes[i][0], axes[i][1], axes[i][2]);

      newMat.setValue(mat);
      newMat.setElem(0,0,1-newMat.elem(0,0));
      newMat.setElem(1,1,1-newMat.elem(1,1));
      newMat.setElem(2,2,1-newMat.elem(2,2));
      newMat.inverse();

      printf("Center = (%g, %g, %g)\n", 
	     -newMat.elem(0,3), -newMat.elem(1,3), -newMat.elem(2,3));
   }

   printf("\nXforms relative to previous scan:\n\n");

   quatAccum.setValue(0,0,0,0);
   transAccum.setValue(0,0,0);

   for (i = 1; i < count; i++) {
      mbase.makeIdentity();
      quats[i-1].toMatrix(mbase);
      mbase.setTranslate(trans[i-1]);
      mbaseinv = mbase.inverse();

      mat.makeIdentity();
      quats[i].toMatrix(mat);
      mat.setTranslate(trans[i]);
      mat.multLeft(mbaseinv);
      quat.fromMatrix(mat);

      quat.normalize();

      quatAccum.q[0] += quat.q[0];
      quatAccum.q[1] += quat.q[1];
      quatAccum.q[2] += quat.q[2];
      quatAccum.q[3] += quat.q[3];

      quat.toAxisAngle(angles[i], axes[i]);
      
      if (axes[i][1] > 0) {
	 angles[i] = 360 - angles[i];
	 axes[i].negate();
      }
      printf("Angle = %f, Axis = (%f, %f, %f)\n", angles[i],
	     axes[i][0], axes[i][1], axes[i][2]);

      newMat.setValue(mat);
      newMat.setElem(0,0,1-newMat.elem(0,3));
      newMat.setElem(1,1,1-newMat.elem(1,3));
      newMat.setElem(2,2,1-newMat.elem(2,3));
      newMat.inverse();

      Vec3f center(newMat.elem(0,3), newMat.elem(1,3), newMat.elem(2,3));
      center.negate();

      transAccum += center;

      printf("Center = (%g, %g, %g)\n", center[0], center[1], center[2]);
   }
   
   transAccum /= -(count-1);

   quatAccum.q[0] /= (count-1);
   quatAccum.q[1] /= (count-1);
   quatAccum.q[2] /= (count-1);
   quatAccum.q[3] /= (count-1);
   quatAccum.normalize();

   quatAccum.toAxisAngle(angle, axis);

   printf("\nRotation axis and translation based on pairwise:\n\n");
   printf("Angle = %f, Axis = (%f, %f, %f)\n", angle,
	  axis[0], axis[1], axis[2]);
   printf("Center = (%g, %g, %g)\n", 
	  transAccum[0], transAccum[1], transAccum[2]);

   return TCL_OK;
   
}

int
Vrip_TestVAScannerCmd(ClientData, Tcl_Interp *interp, 
			  int argc, const char *argv[])
{
   int i;
   int numPairs;
   int doHorizontal;
   float triangAngle;
   float tiltAngle;
   FILE *fp;
   char filename[PATH_MAX];
   Matrix4f mview1, mview1inv, mview2, mview2inv;

   numPairs = atoi(argv[1]);
   triangAngle = atof(argv[2])*M_PI/180;
   tiltAngle = atof(argv[3])*M_PI/180;
   doHorizontal = !strcmp(argv[4],"horiz");
   
   // compute viewing transformations
   for (i = 0; i < numPairs; i++) {
      float pairAngle = 360/numPairs * i * M_PI/180; 

      if (doHorizontal) {
	 mview1.makeIdentity();
	 mview1.setRotateY(pairAngle);
	 mview1.rotateX(-tiltAngle);
	 
	 sprintf(filename, "view%d-A.xf", i);
	 fp = fopen(filename, "w");
	 mview1.write(fp);
	 fclose(fp);
	 
	 mview2.makeIdentity();
	 mview2.setRotateY(pairAngle+triangAngle);
	 mview2.rotateX(-tiltAngle);
	 
	 mview1inv = mview1.inverse();
	 mview1inv.multLeft(mview2);
	 
	 sprintf(filename, "view%d-A-to-B.xf", i);
	 fp = fopen(filename, "w");
	 mview1inv.write(fp);
	 fclose(fp);
	 
	 mview2inv = mview2.inverse();
	 
	 sprintf(filename, "align%d-B.xf", i);
	 fp = fopen(filename, "w");
	 mview2inv.write(fp);
	 fclose(fp);      
      } else {
	 mview1.makeIdentity();
	 mview1.setRotateY(pairAngle);
	 mview1.rotateX(-tiltAngle-triangAngle/2);
	 
	 sprintf(filename, "view%d-A.xf", i);
	 fp = fopen(filename, "w");
	 mview1.write(fp);
	 fclose(fp);
	 
	 mview2.makeIdentity();
	 mview2.setRotateY(pairAngle);
	 mview2.rotateX(-tiltAngle+triangAngle/2);
	 
	 mview1inv = mview1.inverse();
	 mview1inv.multLeft(mview2);
	 
	 sprintf(filename, "view%d-A-to-B.xf", i);
	 fp = fopen(filename, "w");
	 mview1inv.write(fp);
	 fclose(fp);
	 
	 mview2inv = mview2.inverse();
	 
	 sprintf(filename, "align%d-B.xf", i);
	 fp = fopen(filename, "w");
	 mview2inv.write(fp);
	 fclose(fp);
      }
   }

   return TCL_OK;
}
