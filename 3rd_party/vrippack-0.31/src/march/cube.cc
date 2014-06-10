/*

Name:         cube.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Processes a single cube.  Is passed absolute i,j,k position.

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
#include <iostream>
#include <math.h>
#include <assert.h>
#include "mc.h"

static TriangleVertex VertexOnEdge[12];


static void Interpolate(int n, Cube cube, int i, int j, int k);


TriangleVertex*
NewDoCube(Cube cube, int i, int j, int k, unsigned char *pEdgeTableIndex)
{
  int n;
  unsigned char EdgeTableIndex = 0x00;

  /* form 8 bit index into edge table */
  for (n=0;n<8;n++)
    if (cube[n].density > threshold) 
      EdgeTableIndex = EdgeTableIndex | (1 << n);

  /* if edge table entry indicates triangles, */
  if (TheEdgeTable[EdgeTableIndex].Ntriangles != 0) {

    /* interpolate the active edges, */
    for (n=0;n<12;n++)
      if ((TheEdgeTable[EdgeTableIndex].edge)[n])
	Interpolate(n,cube,i,j,k);

    TotalTriangles += TheEdgeTable[EdgeTableIndex].Ntriangles;

  } /* if (Ntriangles != 0) */

  *pEdgeTableIndex = EdgeTableIndex;
  return VertexOnEdge;
  
} /* DoCube */


/*
 * Interpolate along one edge of cube, setting VertexOnEdge[n]
 */
