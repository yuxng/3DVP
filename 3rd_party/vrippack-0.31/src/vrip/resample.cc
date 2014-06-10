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


#include "resample.h"
#include "vripGlobals.h"


void
resample(DepthMap *depthMap, float xOff, float yOff, 
	 float *depth, float *confidence)
{
    int ix, iy, offset, xdim;
    float dx, dy, z1, z2, z3, z4, c1, c2, c3, c4;
    DepthElement *buf;

    xdim = depthMap->xdim;

    ix = int(xOff);
    iy = int(yOff);

    offset = ix + iy*xdim;
    buf = depthMap->elems + offset;

    z1 = buf->z;
    c1 = buf->conf;

    z2 = (buf+1)->z;
    c2 = (buf+1)->conf;

    z3 = (buf+xdim)->z;
    c3 = (buf+xdim)->conf;

    z4 = (buf+xdim+1)->z;
    c4 = (buf+xdim+1)->conf;

    if (!IS_VALID_DEPTH(z1) || !IS_VALID_DEPTH(z2) || 
	!IS_VALID_DEPTH(z3) || !IS_VALID_DEPTH(z4)) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	return;
    }

    float maxZ = MAX(MAX(MAX(z1,z2),z3),z4);
    float minZ = MIN(MIN(MIN(z1,z2),z3),z4);

    if (maxZ - minZ > MAX_DEPTH_DIFFERENCE) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	return;
    }

    dx = xOff - ix;
    dy = yOff - iy;

    *depth = ((1-dx)*(1-dy)*z1 + (dx)*(1-dy)*z2 
	      + (1-dx)*(dy)*z3 + (dx)*(dy)*z4);

    *confidence = ((1-dx)*(1-dy)*c1 + (dx)*(1-dy)*c2 
		   + (1-dx)*(dy)*c3 + (dx)*(dy)*c4);

    if (*confidence < 0) 
	*confidence = 0;
}


void
resampleBetter(DepthMap *depthMap, float xOff, float yOff, 
	       float *depth, float *confidence)
{
    int ix, iy, offset, xdim;
    float dx, dy, z1, z2, z3, z4, c1, c2, c3, c4;
    float alpha, beta, gamma;
    DepthElement *buf;

    xdim = depthMap->xdim;

    ix = int(xOff);
    iy = int(yOff);

    offset = ix + iy*xdim;
    uchar tag = depthMap->tags[offset];
    if (!tag) {
       *depth = -FAR_AWAY_DEPTH;
       *confidence = 0;
       return;
    }

    buf = depthMap->elems + offset;

    z1 = buf->z;
    c1 = buf->conf;

    z2 = (buf+1)->z;
    c2 = (buf+1)->conf;

    z3 = (buf+xdim)->z;
    c3 = (buf+xdim)->conf;

    z4 = (buf+xdim+1)->z;
    c4 = (buf+xdim+1)->conf;

#if 0
    if (!(((DepthMap::TRIANGLE_ONE & tag) && (DepthMap::TRIANGLE_TWO & tag)) ||
	((DepthMap::TRIANGLE_THREE & tag) && (DepthMap::TRIANGLE_FOUR & tag)))) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	return;
    }

    dx = xOff - ix;
    dy = yOff - iy;

    *depth = ((1-dx)*(1-dy)*z1 + (dx)*(1-dy)*z2 
	      + (1-dx)*(dy)*z3 + (dx)*(dy)*z4);

    *confidence = ((1-dx)*(1-dy)*c1 + (dx)*(1-dy)*c2 
		   + (1-dx)*(dy)*c3 + (dx)*(dy)*c4);

    if (*confidence < 0) 
	*confidence = 0;

