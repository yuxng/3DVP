/*

Name:         var.h

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Global variables.


Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include "OccGridRLE.h"
#include "limits.h"

/*
 * File variables
 */
char infile[PATH_MAX], outfile[PATH_MAX];
FILE *InSlice,*OutMCFile;
int FirstSliceFileNumber;
int WriteNormals;
int UseValueWeightProduct;
int SaveGradientAsConfidence;

/*
 * Data Set Dimensions
 */
int Nx,Ny,Nz;
float dx,dy,dz;

/*
 * Scale factors
 */
float FtoSScale;
float NrmMagScale;

/*
 * Surface Specs
 */
int i0,j_0,k0;
int i1,j_1,k1;
float threshold;
float xstart,ystart,zstart;
int TotalTriangles;

/*
 * The edge table
 */
EdgeTableEntry TheEdgeTable[256];

int OCC_CONF_THRESHOLD;

OccGridRLE *occGrid;
