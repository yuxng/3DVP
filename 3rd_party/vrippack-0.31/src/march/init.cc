/*

Name:         init.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Initialize some global variables.

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mc.h"
#include <unistd.h>

void Init()
{
  char OutMCFileName[PATH_MAX+3];
  int outlength;
  float maxsize;

  /* 
   * Read the EdgeTable file 
   */
  InitEdgeTable();

  occGrid = new OccGridRLE(10,10,10,CHUNK_SIZE);
  if (!(occGrid->read(infile))) {
      printf("Cannot open file %s for reading\n",infile);
      exit(-1);
  }


  dx = dy = dz = occGrid->resolution;

  printf("Resolution = %f\n", dx);

  Nx = occGrid->zdim;
  Ny = occGrid->xdim;
  Nz = occGrid->ydim;

  i1 = Nx - 2;
  j_1 = Ny - 2;
  k1 = Nz - 2;

  /* 
   * Initialize some variables
   */
  outlength = strlen(outfile);
  strcpy(OutMCFileName,outfile);
  sprintf(&(OutMCFileName[outlength]),".mc");

  maxsize = Nx*dx;
  if (Ny*dy > maxsize) maxsize = Ny*dy;
  if (Nz*dz > maxsize) maxsize = Nz*dz;
  FtoSScale = maxsize/65520.0;
  NrmMagScale = sqrt((double) 1/(dx*dx)+1/(dy*dy)+1/(dz*dz)) * 255.0/2.0/254.0;

  xstart = Nx/2.0*dx + occGrid->origin[2];
  ystart = Ny/2.0*dy + occGrid->origin[0];
  zstart = Nz/2.0*dz + occGrid->origin[1];

  xstart = 0;
  ystart = 0;
  zstart = 0;


  /* 
   * Initialize the output file
   */


/*
  if (!(OutMCFile = fopen(OutMCFileName,"w")))
    Fatal("Cannot open MC file for writing");

  MC_InitHeader(OutMCFile);
*/

}  /* Init */


