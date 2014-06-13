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
#include <stdio.h>

#include "OccGrid.h"
//#include "levden.h"

OccGrid::OccGrid()
{
    this->xdim = this->ydim = this->zdim = 0;
    this->elems = NULL;
    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;
}


OccGrid::OccGrid(int xd, int yd, int zd)
{
    int size1, size2, size3, sliceSize;

    this->xdim = xd;
    this->ydim = yd;
    this->zdim = zd;
    this->elems = new OccElement[xd*yd*zd];
    if (this->elems == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	this->zdim = 0;
	printf("OccGrid::new - insufficient memory.\n");
    }

    int maxdim = MAX(this->xdim, MAX(this->ydim, this->zdim));
    this->sliceOrigins = new vec3f[maxdim];

    this->axis = Z_AXIS;
    this->flip = FALSE;
    this->origin[0] = 0;
    this->origin[1] = 0;
    this->origin[2] = 0;

    size1 = xd*yd;
    size2 = xd*zd;
    size3 = yd*zd;

    sliceSize = MAX(MAX(size1, size2), size3);
    this->slice = new OccElement[sliceSize];
    if (this->elems == NULL) {
	printf("OccGrid::new - insufficient memory.\n");
    }
}


OccGrid::~OccGrid()
{
    if (this->elems != NULL) {
	delete [] this->elems;
    }
}


int
OccGrid::write(char *)
{
    return TRUE;
}


int
OccGrid::writeDen(char *filename)
{
   /*

    orig_min[0] = extr_min[0] = map_min[0] = 0;
    orig_min[1] = extr_min[1] = map_min[1] = 0;
    orig_min[2] = extr_min[2] = map_min[2] = 0;

    orig_max[0] = extr_max[0] = map_max[0] = this->xdim - 1;
    orig_max[1] = extr_max[1] = map_max[1] = this->ydim - 1;
    orig_max[2] = extr_max[2] = map_max[2] = this->zdim - 1;

    orig_len[0] = extr_len[0] = map_len[0] = this->xdim;
    orig_len[1] = extr_len[1] = map_len[1] = this->ydim;
    orig_len[2] = extr_len[2] = map_len[2] = this->zdim;

    map_warps = 0;

    map_length = (long)map_len[X] * (long)map_len[Y] * (long)map_len[Z];

    map_address = (uchar *)(this->elems);

    Store_Indexed_DEN_File(filename, sizeof(OccElement));

    */
    
    // So much for error checking...
    return TRUE;
}


int
OccGrid::read(char *)
{
    return TRUE;
}


int
OccGrid::transposeXZ()
{
    int dim, bufInc;
    OccElement *buf1, *buf2;
    int xx,yy,zz;

    // Assumes that all dimensions are equal
    if (this->xdim != this->ydim || this->ydim != this->zdim) {
	printf("OccGrid::transposeXZ - need equal dimensions.\n");
	return FALSE;
    }

    dim = this->xdim;
    bufInc = dim*dim;

    for (zz = 0; zz < dim; zz++) {
	for (yy = 0; yy < dim; yy++) {
	    buf1 = this->elems + yy*dim + zz*dim*dim;
	    buf2 = this->elems + zz + yy*dim;
	    for (xx = 0; xx < zz; xx++, buf1++, buf2+=bufInc) {
	        SWAP_USHORT(buf1->value, buf2->value);
		SWAP_USHORT(buf1->totalWeight, buf2->totalWeight);
	    }	    
	}
    }

    SWAP_FLOAT(this->origin[0], this->origin[2]);

    return TRUE;
}


int
OccGrid::transposeYZ()
{
    int dim;
    OccElement *buf1, *buf2;
    int xx,yy,zz;

    // Assumes that all dimensions are equal
    if (this->xdim != this->ydim || this->ydim != this->zdim) {
	printf("OccGrid::transposeYZ - need equal dimensions.\n");
	return FALSE;
    }

    dim = this->xdim;

    for (zz = 0; zz < dim; zz++) {
	for (yy = 0; yy < zz; yy++) {
	    buf1 = this->elems + yy*dim + zz*dim*dim;
	    buf2 = this->elems + zz*dim + yy*dim*dim;
	    for (xx = 0; xx < dim; xx++, buf1++, buf2++) {
		SWAP_USHORT(buf1->value, buf2->value);
		SWAP_USHORT(buf1->totalWeight, buf2->totalWeight);
	    }	    
	}
    }

    SWAP_FLOAT(this->origin[1], this->origin[2]);

    return TRUE;
}

int
OccGrid::copy(OccGrid *src)
{
    OccElement *el = this->elems;
    OccElement *srcel = src->elems;

    if (this->xdim != src->xdim || this->ydim != src->ydim ||
	this->zdim != src->zdim) {
      fprintf(stderr, "Error:  OccGrid::Copy -- grids must be equal size\n");
      return FALSE;
    }
    
    int nels = this->xdim * this->ydim * this->zdim;
    // For every element, copy value & weight
    for (int i=0; i < nels; i++) {
      el->value = srcel->value;
      el->totalWeight = srcel->totalWeight;
      el++; 
      srcel++;
    }

    return TRUE;
}


OccElement *
OccGrid::getSlice(char *axis, int sliceNum, int *pxdim, int *pydim)
{
    OccElement *buf1, *buf2;
    int xx, yy, zz, bufInc;

    buf1 = slice;
    if (EQSTR(axis, "x")) {
	if (sliceNum >= this->xdim)
	    return this->slice;
	bufInc = -this->xdim*this->ydim;
	for (yy = 0; yy < this->ydim; yy++) {
	    buf2 = this->address(sliceNum, yy, this->zdim-1);
	    for (zz = this->zdim-1; zz >= 0 ; zz--, buf1++, buf2+=bufInc) {
		buf1->value = buf2->value;
		buf1->totalWeight = buf2->totalWeight;
	    }
	}
	*pxdim = this->ydim;
	*pydim = this->zdim;
    }
    else if (EQSTR(axis, "y")) {
	if (sliceNum >= this->ydim)
	    return this->slice;
	bufInc = 1;
	for (zz = this->zdim-1; zz >= 0; zz--) {
	    buf2 = this->address(0, sliceNum, zz);
	    for (xx = 0; xx < this->xdim; xx++, buf1++, buf2+=bufInc) {
		buf1->value = buf2->value;
		buf1->totalWeight = buf2->totalWeight;
	    }
	}
	*pxdim = this->xdim;
	*pydim = this->zdim;
    }
    else if (EQSTR(axis, "z")) {
	if (sliceNum >= this->ydim)
	    return this->slice;
	bufInc = 1;
	for (yy = 0; yy < this->ydim; yy++) {
	    buf2 = this->address(0, yy, sliceNum);
	    for (xx = 0; xx < this->xdim; xx++, buf1++, buf2+=bufInc) {
		buf1->value = buf2->value;
		buf1->totalWeight = buf2->totalWeight;
	    }
	}
	*pxdim = this->xdim;
	*pydim = this->ydim;
    }

    return this->slice;
}


void
OccGrid::clear()
{
    OccElement *buf = this->elems;
    for (int i = 0; i < this->xdim*this->ydim*this->zdim; i++, buf++) {
	buf->value = 0;
	buf->totalWeight = 0;
    }
}

