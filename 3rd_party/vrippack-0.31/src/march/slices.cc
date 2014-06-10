/*

Name:         slices.cc

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Processes all selected slices.

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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "mc.h"
#include <unistd.h>
#include "ply.h"
#include "OccGridRLE.h"
#include "SectionRLE.h"
#include "mc_more.h"
#include <assert.h>

//#define USE_VALUE_WEIGHT_PRODUCT

#define GET_MORE_CUBE_TRIS

struct PlyVertex {
    float x, y, z;
    float nx, ny, nz;
    float confidence;
    uchar realData;
};

struct PlyTri {
    unsigned char nverts;
    int *verts;
};

static char *elem_names[] = { 
  "vertex", "face",
};

static PlyProperty vert_props[] =  {
    {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"confidence", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"real_data", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"nx", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"ny", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"nz", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
};

static PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};


int loadCube(Cube cube, SectionRLE **section, int index, int xx, int yy);
void computeGradients(SectionRLE **section, int level);
static void UpdateGeometry(int j, int k, SectionRLE **rleSection, 
			   TriangleVertex *vertexList, uchar EdgeTableIndex);
static void writePlyFile(char *filename, float tx, float ty, float tz);
static Vertex *findVertex(TriangleVertex *vert, TriSet *set);
static void loadSection(SectionRLE *rleSection, 
			OccGridRLE *occGrid, int sliceNum);
static void copyElement(Point *sect, SectionElement *elem);
static Vertex *addVertex(TriangleVertex *inVert, int j, int k, 
			 SectionRLE **rleSection);
int loadCubeAllowPartiallyValid(Cube cube, SectionRLE **section, 
				int index, int xx, int yy);


static int numVerts;
static int numTris;
static ChunkAllocator *triChunker;
static ChunkAllocator *vertChunker;


void 
DoSlicesOccRLE()
{
  SectionRLE *rleSection[5], *temp;
  int i,j,k; /* indices within sections - relative to i0,j_0,k0 */
  int slices,rows,cols; /* dimensions of stack of sections */
  int iter;
  Cube cube;
  unsigned char EdgeTableIndex = 0x00;
  int xx, yy, zz, ii;
  int runType;
  RunLength length;
  SectionElement *element;    

  numVerts = 0;
  numTris = 0;

  triChunker = new ChunkAllocator(CHUNK_SIZE);
  vertChunker = new ChunkAllocator(CHUNK_SIZE);

 

  /* 
   * allocate memory for four sections
   */
  slices = i1 - i0 + 3;
  rows = j_1 - j_0 + 3;
  cols = k1 - k0 + 3;

  if (UseValueWeightProduct) {
     signal(SIGFPE, SIG_IGN);
  }

  for (i=0;i<5;i++)
    rleSection[i] = new SectionRLE(rows,cols,CHUNK_SIZE);

  /* 
   * read initial three slices (into section[1-3])
   */
  for (iter=0;iter<3;iter++) {
    /*
     * read appropriate regions of slice file 
     */
      loadSection(rleSection[iter+1], occGrid, FirstSliceFileNumber+i0-1+iter);
  }


  /* 
   * compute normals in section[2] 
   */

  computeGradients(rleSection, 2);

  /* 
   * Main Loop - Read New Slice
   */

  fflush(stdout);

  for (i=1;i<slices-2;i++) {

    /* rotate section memories to make room for new slice */
      temp = rleSection[0];
      rleSection[0] = rleSection[1];
      rleSection[1] = rleSection[2];
      rleSection[2] = rleSection[3];
      rleSection[3] = temp;

      /*
       * read appropriate regions of slice file 
       */
      
      loadSection(rleSection[3], occGrid, FirstSliceFileNumber+i0+i+1);      
      
      computeGradients(rleSection, 2);
      
      /* Process the cubes in raster order */
      for (yy = 1; yy < rleSection[1]->ydim-2; yy++) {
	  rleSection[1]->setScanline(yy);
	  xx = 0;
	  while (TRUE) {

	      length = rleSection[1]->getNextLength();
	      runType = rleSection[1]->getRunType(&length);

	      if (runType == SectionRLE::END_OF_RUN)
		  break;
	  
	      if (runType == SectionRLE::CONSTANT_DATA) {
		  xx += length;
	      }
	      else {
		  for (ii=0; ii<length; ii++, xx++) {
		      if (xx == 0 || xx >= rleSection[2]->xdim-2)
			  continue;

#ifdef GET_MORE_CUBE_TRIS
		      if (!loadCubeAllowPartiallyValid(cube, rleSection, 
						       1, xx, yy))
			  continue;
#else
		      if (!loadCube(cube, rleSection, 1, xx, yy))
		          continue;
#endif

		      /* dispatch to DoCube with absolute i,j,k position */
		      TriangleVertex *vertexList = 
			  NewDoCube(cube,i0+i,j_0+xx,k0+yy,&EdgeTableIndex);
		  
		      if (TheEdgeTable[EdgeTableIndex].Ntriangles > 0)
			  UpdateGeometry(xx, yy, rleSection, vertexList, 
				     EdgeTableIndex);
		  }
	      }
	  }
      }
      printf("\rProcess slice %d of %d.  Triangle count = %d", 
	     i+3, slices+1, TotalTriangles);
      fflush(stdout);
      
  } /* for (i) */

  printf("\n\n");
  fflush(stdout);


  printf("Writing out triangles...\n");
  writePlyFile(outfile, 
	       occGrid->origin[0] - occGrid->resolution, 
	       occGrid->origin[1] - occGrid->resolution, 
	       occGrid->origin[2] - occGrid->resolution);

  printf("Done.\n\n");

} /* DoSlices */


