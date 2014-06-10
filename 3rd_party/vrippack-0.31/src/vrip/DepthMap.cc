/*

Brian Curless

Computer Graphics Laboratory
Stanford University

---------------------------------------------------------------------

Copyright (1997-2001) The Board of Trustees of the Leland Stanford
Junior University. Except for commercial resale, lease, license or
other commercial transactions, permission is hereby given to use,
copy, modify this software for academic purposes only.  No part of
this software or any derivatives thereof may be used in the production
of computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include "DepthMap.h"
#include "defines.h"
#include "linePersp.h"
#include "perspective.h"
#include "vripGlobals.h"
#include <limits.h>
#include <ply.h>
#include <math.h>
#include <stdlib.h>

#ifdef linux
#include <float.h>
#endif

static char *elem_names[] = { 
  "vertex", "range_grid"
};

static PlyProperty vert_props[] =  {
    {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0}
};

static PlyProperty range_grid_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};

struct PlyVertex {
    float x, y, z;
};

struct RangeGridList {
    uchar nverts;
    int *verts;
};

static PlyVertex *vert_dummy;
#define voffset(field) ((char *) (&vert_dummy->field) - (char *) vert_dummy)
RangeGridList *range_grid_dummy;
#define roffset(field) ((char *) (&range_grid_dummy->field) - (char *) range_grid_dummy)


DepthMap::DepthMap()
{
    this->xdim = this->ydim = 0;
    this->origXdim = this->origYdim = 0;
    this->elems = NULL;
    this->norms = NULL;
    this->tags = NULL;
    this->treeElems = NULL;
    this->holeTreeElems = NULL;
}


DepthMap::DepthMap(int xd, int yd, int useNorms)
{
    this->xdim = xd;
    this->ydim = yd;

    this->origXdim = xd;
    this->origYdim = yd;
    
    this->elems = new DepthElement[xd*yd];
    if (this->elems == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
    }

    this->tags = new uchar[xd*yd];
    if (this->tags == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
    }

    this->edgeSteps = new signed char[xd*yd];
    if (this->edgeSteps == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
    }

    if (useNorms) {
	this->norms = new DepthMapNorm[xd*yd];
	if (this->norms == NULL ) {
	    this->xdim = 0;
	    this->ydim = 0;
	    fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
	}
    }
}


DepthMap::DepthMap(int xd, int yd, int useNorms, int granularity)
{
    this->xdim = xd;
    this->ydim = yd;
    
    this->origXdim = xd;
    this->origYdim = yd;
    
    this->elems = new DepthElement[xd*yd];
    if (this->elems == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	printf("DepthMap::new - couldn't allocate memory\n");
    }

    this->tags = new uchar[xd*yd];
    if (this->tags == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	printf("DepthMap::new - couldn't allocate memory\n");
    }

    this->edgeSteps = new signed char[xd*yd];
    if (this->edgeSteps == NULL) {
	this->xdim = 0;
	this->ydim = 0;
	fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
    }

    if (useNorms) {
	this->norms = new DepthMapNorm[xd*yd];
	if (this->norms == NULL ) {
	    this->xdim = 0;
	    this->ydim = 0;
	    fprintf(stderr, "DepthMap::new - couldn't allocate memory\n");
	}
    }

    this->initTree(granularity);
}


void
DepthMap::initTree(int granularity)
{
    this->treeGranularity = granularity;
    int numLeaves = int(ceil(float(this->xdim)/granularity));

    int temp = 1;
    int depth = 1;
    while (temp < numLeaves) {
	temp = temp << 1;
	depth++;
    }
    
    //this->treeDepth = int(ceil(log(numLeaves)/log(2)))+1;    
    this->treeDepth = depth;
    this->origTreeDepth = depth;

    this->treeElems = new DepthTreeElement**[this->ydim];
    this->holeTreeElems = new DepthTreeElement**[this->ydim];
    for (int yy = 0; yy < this->ydim; yy++) {
	this->treeElems[yy] = new DepthTreeElement*[this->treeDepth];
	this->holeTreeElems[yy] = new DepthTreeElement*[this->treeDepth];
	int numElems = 1;
	for (int j = 0; j < this->treeDepth; j++) {
	    this->treeElems[yy][j] = new DepthTreeElement[numElems];
	    this->holeTreeElems[yy][j] = new DepthTreeElement[numElems];
	    for (int k = 0; k < numElems; k++) {
		this->treeElems[yy][j][k].minZ = FLT_MAX;
		this->treeElems[yy][j][k].maxZ = -FLT_MAX;
		this->holeTreeElems[yy][j][k].minZ = FLT_MAX;
		this->holeTreeElems[yy][j][k].maxZ = -FLT_MAX;
	    }
	    numElems *= 2;
	}
    }
    
    this->leafIndices = new int[2*numLeaves];
    this->runs = new int[4*numLeaves];

}

void
DepthMap:: updateNx()
{
    for (int i = 0; i < this->xdim*this->ydim; i++) {
	this->norms[i].nx = (signed char)(127*this->elems[i].conf);
    }
}

void
DepthMap:: updateNy()
{
    for (int i = 0; i < this->xdim*this->ydim; i++) {
	this->norms[i].ny = (signed char)(127*this->elems[i].conf);
    }
}

void
DepthMap:: updateNz()
{
    for (int i = 0; i < this->xdim*this->ydim; i++) {
	this->norms[i].nz = (signed char)(127*this->elems[i].conf);
    }
}

void
DepthMap:: updateEdgeSteps()
{
    for (int i = 0; i < this->xdim*this->ydim; i++) {
	this->edgeSteps[i] = (signed char)(127*this->elems[i].conf+0.5);
    }
}


//
// Nomenclature for triangles when tagging
//  
//  z3 ----- z4
//  |       /|
//  |      / |
//  |  4  /  |
//  |    /   |
//  |   /    |
//  |  /  3  |
//  | /      |
//  z1 ----- z2
//
//
//  
//  z3 ----- z4
//  |\       |
//  | \   2  |
//  |  \     |
//  |   \    |
//  |    \   |
//  |  1  \  |
//  |      \ |
//  z1 ----- z2
//

void
DepthMap::tagCellsForResampling()
{
   float z1, z2, z3, z4, minZ, maxZ;

   for (int yy = 0; yy < this->ydim-1; yy++) {
      for (int xx = 0; xx < this->xdim-1; xx++) {
	 DepthElement *depthBuf = this->elems + xx + yy*xdim;
	 uchar *tagBuf = this->tags + xx + yy*xdim;
	 z1 = depthBuf->z;
	 z2 = (depthBuf+1)->z;
	 z3 = (depthBuf+xdim)->z;
	 z4 = (depthBuf+xdim+1)->z;

	 *tagBuf = 0;

	 int valid_count = IS_VALID_DEPTH(z1) + IS_VALID_DEPTH(z2) +
	    IS_VALID_DEPTH(z3) + IS_VALID_DEPTH(z4);

	 if (valid_count < 3) {	    
	    continue;
	 }

	 if (valid_count == 3) {
	    if (!IS_VALID_DEPTH(z4)) {
	       maxZ = MAX(MAX(z1,z2),z3);
	       minZ = MIN(MIN(z1,z2),z3);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_ONE;
	       }
	    }
	    else if (!IS_VALID_DEPTH(z1)) {
	       maxZ = MAX(MAX(z4,z2),z3);
	       minZ = MIN(MIN(z4,z2),z3);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_TWO;
	       }
	    }
	    else if (!IS_VALID_DEPTH(z3)) {
	       maxZ = MAX(MAX(z1,z2),z4);
	       minZ = MIN(MIN(z1,z2),z4);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_THREE;
	       }
	    }
	    else if (!IS_VALID_DEPTH(z2)) {
	       maxZ = MAX(MAX(z1,z3),z4);
	       minZ = MIN(MIN(z1,z3),z4);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_FOUR;
	       }
	    }
	 }
	 else {
	    if (fabs(z1-z4) > fabs(z2-z3)) {
	       maxZ = MAX(MAX(z1,z2),z3);
	       minZ = MIN(MIN(z1,z2),z3);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_ONE;
	       }
	       maxZ = MAX(MAX(z4,z2),z3);
	       minZ = MIN(MIN(z4,z2),z3);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf |= DepthMap::TRIANGLE_TWO;
	       }
	    } else {
	       maxZ = MAX(MAX(z1,z2),z4);
	       minZ = MIN(MIN(z1,z2),z4);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf = DepthMap::TRIANGLE_THREE;
	       }

	       maxZ = MAX(MAX(z1,z3),z4);
	       minZ = MIN(MIN(z1,z3),z4);
	       if (maxZ - minZ < MAX_DEPTH_DIFFERENCE) {
		  *tagBuf |= DepthMap::TRIANGLE_FOUR;
	       }
	    }
	 }
      }
   }
}


void
DepthMap:: normalize()
{
    float nx, ny, nz, length;

    for (int i = 0; i < this->xdim*this->ydim; i++) {
	nx = (float)this->norms[i].nx;
	ny = (float)this->norms[i].ny;
	nz = (float)this->norms[i].nz;
	length = sqrt(nx*nx+ny*ny+nz*nz);
	if (length != 0) {
	    nx /= length;
	    ny /= length;
	    nz /= length;
	} else {
	    nx = ny = nz = 0;
	}

	this->norms[i].nx = (signed char)(nx);
	this->norms[i].ny = (signed char)(ny);
	this->norms[i].nz = (signed char)(nz);
    }
}

int
DepthMap::reuse(int xd, int yd)
{
    this->xdim = xd;
    this->ydim = yd;

    if (this->xdim > this->origXdim || this->ydim > this->origYdim) {
       return 0;
    }

    int numLeaves = int(ceil(float(this->xdim)/this->treeGranularity));

    int temp = 1;
    int depth = 1;
    while (temp < numLeaves) {
	temp = temp << 1;
	depth++;
    }
    
    this->treeDepth = depth;

    for (int yy = 0; yy < this->ydim; yy++) {
	int numElems = 1;
	for (int j = 0; j < this->treeDepth; j++) {
	    for (int k = 0; k < numElems; k++) {
		this->treeElems[yy][j][k].minZ = FLT_MAX;
		this->treeElems[yy][j][k].maxZ = -FLT_MAX;
		this->holeTreeElems[yy][j][k].minZ = FLT_MAX;
		this->holeTreeElems[yy][j][k].maxZ = -FLT_MAX;
	    }
	    numElems *= 2;
	}
    }
}


void
DepthMap::fillTree(int numLines)
{
    DepthTreeElement *leaf, *holeLeaf;
    int yoffmin, yoffmax, xoffmax;
    int yy;

    for (yy = 0; yy < this->ydim; yy++) {

	if (IS_EVEN(numLines)) {
	    yoffmax = numLines/2;
	    yoffmin = -numLines/2 + 1;
	}  else {
	    yoffmax = (numLines+1)/2 - 1;
	    //  Used to be "yoffmin = -yoffmin;"  Fix indicated by Huber 4/5/01
	    yoffmin = -yoffmax;
	}

	if (yy + yoffmax >= this->ydim) {
	    yoffmax = this->ydim - yy - 1;
	}

	if (yy + yoffmin < 0) {
	    yoffmin = -yy;
	}

	for (int yoff = yoffmin; yoff <= yoffmax; yoff++) {
	    leaf = this->treeElems[yy][this->treeDepth-1];
	    holeLeaf = this->holeTreeElems[yy][this->treeDepth-1];
	    for (int xx = 0; xx < this->xdim; 
			     xx+=this->treeGranularity, leaf++, holeLeaf++) {
		if (xx <= this->xdim-this->treeGranularity)
		    xoffmax = this->treeGranularity;
		else
		    xoffmax = xx % this->treeGranularity;

		for (int xoff = 0; xoff < xoffmax; xoff++) {
		    float depth = this->elems[(yy+yoff)*this->xdim+ xx+xoff].z;
		    if (IS_VALID_DEPTH(depth)) {
			leaf->minZ = MIN(depth, leaf->minZ);
			leaf->maxZ = MAX(depth, leaf->maxZ);

			holeLeaf->minZ = MIN(depth, holeLeaf->minZ);
			holeLeaf->maxZ = MAX(depth, holeLeaf->maxZ);
		    } else {
			holeLeaf->minZ = -FLT_MAX;
			holeLeaf->maxZ = FLT_MAX;
		    }
		}
	    }
	}
    }

    for (yy = 0; yy < this->ydim; yy++) {
	holeLeaf = this->holeTreeElems[yy][this->treeDepth-1];
	for (int xx = 0; xx < this->xdim; 
		     xx+=this->treeGranularity, holeLeaf++) {
	    if (holeLeaf->minZ == -FLT_MAX) {
		holeLeaf->minZ = FLT_MAX;
	    }
	}
	this->pullUpTree(yy, this->treeDepth - 1);
    }
}


void
DepthMap::pullUpTree(int y, int level) 
{
    if (level != 0) {
	int numNodes = ROUND_INT(pow(2,level-1));
	DepthTreeElement **tree = this->treeElems[y];
	DepthTreeElement **holeTree = this->holeTreeElems[y];
	for (int i = 0; i < numNodes; i++) {
	    tree[level-1][i].minZ = 
		MIN(tree[level][2*i].minZ, tree[level][2*i+1].minZ);
	    tree[level-1][i].maxZ = 
		MAX(tree[level][2*i].maxZ, tree[level][2*i+1].maxZ);

	    holeTree[level-1][i].minZ = 
		MIN(holeTree[level][2*i].minZ, holeTree[level][2*i+1].minZ);

	    holeTree[level-1][i].maxZ = 
		MAX(holeTree[level][2*i].maxZ, holeTree[level][2*i+1].maxZ);
	}
	this->pullUpTree(y, level - 1);
    }
}


int *
DepthMap::getDepthRuns(float y, float zmin, float zmax, int *numRuns)
{
    int numLeaves = 0;
    int yy = int(y);
    this->traverseTree(yy, 0, 0, zmin, zmax, leafIndices, &numLeaves);
    this->consolidateRuns(leafIndices, numLeaves, numRuns);
    return this->runs;
}


void
DepthMap::traverseTree(int y, int level, int off, float zmin, float zmax,
		       int *leafIndices, int *numLeaves)
{
    DepthTreeElement *node = &this->treeElems[y][level][off];
    if (level < this->treeDepth - 1) {
	if (zmax >= node->minZ && zmin <= node->maxZ) {
	    this->traverseTree(y, level+1, 2*off, 
			       zmin, zmax, leafIndices, numLeaves);
	    this->traverseTree(y, level+1, 2*off+1,
			       zmin, zmax, leafIndices, numLeaves);
	}
	else {
	    return;
	}
    } 
    else {
	if (zmax >= node->minZ && zmin <= node->maxZ) {
	    leafIndices[*numLeaves] = off;
	    *numLeaves = *numLeaves + 1;
	    return;
	}
	else
	    return;
    }
}


int *
DepthMap::getDepthRunsUpperBound(float y, float zmax, int *numRuns)
{
    int numLeaves = 0;
    int yy = int(y);
    this->traverseTreeUpperBound(yy, 0, 0, zmax, leafIndices, &numLeaves);
    this->consolidateRuns(leafIndices, numLeaves, numRuns);
    return this->runs;
}


int *
DepthMap::getDepthRunsEdges(float y, float zmax, int *numRuns)
{
    int numLeaves = 0;
    int yy = int(y);
    this->traverseTreeEdges(yy, 0, 0, zmax, leafIndices, &numLeaves);
    this->consolidateRuns(leafIndices, numLeaves, numRuns);
    return this->runs;
}


void
DepthMap::traverseTreeUpperBound(int y, int level, int off, float zmax,
				 int *leafIndices, int *numLeaves)
{

    DepthTreeElement *node = &this->holeTreeElems[y][level][off];
    if (level < this->treeDepth - 1) {
	if (zmax > node->minZ) {
	    this->traverseTreeUpperBound(y, level+1, 2*off, 
					 zmax, leafIndices, numLeaves);
	    this->traverseTreeUpperBound(y, level+1, 2*off+1,
					 zmax, leafIndices, numLeaves);
	}
	else {
	    return;
	}
    } 
    else {
	if (zmax > node->maxZ && node->minZ != FLT_MAX) {
	    leafIndices[*numLeaves] = off;
	    *numLeaves = *numLeaves + 1;
	    return;
	}
	else
	    return;
    }
}


void
DepthMap::traverseTreeEdges(int y, int level, int off, float zmax,
			    int *leafIndices, int *numLeaves)
{
    DepthTreeElement *node = &this->treeElems[y][level][off];
    if (level < this->treeDepth - 1) {
	if (zmax > node->minZ) {
	    this->traverseTreeEdges(y, level+1, 2*off, 
				    zmax, leafIndices, numLeaves);
	    this->traverseTreeEdges(y, level+1, 2*off+1,
				    zmax, leafIndices, numLeaves);
	}
	else {
	    return;
	}
    } 
    else {
	DepthTreeElement *holeNode = &this->holeTreeElems[y][level][off];
	if (zmax > node->maxZ && zmax < holeNode->maxZ) {
	    leafIndices[*numLeaves] = off;
	    *numLeaves = *numLeaves + 1;
	    return;
	}
	else
	    return;
    }
}


void
DepthMap::consolidateRuns(int *leafIndices, int numLeaves, int *numRuns)
{
    if (numLeaves == 0) {
	*numRuns = 0;
	return;
    }

    this->runs[0] = leafIndices[0]*this->treeGranularity;
    this->runs[1] = this->runs[0]+this->treeGranularity-1;
    *numRuns = 1;
    for (int i = 1; i < numLeaves; i++) {
	if (leafIndices[i] == leafIndices[i-1]+1) {
	    this->runs[2*(*numRuns-1)+1] += this->treeGranularity;
	}
	else {
	    this->runs[2*(*numRuns)] = leafIndices[i]*this->treeGranularity;
	    this->runs[2*(*numRuns)+1] = this->runs[2*(*numRuns)]
		+this->treeGranularity-1;
	    *numRuns = *numRuns + 1;
	}
    }
    
    if (this->runs[2*(*numRuns - 1)+1] > this->xdim - 1)
	this->runs[2*(*numRuns - 1)+1] = this->xdim - 1;
}


DepthMap::~DepthMap()
{
    if (this->elems != NULL) {
	delete [] this->elems;
	this->elems = NULL;
    }

    if (this->treeElems != NULL) {
	for (int yy = 0; yy < this->origYdim; yy++) {
	    for (int j = 0; j < this->origTreeDepth; j++) {
		delete [] this->treeElems[yy][j];
	    }
	    delete [] this->treeElems[yy];
	}
	delete [] this->treeElems;
	this->treeElems = NULL;
	delete [] this->leafIndices;
	delete [] this->runs;
    }

    if (this->holeTreeElems != NULL) {
	for (int yy = 0; yy < this->origYdim; yy++) {
	    for (int j = 0; j < this->origTreeDepth; j++) {
		delete [] this->holeTreeElems[yy][j];
	    }
	    delete [] this->holeTreeElems[yy];
	}
	delete [] this->holeTreeElems;
	this->holeTreeElems = NULL;
    }
}


int 
DepthMap::writePly(const char *filename, float noiseLevel)
{
    float version;
    int xx, yy, index;
    int numRangeGridPoints;
    float noiseOffset;

    numRangeGridPoints = this->xdim*this->ydim;
    int numVerts = this->xdim*this->ydim;

    PlyFile *ply = ply_open_for_writing(filename, 2, elem_names, 
					PLY_BINARY_BE, &version);

    int nvp = 0;
    vert_props[nvp].offset = voffset(x); nvp++;
    vert_props[nvp].offset = voffset(y); nvp++;
    vert_props[nvp].offset = voffset(z); nvp++;

    int *grid = new int[numRangeGridPoints];
    int *pGrid = grid;
    index = 0;
    DepthElement *buf = this->elems;
    float minZ = FLT_MAX;
    for (yy = 0; yy < this->ydim; yy++) {
	for (xx = 0; xx < this->xdim; xx++, buf++, pGrid++) {
	    if (IS_VALID_DEPTH(buf->z)) {
	       if (noiseLevel != 0) {
		  noiseOffset = 2*(rand()/32767.0) - 1;
		  noiseOffset = noiseOffset*noiseOffset*noiseLevel;
		  buf->z += noiseOffset;
	       }
	       *pGrid = index;
	       index++;
	    } else {
	       *pGrid = -1;
	    }
	}
    }    

    ply_describe_element (ply, "vertex", index, 
			  nvp, vert_props);

    range_grid_props[0].offset = roffset(verts);
    range_grid_props[0].count_offset = roffset(nverts);  /* count offset */
    ply_describe_element (ply, "range_grid", numRangeGridPoints, 1, 
			  range_grid_props);

    char objInfo[PATH_MAX];

    sprintf(objInfo, "num_cols %d", this->xdim);
    ply_put_obj_info (ply, objInfo);

    sprintf(objInfo, "num_rows %d", this->ydim);
    ply_put_obj_info (ply, objInfo);

    sprintf(objInfo, "is_mesh 0");
    ply_put_obj_info (ply, objInfo);

    ply_header_complete (ply);
    
    /* set up and write the vertex elements */
    Vec3f vin, vout;
    PlyVertex vert;
    float res = this->resolution;
    float ypos = this->origin[1];
    buf = this->elems;
    ply_put_element_setup (ply, "vertex");
    for (yy = 0; yy < this->ydim; yy++, ypos+=res) {
       float xpos = this->origin[0];
       for (xx = 0; xx < this->xdim; xx++, buf++, xpos+=res) {
	  if (IS_VALID_DEPTH(buf->z)) {
	     if (this->linePersp) {
		vin.setValue(xpos, ypos, buf->z);
		applyInvLinePersp(vin,vout);
		vert.x = vout.x;
		vert.y = vout.y;
		vert.z = vout.z;	     
	     }
	     else if (this->perspective) {
		vin.setValue(xpos, ypos, buf->z);
		applyInvPersp(vin,vout);
		vert.x = vout.x;
		vert.y = vout.y;
		vert.z = vout.z;	     
	     }
	     else {
		vert.x = xpos;
		vert.y = ypos;
		vert.z = buf->z;	     
	     }
	     ply_put_element (ply, (void *) &vert);
	  }
       }
    } 

    /* set up and write the vertex elements */
    RangeGridList gridElem;
    gridElem.verts = new int[1];
    pGrid = grid;
    ply_put_element_setup (ply, "range_grid");
    for (yy = 0; yy < this->ydim; yy++) {
       for (xx = 0; xx < this->xdim; xx++, pGrid++) {
	  if (*pGrid < 0) {
	     gridElem.nverts = 0;
	  } else {
	     gridElem.nverts = 1;
	     gridElem.verts[0] = *pGrid;
	  }
	  ply_put_element (ply, (void *) &gridElem);
       }
    } 
    delete gridElem.verts;

    /* close the PLY file */
    ply_close (ply);    

    delete grid;

    return 1;    
}


