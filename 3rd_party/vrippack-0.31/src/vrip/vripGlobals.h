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


#ifndef _VRIP_GLOBALS_
#define _VRIP_GLOBALS_

#include "Linear.h"
#include "OccGrid.h"
#include "OccGridRLE.h"
#include "OccGridNormRLE.h"
#include "DepthMap.h"


extern float LASER_LINE_DIR_X;
extern float LASER_LINE_DIR_Y;
extern float LASER_LINE_DIR_Z;

extern float LASER_LINE_AT_T0_X;
extern float LASER_LINE_AT_T0_Y;
extern float LASER_LINE_AT_T0_Z;

extern int UsePerspectiveDir;
extern Vec3f PerspectiveCOP;
extern Vec3f PerspectiveDir;

extern int SuperQuiet;
extern int Verbose;
extern int Warn;
extern int Quiet;
extern int MeshResolution;
extern int UseTails;
extern int TailsOnly;
extern int FillGaps;
extern int FillBackground;
extern int DoSilhouette;
extern int ShowNormals;
extern int ShowConfSlice;
extern int ShowValueWeight;
extern int OneLineAtATime;
extern int MaxStepsToEdge;
extern float EdgeLength;
extern int EdgeExtensionSamples;
extern float EdgeExtensionAngle;
extern unsigned char ConfidenceBias;
extern float MinViewDot;
extern float MaxEdgeLength;
extern uchar MinColor;
extern float ConfidenceExponent;
extern float EdgeConfExponent;
extern float MinVertexConfidence;
extern unsigned char MinWeight;
extern int EdgeConfSteps;
extern int UseEdgeLength;
extern unsigned char MinEdgeConfidence;
extern float OccupancyRampWidth;
extern float WeightPos1;
extern float WeightPos2;
extern float WeightPos3;
extern float WeightPos4;
extern float WeightPos5;


extern OccGrid *theGrid;
extern OccGridRLE *backRLEGrid;
extern OccGridRLE *frontRLEGrid;
extern OccGridNormRLE *backRLEGridNorm;
extern OccGridNormRLE *frontRLEGridNorm;
extern DepthMap *theDepthMap;

extern float D1, D2, M_VALUE, B_VALUE;
extern float C1, C2, C3, C4, C5;
extern float M_WEIGHT_1, M_WEIGHT_2, B_WEIGHT_1, B_WEIGHT_2;
extern float MAX_STRETCH;
extern float MAX_DEPTH_DIFFERENCE;

extern float MergeTime;
extern float TesselationTime;
extern float ResampleRangeTime;

#endif

