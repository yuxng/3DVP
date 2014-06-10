/*

Name:         mc.h

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Include file for global definitions and typing

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#ifndef _MC_H_
#define _MC_H_

#include "mcfile.h"
#include "OccGridRLE.h"
#include "limits.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Global definitions
 */

typedef struct {
    float density;
    float nx;
    float ny;
    float nz;
    float confidence;
    unsigned char valid;
    unsigned char realData;    
} Point;

typedef Point **Section;

typedef Point Cube[8];

typedef int Index;

typedef struct {Index A;
		Index B;
		Index C;
	      } Triple;

typedef struct {Boolean edge[12];
		int Ntriangles;
		Triple *TriangleList;
	      } EdgeTableEntry;

#define Nrow Nz
#define Ncol Ny
#define Nslice Nx

#define drow dz
#define dcol dy
#define dslice dx

#define round(x) ((int) ((x) > 0 ? (x)+0.5 : (x)-0.5))


/*
 * Global variable typing
 */

extern char infile[PATH_MAX], outfile[PATH_MAX];
extern int WriteNormals;
extern int UseValueWeightProduct;
extern int SaveGradientAsConfidence;
extern FILE *InSlice,*OutMCFile;
extern int FirstSliceFileNumber;

extern int Nx,Ny,Nz;
extern float dx,dy,dz;

extern float FtoSScale;
extern float NrmMagScale;

extern int i0,j_0,k0;
extern int i1,j_1,k1;
extern float threshold;
extern float xstart,ystart,zstart;
extern int TotalTriangles;
extern OccGridRLE *occGrid;
extern int OCC_CONF_THRESHOLD;

extern EdgeTableEntry TheEdgeTable[256];

/*
 * Global function types
 */

extern void GetInfo(),Init(),DoSlices(), DoSlicesOccRLE();
extern void InitEdgeTable();
extern void DoCube(Cube cube, int i, int j, int k);
extern TriangleVertex *NewDoCube(Cube cube, int i, int j, int k, 
				 unsigned char *pEdgeTableIndex);
extern void Quit();
extern void Fatal(char *message);

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 100000
#endif

#ifdef __cplusplus
}
#endif

#endif