static void
UpdateGeometry(int j, int k, SectionRLE **rleSection, 
	       TriangleVertex *vertexList, uchar EdgeTableIndex)
{
    int n;
    Triple triangle;
    Tri *tri;
    TriSet *set;

    set = &rleSection[2]->getElement(j,k)->set;
    set->ntris = 0;
    TriangleVertex *v0, *v1, *v2;
    for (n=0;n<TheEdgeTable[EdgeTableIndex].Ntriangles;n++) {
	triangle = (TheEdgeTable[EdgeTableIndex].TriangleList)[n];

	v0 = &vertexList[triangle.A];
	v1 = &vertexList[triangle.B];
	v2 = &vertexList[triangle.C];

#ifdef GET_MORE_CUBE_TRIS
	if (!v0->valid || !v1->valid || !v2->valid) {
	   continue;
	}
#endif

	// Don't add degenerate triangles
	if (((v0->x == v1->x) && (v0->y == v1->y) && (v0->z == v1->z)) ||
	    ((v0->x == v2->x) && (v0->y == v2->y) && (v0->z == v2->z)) ||
	    ((v2->x == v1->x) && (v2->y == v1->y) && (v2->z == v1->z))) {

	    continue;
	}
	
	tri = (Tri *)triChunker->alloc(sizeof(Tri));
	tri->verts[0] = addVertex(v0,j,k,rleSection);
	tri->verts[1] = addVertex(v1,j,k,rleSection);
	tri->verts[2] = addVertex(v2,j,k,rleSection);
	numTris++;

	set->tris[set->ntris] = tri;
	set->ntris++;
    }
}


static Vertex *
addVertex(TriangleVertex *inVert, int j, int k, SectionRLE **rleSection)
{
    Vertex *outVert, *newVert, *oldVert, temp;
    TriSet *set;
    SectionElement *elem;

    // We should only need to go to (j,k), not (j+1,k+1), but
    // since we're dropping triangles with low confidence, we
    // need to account for the possiblity that vertices below and
    // behind may be dropped - really should just need to consider
    // the (j+1,k+1) in the slice below...

    for (int zz = 1; zz <= 2; zz++) {
	for (int yy = k-1; yy <= k+1; yy++) {
	    for (int xx = j-1; xx <= j+1; xx++) {
		elem = rleSection[zz]->getElement(xx,yy);
		set = &elem->set;
		oldVert = findVertex(inVert, set);
		if (oldVert != NULL)
		    return oldVert;

	    }
	}
    }

    outVert = (Vertex *)vertChunker->alloc(sizeof(Vertex));
    outVert->x = inVert->x;
    outVert->y = inVert->y;
    outVert->z = inVert->z;
    outVert->nx = inVert->nx;
    outVert->ny = inVert->ny;
    outVert->nz = inVert->nz;
    outVert->confidence = inVert->confidence;
    outVert->realData = inVert->realData;
    outVert->index = numVerts;

    numVerts++;

    return outVert;
}


static Vertex*
findVertex(TriangleVertex *vert, TriSet *set)
{
    int i, j;
    Vertex *oldVert;

    for (i = 0; i < set->ntris; i++) {
	for (j = 0; j < 3; j++) {
	    oldVert = set->tris[i]->verts[j];
#if 0
            if (fabs(oldVert->x - vert->x) < CULL_TRIANGLE_FACTOR*dx &&
                fabs(oldVert->y - vert->y) < CULL_TRIANGLE_FACTOR*dy &&
                fabs(oldVert->z - vert->z) < CULL_TRIANGLE_FACTOR*dz)
		return oldVert;
#else
	    if (oldVert->x == vert->x &&
		oldVert->y == vert->y &&
		oldVert->z == vert->z)
		return oldVert;
#endif
	}
    }

    return NULL;
}