/*
int 
DepthMap::writePly(char *filename)
{
    float version;
    int yy;

    int numVerts = this->xdim*this->ydim;

    PlyFile *ply = ply_open_for_writing(filename, 1, elem_names, 
					PLY_BINARY_BE, &version);

    int nvp = 0;
    vert_props[nvp].offset = voffset(x); nvp++;
    vert_props[nvp].offset = voffset(y); nvp++;
    vert_props[nvp].offset = voffset(z); nvp++;

    ply_describe_element (ply, "vertex", numVerts, 
			  nvp, vert_props);

    ply_header_complete (ply);
    
    // set up and write the vertex elements
    DepthElement *buf = this->elems;
    float minZ = FLT_MAX;
    for (yy = 0; yy < this->ydim; yy++) {
	for (int xx = 0; xx < this->xdim; xx++, buf++) {
	    if (IS_VALID_DEPTH(buf->z)) {
		minZ = MIN(buf->z, minZ);
	    }
	}
    }    

    PlyVertex vert;
    float res = this->resolution;
    ply_put_element_setup (ply, "vertex");
    float ypos = this->origin[1];
    buf = this->elems;
    for (yy = 0; yy < this->ydim; yy++, ypos+=res) {
	float xpos = this->origin[0];
	for (int xx = 0; xx < this->xdim; xx++, xpos+=res, buf++) {
	    vert.x = xpos;
	    vert.y = ypos;
	    if (IS_VALID_DEPTH(buf->z)) {
		vert.z = buf->z;
	    }
	    else {
		vert.z = minZ;
	    }
		    
	    ply_put_element (ply, (void *) &vert);
	}
    }    

    // close the PLY file
    ply_close (ply);    

    return 1;    
}

*/