#else

    dx = xOff - ix;
    dy = yOff - iy;

    if (dx > dy) {
       if (DepthMap::TRIANGLE_THREE & tag) {
	  alpha = dy;
	  beta = 1-dx;
	  gamma = 1 - alpha - beta;
	  *depth = alpha*z4 + beta*z1 + gamma*z2;
	  *confidence = alpha*c4 + beta*c1 + gamma*c2;
	  return;
       } 
    } else if (DepthMap::TRIANGLE_FOUR & tag) {
       alpha = 1-dy;
       beta = dx;
       gamma = 1 - alpha - beta;
       *depth = alpha*z1 + beta*z4 + gamma*z3;
       *confidence = alpha*c1 + beta*c4 + gamma*c3;
       return;
    }

    if (1-dx > dy) {
       if (DepthMap::TRIANGLE_ONE & tag) {
	  alpha = dy;
	  beta = dx;
	  gamma = 1 - alpha - beta;
	  *depth = alpha*z3 + beta*z2 + gamma*z1;
	  *confidence = alpha*c3 + beta*c2 + gamma*c1;
	  return;
       } 
    } else if (DepthMap::TRIANGLE_TWO & tag) {
       alpha = 1-dy;
       beta = 1-dx;
       gamma = 1 - alpha - beta;
       *depth = alpha*z2 + beta*z3 + gamma*z4;
       *confidence = alpha*c2 + beta*c3 + gamma*c4;
       return;
    }

    *depth = -FAR_AWAY_DEPTH;
    *confidence = 0;
    return;

#endif

}


void
resampleWithEdge(DepthMap *depthMap, float xOff, float yOff, 
		 float *depth, float *confidence, float *edge)
{
    int ix, iy, offset, xdim;
    float dx, dy, z1, z2, z3, z4, c1, c2, c3, c4;
    float e1, e2, e3, e4;
    DepthElement *buf;
    signed char *edgeBuf;

    xdim = depthMap->xdim;

    ix = int(xOff);
    iy = int(yOff);

    offset = ix + iy*xdim;
    buf = depthMap->elems + offset;
    edgeBuf = depthMap->edgeSteps + offset;

    z1 = buf->z;
    c1 = buf->conf;
    e1 = *edgeBuf;

    z2 = (buf+1)->z;
    c2 = (buf+1)->conf;
    e2 = *(edgeBuf+1);

    z3 = (buf+xdim)->z;
    c3 = (buf+xdim)->conf;
    e3 = *(edgeBuf+xdim);

    z4 = (buf+xdim+1)->z;
    c4 = (buf+xdim+1)->conf;
    e4 = *(edgeBuf+xdim+1);

    if (!IS_VALID_DEPTH(z1) || !IS_VALID_DEPTH(z2) || 
	!IS_VALID_DEPTH(z3) || !IS_VALID_DEPTH(z4)) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	*edge = 0;
	return;
    }

    float maxZ = MAX(MAX(MAX(z1,z2),z3),z4);
    float minZ = MIN(MIN(MIN(z1,z2),z3),z4);

    if (maxZ - minZ > MAX_DEPTH_DIFFERENCE) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	*edge = 0;
	return;
    }

    dx = xOff - ix;
    dy = yOff - iy;

    *depth = ((1-dx)*(1-dy)*z1 + (dx)*(1-dy)*z2 
	      + (1-dx)*(dy)*z3 + (dx)*(dy)*z4);

    *confidence = ((1-dx)*(1-dy)*c1 + (dx)*(1-dy)*c2 
		   + (1-dx)*(dy)*c3 + (dx)*(dy)*c4);

    *edge = (((1-dx)*(1-dy)*e1 + (dx)*(1-dy)*e2 
	     + (1-dx)*(dy)*e3 + (dx)*(dy)*e4))/128.0;

    if (*confidence < 0) 
	*confidence = 0;
}



void
resampleForCarving(DepthMap *depthMap, float xOff, float yOff, 
		   float *depth, float *confidence)
{
    int ix, iy, offset, xdim;
    float dx, dy, z1, z2, z3, z4, c1, c2, c3, c4;
    DepthElement *buf;

    xdim = depthMap->xdim;

    ix = int(xOff);
    iy = int(yOff);

    offset = ix + iy*xdim;
    buf = depthMap->elems + offset;

    z1 = buf->z;
    c1 = buf->conf;

    z2 = (buf+1)->z;
    c2 = (buf+1)->conf;

    z3 = (buf+xdim)->z;
    c3 = (buf+xdim)->conf;

    z4 = (buf+xdim+1)->z;
    c4 = (buf+xdim+1)->conf;

    if (!IS_VALID_DEPTH(z1) || !IS_VALID_DEPTH(z2) || 
	!IS_VALID_DEPTH(z3) || !IS_VALID_DEPTH(z4)) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	return;
    }

    float maxZ = MAX(MAX(MAX(z1,z2),z3),z4);
    float minZ = MIN(MIN(MIN(z1,z2),z3),z4);
    float minC = MIN(MIN(MIN(c1,c2),c3),c4);

    dx = xOff - ix;
    dy = yOff - iy;

    *depth = ((1-dx)*(1-dy)*z1 + (dx)*(1-dy)*z2 
	      + (1-dx)*(dy)*z3 + (dx)*(dy)*z4);

    *confidence = ((1-dx)*(1-dy)*c1 + (dx)*(1-dy)*c2 
		   + (1-dx)*(dy)*c3 + (dx)*(dy)*c4);

    if (maxZ - minZ > MAX_DEPTH_DIFFERENCE || minC <= 0) {
	*confidence = 0;
    }
}