static void
loadSection(SectionRLE *rleSection, OccGridRLE *occGrid, int sliceNum)
{
    RunLength length;
    int runType, xx;
    SectionElement sectElement;
    OccElement *occElement;
    ushort totalWeight;

    rleSection->reset();

    for (int yy = 0; yy < occGrid->ydim; yy++) {
	occGrid->setScanline(yy, sliceNum);
	rleSection->allocNewRun(yy);
	xx = 0;
	while (TRUE) {
	    length = occGrid->getNextLength();
	    rleSection->putNextLength(length);
	    runType = rleSection->getRunType(&length);

	    if (runType == SectionRLE::END_OF_RUN) {
		break;
	    }

	    if (runType == SectionRLE::CONSTANT_DATA) {
		occElement = occGrid->getNextElement();

		// This line was not present, causing compiler
		// warnings up until 6/5/06.  (Brian Curless)
		totalWeight = occElement->totalWeight & 
		   ~OccGridRLE::FALSE_DATA_BIT;

		if (UseValueWeightProduct) {
		   if (totalWeight > 0) {
		      sectElement.density = (occElement->value/256.0 - 128)*
			 occElement->totalWeight + 128;
		   } else {
		      sectElement.density = occElement->value/256.0;
		   }
		}
		else {
		   sectElement.density = occElement->value/256.0;
		}

		sectElement.confidence = occElement->totalWeight/256.0;
	        sectElement.valid = FALSE;
	        sectElement.realData = TRUE;
		sectElement.set.ntris = 0;

		rleSection->putNextElement(&sectElement);
		xx += length;
	    }
	    else {
		for (int i = 0; i < length; i++) {
		    occElement = occGrid->getNextElement();
		    totalWeight = occElement->totalWeight & 
			~OccGridRLE::FALSE_DATA_BIT;
		    if (UseValueWeightProduct) {
		       if (totalWeight > 0) {
			  sectElement.density = (occElement->value/256.0 - 128)
			     *occElement->totalWeight + 128;
		       } else {
			  sectElement.density = occElement->value/256.0;
		       }
		    }
		    else {
		       sectElement.density = occElement->value/256.0;
		    }
		    sectElement.confidence = totalWeight/256.0;
		    sectElement.valid = 
			totalWeight > OCC_CONF_THRESHOLD;
		    sectElement.realData = 
			(occElement->totalWeight & 
			  OccGridRLE::FALSE_DATA_BIT) == 0;

		    sectElement.set.ntris = 0;
		    rleSection->putNextElement(&sectElement);
		    xx++;
		}
	    }
	}
    }
}


static void
copyElement(Point *sect, SectionElement *elem)
{
    sect->density = elem->density;
    sect->nx = elem->nx;
    sect->ny = elem->ny;
    sect->nz = elem->nz;
    sect->valid = elem->valid;
    sect->realData = elem->realData;
    sect->confidence = elem->confidence;
}


void
computeGradients(SectionRLE **section, int level)
{
   int yy, xx, i;
   int runType;
   RunLength length;
   SectionElement *element, *elem1, *elem2;    

   for (yy = 1; yy < section[level]->ydim-1; yy++) {
      section[level]->setScanline(yy);
      xx = 0;
      while (TRUE) {

	  length = section[level]->getNextLength();
	  runType = section[level]->getRunType(&length);

	  if (runType == SectionRLE::END_OF_RUN)
	      break;
	  
	  if (runType == SectionRLE::CONSTANT_DATA) {
	      element = section[level]->getNextElement();
	      xx += length;
	  }
	  else {
	      for (i=0; i<length; i++, xx++) {
		  element = section[level]->getNextElement();
		  if (xx == 0 || xx == section[level]->xdim-1)
		      continue;
		  element->nx = 
		     -(section[level+1]->getElement(xx, yy)->density 
		       - section[level-1]->getElement(xx, yy)->density)/2.0/dx;
		  element->ny = 
		     (section[level]->getElement(xx, yy+1)->density 
		      - section[level]->getElement(xx, yy-1)->density)/2.0/dy;
		  element->nz = 
		     (section[level]->getElement(xx-1, yy)->density 
		      - section[level]->getElement(xx+1, yy)->density)/2.0/dz;
	      }
	  }
      }
  }
}


int
loadCube(Cube cube, SectionRLE **section, int index, int xx, int yy)
{
    copyElement(&cube[0], section[index]->getElement(xx+1,yy));
    if (!cube[0].valid) {
	return 0;
    }
		  
    copyElement(&cube[1], section[index]->getElement(xx+1,yy+1));
    if (!cube[1].valid)
	return 0;
		  
    copyElement(&cube[2], section[index]->getElement(xx,yy+1));
    if (!cube[2].valid)
	return 0;
		  
    copyElement(&cube[3], section[index]->getElement(xx,yy));
    if (!cube[3].valid)
	return 0;
    
    copyElement(&cube[4], section[index+1]->getElement(xx+1,yy));
    if (!cube[4].valid)
	return 0;
    
    copyElement(&cube[5], section[index+1]->getElement(xx+1,yy+1));
    if (!cube[5].valid)
	return 0;
    
    copyElement(&cube[6], section[index+1]->getElement(xx,yy+1));
    if (!cube[6].valid)
	return 0;
    
    copyElement(&cube[7], section[index+1]->getElement(xx,yy));
    if (!cube[7].valid)
	return 0;

    return 1;
}