static void 
Interpolate(int n, Cube cube, int i, int j, int k)
{
  float alpha=0,beta;
  float x=0,y=0,z=0,nx,ny,nz,mag,confidence;
  float NrmScale;  /* scales nx,ny,nz to just within -32768 to 32767 */
  Index a=0,b=0;
  unsigned char realData, valid=0;

  realData = TRUE;

  /* set the spatial components */
  switch (n) {

    /*
     * four edges in k direction
     */

  case 0:
      if (cube[1].density == cube[0].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[0].density)/
	      (cube[1].density - cube[0].density);
      x = xstart - i*dx;
      y = ystart + (k+alpha)*dy;
      z = zstart - (j+1)*dz;
      a = 0;
      b = 1;
      realData = cube[0].realData && cube[1].realData;      
      valid = cube[0].valid && cube[1].valid;      
      break;

  case 2:
      if (cube[2].density == cube[3].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[3].density)/
	      (cube[2].density - cube[3].density);
      x = xstart - i*dx;
      y = ystart + (k+alpha)*dy;
      z = zstart - j*dz;
      a = 3;
      b = 2;
      realData = cube[2].realData && cube[3].realData;
      valid = cube[2].valid && cube[3].valid;
      break;

  case 4:
      if (cube[5].density == cube[4].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[4].density)/
	      (cube[5].density - cube[4].density);
      x = xstart - (i+1)*dx;
      y = ystart + (k+alpha)*dy;
      z = zstart - (j+1)*dz;
      a = 4;
      b = 5;
      realData = cube[4].realData && cube[5].realData;
      valid = cube[4].valid && cube[5].valid;
      break;

  case 6:
      if (cube[6].density == cube[7].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[7].density)/
	      (cube[6].density - cube[7].density);
      x = xstart - (i+1)*dx;
      y = ystart + (k+alpha)*dy;
      z = zstart - j*dz;
      a = 7;
      b = 6;
      realData = cube[6].realData && cube[7].realData;
      valid = cube[6].valid && cube[7].valid;
      break;

    /*
     * four edges in j direction
     */

  case 1:
      if (cube[1].density == cube[2].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[2].density)/
	      (cube[1].density - cube[2].density);
      x = xstart - i*dx;
      y = ystart + (k+1)*dy;
      z = zstart - (j+alpha)*dz;
      a = 2;
      b = 1;
      realData = cube[1].realData && cube[2].realData;
      valid = cube[1].valid && cube[2].valid;
      break;
      
  case 3:
      if (cube[0].density == cube[3].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[3].density)/
	      (cube[0].density - cube[3].density);
      x = xstart - i*dx;
      y = ystart + k*dy;
      z = zstart - (j+alpha)*dz;
      a = 3;
      b = 0;
      realData = cube[0].realData && cube[3].realData;
      valid = cube[0].valid && cube[3].valid;
      break;

  case 5:
      if (cube[5].density == cube[6].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[6].density)/
	      (cube[5].density - cube[6].density);
      x = xstart - (i+1)*dx;
      y = ystart + (k+1)*dy;
      z = zstart - (j+alpha)*dz;
      a = 6;
      b = 5;
      realData = cube[5].realData && cube[6].realData;
      valid = cube[5].valid && cube[6].valid;
      break;
      
  case 7:
      if (cube[4].density == cube[7].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[7].density)/
	      (cube[4].density - cube[7].density);
      x = xstart - (i+1)*dx;
      y = ystart + k*dy;
      z = zstart - (j+alpha)*dz;
      a = 7;
      b = 4;
      realData = cube[4].realData && cube[7].realData;
      valid = cube[4].valid && cube[7].valid;
      break;

    /*
     * four edges in i direction
     */

  case 8:
      if (cube[4].density == cube[0].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[0].density)/
	      (cube[4].density - cube[0].density);
      x = xstart - (i+alpha)*dx;
      y = ystart + k*dy;
      z = zstart - (j+1)*dz;
      a = 0;
      b = 4;
      realData = cube[0].realData && cube[4].realData;
      valid = cube[0].valid && cube[4].valid;
      break;

  case 9:
      if (cube[5].density == cube[1].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[1].density)/
	      (cube[5].density - cube[1].density);
      x = xstart - (i+alpha)*dx;
      y = ystart + (k+1)*dy;
      z = zstart - (j+1)*dz;
      a = 1;
      b = 5;
      realData = cube[1].realData && cube[5].realData;
      valid = cube[1].valid && cube[5].valid;
      break;

  case 10:
      if (cube[7].density == cube[3].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[3].density)/
	      (cube[7].density - cube[3].density);
      x = xstart - (i+alpha)*dx;
      y = ystart + k*dy;
      z = zstart - j*dz;
      a = 3;
      b = 7;
      realData = cube[3].realData && cube[7].realData;
      valid = cube[3].valid && cube[7].valid;
      break;

  case 11:
      if (cube[6].density == cube[2].density)
	  alpha = 0.5;
      else
	  alpha=(threshold - cube[2].density)/
	      (cube[6].density - cube[2].density);
      x = xstart - (i+alpha)*dx;
      y = ystart + (k+1)*dy;
      z = zstart - j*dz;
      a = 2;
      b = 6;
      realData = cube[2].realData && cube[6].realData;
      valid = cube[2].valid && cube[6].valid;
      break;
  }

  /* compute the normal components and magnitude */
  beta = 1 - alpha;
  nx = beta*cube[a].nx + alpha*cube[b].nx;
  ny = beta*cube[a].ny + alpha*cube[b].ny;
  nz = beta*cube[a].nz + alpha*cube[b].nz;
  confidence = beta*cube[a].confidence + alpha*cube[b].confidence;
  mag = sqrt(nx*nx + ny*ny + nz*nz);

  if (mag == 0) {
      nx = 0;
      ny = 0;
      nz = 0;
  }
  else {
      nx = nx/mag;
      ny = ny/mag;
      nz = nz/mag;
  }

  VertexOnEdge[n].x = x;
  VertexOnEdge[n].y = y;
  VertexOnEdge[n].z = z;
  VertexOnEdge[n].nx = nx;
  VertexOnEdge[n].ny = ny;
  VertexOnEdge[n].nz = nz;
  VertexOnEdge[n].realData = realData;
  VertexOnEdge[n].valid = valid;

  if (SaveGradientAsConfidence)
     VertexOnEdge[n].confidence = mag;
  else
     VertexOnEdge[n].confidence = confidence;

  VertexOnEdge[n].tex1 = (unsigned char) round(mag);
  VertexOnEdge[n].tex2 = 0;

} /* Interpolate */