void
resampleNorm(DepthMap *depthMap, float xOff, float yOff, 
	     float *depth, float *confidence, Vec3f &norm)
{
    int ix, iy, offset, xdim;
    float dx, dy, z1, z2, z3, z4, c1, c2, c3, c4;
    float nx, ny, nz;
    float nx1, ny1, nz1;
    float nx2, ny2, nz2;
    float nx3, ny3, nz3;
    float nx4, ny4, nz4;
    DepthElement *buf;
    DepthMapNorm *normBuf;

    xdim = depthMap->xdim;

    ix = int(xOff);
    iy = int(yOff);

    offset = ix + iy*xdim;
    buf = depthMap->elems + offset;
    normBuf = depthMap->norms + offset;

    z1 = buf->z;
    z2 = (buf+1)->z;
    z3 = (buf+xdim)->z;
    z4 = (buf+xdim+1)->z;

    if (!IS_VALID_DEPTH(z1) || !IS_VALID_DEPTH(z2) || 
	!IS_VALID_DEPTH(z3) || !IS_VALID_DEPTH(z4)) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	norm.setValue(0,0,0);
	return;
    }

    float maxZ = MAX(MAX(MAX(z1,z2),z3),z4);
    float minZ = MIN(MIN(MIN(z1,z2),z3),z4);

    if (maxZ - minZ > MAX_DEPTH_DIFFERENCE) {
	*depth = -FAR_AWAY_DEPTH;
	*confidence = 0;
	norm.setValue(0,0,0);
	return;
    }

    c1 = buf->conf;
    c2 = (buf+1)->conf;
    c3 = (buf+xdim)->conf;
    c4 = (buf+xdim+1)->conf;

    nx1 = normBuf->nx;
    ny1 = normBuf->ny;
    nz1 = normBuf->nz;

    nx2 = (normBuf+1)->nx;
    ny2 = (normBuf+1)->ny;
    nz2 = (normBuf+1)->nz;

    nx3 = (normBuf+xdim)->nx;
    ny3 = (normBuf+xdim)->ny;
    nz3 = (normBuf+xdim)->nz;

    nx4 = (normBuf+xdim+1)->nx;
    ny4 = (normBuf+xdim+1)->ny;
    nz4 = (normBuf+xdim+1)->nz;

    dx = xOff - ix;
    dy = yOff - iy;

    *depth = ((1-dx)*(1-dy)*z1 + (dx)*(1-dy)*z2 
	      + (1-dx)*(dy)*z3 + (dx)*(dy)*z4);

    *confidence = ((1-dx)*(1-dy)*c1 + (dx)*(1-dy)*c2 
		   + (1-dx)*(dy)*c3 + (dx)*(dy)*c4);

    nx = ((1-dx)*(1-dy)*nx1 + (dx)*(1-dy)*nx2 
	  + (1-dx)*(dy)*nx3 + (dx)*(dy)*nx4);

    ny = ((1-dx)*(1-dy)*ny1 + (dx)*(1-dy)*ny2 
	  + (1-dx)*(dy)*ny3 + (dx)*(dy)*ny4);

    nz = ((1-dx)*(1-dy)*nz1 + (dx)*(1-dy)*nz2 
	  + (1-dx)*(dy)*nz3 + (dx)*(dy)*nz4);

    norm.setValue(nx, ny, nz);
    norm.normalize();
    
}