int
loadCubeAllowPartiallyValid(Cube cube, SectionRLE **section, 
			    int index, int xx, int yy)
{
   uchar valid_count = 0;

   copyElement(&cube[0], section[index]->getElement(xx+1,yy));
   valid_count += cube[0].valid;
		  
   copyElement(&cube[1], section[index]->getElement(xx+1,yy+1));
   valid_count += cube[1].valid;
		  
   copyElement(&cube[2], section[index]->getElement(xx,yy+1));
   valid_count += cube[2].valid;
   
   copyElement(&cube[3], section[index]->getElement(xx,yy));
   valid_count += cube[3].valid;
   
   copyElement(&cube[4], section[index+1]->getElement(xx+1,yy));
   valid_count += cube[4].valid;
   
   // First early exit
   if (valid_count < 1)
      return 0;
      
   copyElement(&cube[5], section[index+1]->getElement(xx+1,yy+1));
   valid_count += cube[5].valid;
   
   if (valid_count < 2)
      return 0;

   copyElement(&cube[6], section[index+1]->getElement(xx,yy+1));
   valid_count += cube[6].valid;
   
   if (valid_count < 3)
      return 0;

   copyElement(&cube[7], section[index+1]->getElement(xx,yy));
   valid_count += cube[7].valid;

   if (valid_count < 4)
      return 0;
   else
      return 1;
}
		  

static void
writePlyFile(char *filename, float tx, float ty, float tz)
{
    int i, nvp;
    float version;
    PlyFile *ply;
    PlyVertex aVert;
    Vertex *vert;
    Tri *currentTri;
    PlyTri aTri;
    Vec3f norm;

    ply = ply_open_for_writing(filename, 2, elem_names, 
			       PLY_BINARY_BE, &version);

    nvp = 0;
    vert_props[nvp].offset = offsetof(PlyVertex, x); nvp++;
    vert_props[nvp].offset = offsetof(PlyVertex, y); nvp++;
    vert_props[nvp].offset = offsetof(PlyVertex, z); nvp++;


    vert_props[nvp].offset = offsetof(PlyVertex, confidence); nvp++;

    vert_props[nvp].offset = offsetof(PlyVertex, realData); nvp++;

    if (WriteNormals) {
	vert_props[nvp].offset = offsetof(PlyVertex, nx); nvp++;
	vert_props[nvp].offset = offsetof(PlyVertex, ny); nvp++;
	vert_props[nvp].offset = offsetof(PlyVertex, nz); nvp++;
    }


    face_props[0].offset = offsetof(PlyTri, verts);
    face_props[0].count_offset = offsetof(PlyTri, nverts);  /* count offset */
    
    ply_describe_element (ply, "vertex", numVerts, nvp, vert_props);

    ply_describe_element (ply, "face", numTris, 1, face_props);

    ply_header_complete (ply);
    
    /* set up and write the vertex elements */
    ply_put_element_setup (ply, "vertex");

    vertChunker->reset();
    for (i = 0; i < numVerts; i++) {
	vert = (Vertex *)vertChunker->nextElem(sizeof(Vertex));

	aVert.x = -vert->z + tx;
	aVert.y = vert->y + ty;
	aVert.z = -vert->x + tz;

	if (WriteNormals) {
	    norm.setValue(vert->nz, -vert->ny, vert->nx);
	    norm.normalize();
	    aVert.nx = norm.x;
	    aVert.ny = norm.y;
	    aVert.nz = norm.z;
	}

	aVert.confidence = vert->confidence;
	aVert.realData = vert->realData;
	
	ply_put_element (ply, (void *) &aVert);
    }

    /* set up and write the face elements */
    ply_put_element_setup (ply, "face");

    int v[3];
    aTri.nverts = 3;
    aTri.verts = (int *)v;
    triChunker->reset();
    for (i = 0; i < numTris; i++) {
	currentTri = (Tri *)triChunker->nextElem(sizeof(Tri));

	aTri.verts[0] = currentTri->verts[1]->index;
	aTri.verts[1] = currentTri->verts[0]->index;
	aTri.verts[2] = currentTri->verts[2]->index;

	ply_put_element (ply, (void *) &aTri);
    }
    
    /* close the PLY file */
    ply_close (ply);    
}
