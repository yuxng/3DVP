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


#include "Linear.h"
#include "defines.h"
#include "OccGrid.h"
#include "OccGridRLE.h"
#include "OccGridNormRLE.h"
#include "DepthMap.h"

int SuperQuiet = FALSE;
int Quiet = FALSE;
int Verbose = FALSE;
int Warn = FALSE;
int MeshResolution = 1;
int UseTails = FALSE;
int TailsOnly = FALSE;
int FillGaps = FALSE;
int FillBackground = FALSE;
int DoSilhouette = FALSE;
int ShowNormals = FALSE;
int ShowConfSlice = FALSE;
int ShowValueWeight = FALSE;

int OneLineAtATime = FALSE;

float LASER_LINE_DIR_X = 1;
float LASER_LINE_DIR_Y = 0;
float LASER_LINE_DIR_Z = 0;

float LASER_LINE_AT_T0_X = 0;
float LASER_LINE_AT_T0_Y = 0.15920981547868;
float LASER_LINE_AT_T0_Z = 1.22793387875641;

int UsePerspectiveDir = 0;
Vec3f PerspectiveCOP(0,0,0);
Vec3f PerspectiveDir(0,0,-1);

int MaxStepsToEdge;
float EdgeLength;
float MinViewDot = 0.15;
float MaxEdgeLength = 0.003;
uchar MinColor = 0;
int UseEdgeLength = 0;
float EdgeExtensionAngle = 45;
int EdgeExtensionSamples = 3;
float ConfidenceExponent = 2.0;
float EdgeConfExponent = 1;
unsigned char MinWeight = 0;
int EdgeConfSteps = 8;
unsigned char MinEdgeConfidence = 10;
unsigned char ConfidenceBias = 1;
float MinVertexConfidence = 10/255.0;
float OccupancyRampWidth = 0.002;
float WeightPos1 = OccupancyRampWidth/2 + 0.002;
float WeightPos2 = WeightPos1 - 0.0005;
float WeightPos3 = -OccupancyRampWidth/2;
float WeightPos4 = WeightPos3 - 0.0005;
float WeightPos5 = WeightPos4 - 0.0005;

OccGrid *theGrid;
OccGridRLE *backRLEGrid;
OccGridRLE *frontRLEGrid;
OccGridNormRLE *backRLEGridNorm;
OccGridNormRLE *frontRLEGridNorm;
DepthMap *theDepthMap;

float D1, D2, M_VALUE, B_VALUE;
float C1, C2, C3, C4, C5;
float M_WEIGHT_1, M_WEIGHT_2, B_WEIGHT_1, B_WEIGHT_2;
float MAX_STRETCH;
float MAX_DEPTH_DIFFERENCE;

float MergeTime = 0;
float TesselationTime = 0;
float ResampleRangeTime = 0;
