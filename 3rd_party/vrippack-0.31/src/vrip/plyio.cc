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


#include <stdio.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>
#include <limits.h>

#define MAX_VERT_PROPS 20

#include "plyio.h"
#include "vripGlobals.h"
#include "rangePly.h"

void keepUsedVerts(Mesh *mesh, int &numVertsUsed, 
		   uchar *vertUsed, Triangle *tris);


struct PlyVertex {
    float x, y, z;
    float nx, ny, nz;
    uchar diff_r, diff_g, diff_b;
    float intensity;
    float std_dev;
    float confidence;
};


struct PlyFace {
    uchar nverts;
    int *verts;
};



static PlyProperty vert_prop_x =  
   {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_y =  
  {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_z =  
  {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_nx =  
   {"nx", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_ny =  
  {"ny", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_nz =  
  {"nz", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_intens =  
  {"intensity", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_std_dev =  
  {"std_dev", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_confidence =  
  {"confidence", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_r =  
  {"diffuse_red", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_g =  
  {"diffuse_green", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_b =  
  {"diffuse_blue", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_prop_r =  
  {"red", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_g =  
  {"green", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_b =  
  {"blue", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_props[MAX_VERT_PROPS];


static PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};


/* dummy variables and associated macros for computing field offsets */

static PlyVertex *vert_dummy;
#define voffset(field) ((char *) (&vert_dummy->field) - (char *) vert_dummy)
static PlyFace *face_dummy;
#define foffset(field) ((char *) (&face_dummy->field) - (char *) face_dummy)




//
// Read in data from the ply file
//

Mesh *
readPlyFile(const char *filename)
{
    int i, j;
    int nelems;
    char **elist;
    int file_type;
    float version;
    char *elem_name;
    int nprops, num_vert_props;
    int num_elems;
    PlyProperty **plist;

    face_props[0].offset = foffset(verts);
    face_props[0].count_offset = foffset(nverts);  /* count offset */
    
    PlyFile *ply = 
	ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);

    if (!ply)
	exit(1);

    int nvp = 0;

    if (ply_is_valid_property(ply, "vertex", vert_prop_x.name)) {
	vert_props[nvp] = vert_prop_x;
	vert_props[nvp].offset = voffset(x); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_y.name)) {
	vert_props[nvp] = vert_prop_y;
	vert_props[nvp].offset = voffset(y); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_z.name)) {
	vert_props[nvp] = vert_prop_z;
	vert_props[nvp].offset = voffset(z); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_nx.name)) {
	vert_props[nvp] = vert_prop_nx;
	vert_props[nvp].offset = voffset(nx); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_ny.name)) {
	vert_props[nvp] = vert_prop_ny;
	vert_props[nvp].offset = voffset(ny); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_nz.name)) {
	vert_props[nvp] = vert_prop_nz;
	vert_props[nvp].offset = voffset(nz); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_intens.name)) {
	vert_props[nvp] = vert_prop_intens;
	vert_props[nvp].offset = voffset(intensity); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_std_dev.name)) {
	vert_props[nvp] = vert_prop_std_dev;
	vert_props[nvp].offset = voffset(std_dev); nvp++;
    }
    
    int hasConfidence = 0;
    if (ply_is_valid_property(ply, "vertex", vert_prop_confidence.name)) {
	vert_props[nvp] = vert_prop_confidence;
	vert_props[nvp].offset = voffset(confidence); nvp++;
	hasConfidence = 1;
    }
    
    int hasDiffColor = 0;
    if (ply_is_valid_property(ply, "vertex", "diffuse_red") &&
	ply_is_valid_property(ply, "vertex", "diffuse_green") &&
	ply_is_valid_property(ply, "vertex", "diffuse_blue")) 
    {
	vert_props[nvp] = vert_prop_diff_r;
	vert_props[nvp].offset = voffset(diff_r); nvp++;
	vert_props[nvp] = vert_prop_diff_g;
	vert_props[nvp].offset = voffset(diff_g); nvp++;
	vert_props[nvp] = vert_prop_diff_b;
	vert_props[nvp].offset = voffset(diff_b); nvp++;
	hasDiffColor = 1;
    }
    
    int hasColor = 0;
    if (ply_is_valid_property(ply, "vertex", "red") &&
	ply_is_valid_property(ply, "vertex", "green") &&
	ply_is_valid_property(ply, "vertex", "blue")) 
    {
	vert_props[nvp] = vert_prop_r;
	vert_props[nvp].offset = voffset(diff_r); nvp++;
	vert_props[nvp] = vert_prop_g;
	vert_props[nvp].offset = voffset(diff_g); nvp++;
	vert_props[nvp] = vert_prop_b;
	vert_props[nvp].offset = voffset(diff_b); nvp++;
	hasColor = 1;
    }
    
    num_vert_props = nvp;

    Mesh *mesh = new Mesh;
    mesh->hasConfidence = hasConfidence;
    mesh->hasColor = hasColor || hasDiffColor;
    Vertex *vert;
    Triangle *tri;
    PlyVertex plyVert;
    PlyFace plyFace;

    for (i = 0; i < nelems; i++) {

	/* get the description of the first element */
	elem_name = elist[i];
	plist = ply_get_element_description 
	    (ply, elem_name, &num_elems, &nprops);
	
	/* if we're on vertex elements, read them in */
	if (equal_strings ("vertex", elem_name)) {
	    
	    mesh->numVerts = num_elems;
	    mesh->verts = new Vertex[mesh->numVerts];
	    
	    /* set up for getting vertex elements */
	    ply_get_element_setup (ply, elem_name, num_vert_props, vert_props);
	    
	    if (Verbose)
		printf("Reading vertices...\n");

	    /* grab all the vertex elements */
	    for (j = 0; j < mesh->numVerts; j++) {
		ply_get_element (ply, (void *) &plyVert);
		vert = &mesh->verts[j];
		vert->coord.x = plyVert.x;
		vert->coord.y = plyVert.y;
		vert->coord.z = plyVert.z;
		if (hasConfidence)
		    vert->confidence = plyVert.confidence;
		else 
		    vert->confidence = 1.0;
		if (hasDiffColor ||  hasColor) {
		   vert->red = plyVert.diff_r;
		   vert->green = plyVert.diff_g;
		   vert->blue = plyVert.diff_b;
		}
	    }
	    fflush(stdout);

	    if (Verbose)
		printf("Done.\n");

	}

	if (equal_strings ("face", elem_name)) {

	    ply_get_element_setup (ply, elem_name, 1, face_props);

	    mesh->numTris = num_elems;

	    if (mesh->numTris == 0)
		continue;

	    mesh->tris = new Triangle[mesh->numTris];

	    if (Verbose)
		printf("Reading Faces...\n");

	    for (j = 0; j < mesh->numTris; j++) {		
		ply_get_element (ply, (void *) &plyFace);
		tri = &mesh->tris[j];
		tri->vindex1 = plyFace.verts[0];
		tri->vindex2 = plyFace.verts[1];
		tri->vindex3 = plyFace.verts[2];
		free(plyFace.verts);
	    }

	    if (Verbose)
		printf("Done.\n");
	}
    }

    int num_obj_info, found_mirror;
    char **obj_info = ply_get_obj_info (ply, &num_obj_info);

    char temp[PATH_MAX];
    found_mirror = FALSE;
    mesh->isWarped = FALSE;
    mesh->isRightMirrorOpen = TRUE;
    for (i = 0; i < num_obj_info; i++) {
	if (strstr(obj_info[i], "is_right_mirror_open")) {
	    found_mirror = TRUE;
	    sscanf(obj_info[i], "%s%d", temp, &mesh->isRightMirrorOpen);
	}
	else if (strstr(obj_info[i], "is_warped")) {
	    sscanf(obj_info[i], "%s%d", temp, &mesh->isWarped);
	}
    }

    if (mesh->isWarped && !found_mirror)
	printf("Couldn't tell which mirror was open.  Right mirror assumed.\n");

    ply_close(ply);
    return mesh;
}


int
writePlyFile(const char *filename, Mesh *mesh)
{
    int i, j;
    int nelems;
    char **elist;
    int file_type;
    float version;
    char *elem_name;
    int nprops, num_vert_props;
    int num_elems;
    PlyProperty **plist;
    int hasIntensity;
    int hasColor;
    int hasConfidence;
    int nvp;
    char *elem_names[] = {"vertex", "face"};
    PlyFile *ply;

   if (filename == NULL) {
       ply = ply_write(stdout, 2, elem_names, PLY_BINARY_BE);
    } else {
       ply = ply_open_for_writing(filename, 2, elem_names, 
                            PLY_BINARY_BE, &version);
    }

    if (ply == NULL)
        return 0;

    int numVertsUsed;
    uchar *vertUsed = new uchar[mesh->numVerts];
    Triangle *tris = new Triangle[mesh->numTris];
    keepUsedVerts(mesh, numVertsUsed, vertUsed, tris);

    nvp = 0;

    vert_props[nvp] = vert_prop_x;
    vert_props[nvp].offset = offsetof(PlyVertex,x); nvp++;
    vert_props[nvp] = vert_prop_y;
    vert_props[nvp].offset = offsetof(PlyVertex,y); nvp++;
    vert_props[nvp] = vert_prop_z;
    vert_props[nvp].offset = offsetof(PlyVertex,z); nvp++;

    if (mesh->hasConfidence) {
       vert_props[nvp] = vert_prop_confidence;
       vert_props[nvp].offset = offsetof(PlyVertex, confidence); nvp++;
    }

    if (mesh->hasColor) {
	vert_props[nvp] = vert_prop_diff_r;
	vert_props[nvp].offset = voffset(diff_r); nvp++;
	vert_props[nvp] = vert_prop_diff_g;
	vert_props[nvp].offset = voffset(diff_g); nvp++;
	vert_props[nvp] = vert_prop_diff_b;
	vert_props[nvp].offset = voffset(diff_b); nvp++;
    }

    num_vert_props = nvp;

    face_props[0].offset = offsetof(PlyFace, verts);
    face_props[0].count_offset = offsetof(PlyFace, nverts);  /* count offset */
    
    ply_describe_element (ply, "vertex", numVertsUsed, 
			  num_vert_props, vert_props);

    ply_describe_element (ply, "face", mesh->numTris, 1, face_props);

    ply_header_complete (ply);
    


    /* set up and write the vertex elements */
    PlyVertex plyVert;
    Vertex *vert;

    ply_put_element_setup (ply, "vertex");

     for (i = 0; i < mesh->numVerts; i++) {
       if (!vertUsed[i])
         continue;

	vert = &mesh->verts[i];
	plyVert.x = vert->coord.x;
	plyVert.y = vert->coord.y;
	plyVert.z = vert->coord.z;

	if (mesh->hasConfidence)
	   plyVert.confidence = vert->confidence;


	if (mesh->hasColor) {
	   plyVert.diff_r = vert->red;
	   plyVert.diff_g = vert->green;
	   plyVert.diff_b = vert->blue;
	}
	    
	ply_put_element (ply, (void *) &plyVert);
    }

    PlyFace plyFace;
    Triangle *tri;
    int vertIndices[3];

    ply_put_element_setup (ply, "face");

    for (i = 0; i < mesh->numTris; i++) {
	tri = &tris[i];
	plyFace.nverts = 3;
	vertIndices[0] = tri->vindex1;
	vertIndices[1] = tri->vindex2;
	vertIndices[2] = tri->vindex3;
	plyFace.verts = (int *)vertIndices;

	ply_put_element (ply, (void *) &plyFace);
    }
    
    /* close the PLY file */
    ply_close (ply);    

    delete [] vertUsed;
    delete [] tris;

    return 1;
}




Mesh *
readMeshFromPly(const char *filename, int fillGaps, int extendEdges)
{
    int i;
    Mesh *mesh;

    if (is_range_grid_file(filename)) {
	RangeGrid *rangeGrid;
	if (fillGaps) {
	    rangeGrid = readRangeGridFillGaps(filename);
	    if (rangeGrid == NULL) {
		return NULL;
	    }
	    mesh = meshFromGrid(rangeGrid, MeshResolution, TRUE);
	} else if (extendEdges) {
	    rangeGrid = readRangeGridExtendEdges(filename);
	    if (rangeGrid == NULL) {
		return NULL;
	    }
	    mesh = meshFromGrid(rangeGrid, MeshResolution, FALSE);
	} else {
	    rangeGrid = readRangeGrid(filename);
	    if (rangeGrid == NULL) {
		return NULL;
	    }
	    mesh = meshFromGrid(rangeGrid, MeshResolution, FALSE);
	}
	delete rangeGrid;
    }
    else {
	mesh = readPlyFile(filename);
	if (mesh == NULL)
	    return NULL;
	else
	   prepareMesh(mesh);
   }    

    return mesh;
}

void
prepareMesh(Mesh *mesh) 
{
   int i;
   for (i = 0; i < mesh->numVerts; i++) {
      mesh->verts[i].stepsToEdge = 0;    
      mesh->verts[i].holeFill = FALSE;    
      mesh->verts[i].numVerts = 0;    
      mesh->verts[i].numTris = 0;    
      mesh->verts[i].maxVerts = 8;
      mesh->verts[i].verts = new Vertex*[8];
      mesh->verts[i].edgeLengths = new float[8];
      mesh->verts[i].maxTris = 8;
      mesh->verts[i].tris = new Triangle*[8];
   }
   
   for (i = 0; i < mesh->numTris; i++) {
      Triangle *tri = &mesh->tris[i];
      
      Vertex *vert1 = &mesh->verts[tri->vindex1];
      Vertex *vert2 = &mesh->verts[tri->vindex2];
      Vertex *vert3 = &mesh->verts[tri->vindex3];
      
      if (vert1->numTris == vert1->maxTris)
	 reallocTris(vert1);
      
      if (vert2->numTris == vert2->maxTris)
	 reallocTris(vert2);
      
      if (vert3->numTris == vert3->maxTris)
	 reallocTris(vert3);
      
      vert1->tris[vert1->numTris++] = tri;
      vert2->tris[vert2->numTris++] = tri;
      vert3->tris[vert3->numTris++] = tri;
      
      addNeighbors(vert1,vert2);
      addNeighbors(vert1,vert3);
      addNeighbors(vert2,vert3);
      
   }
   
   mesh->initNormals();  
   find_mesh_edges(mesh);
}


void
keepUsedVerts(Mesh *mesh, int &numVertsUsed, 
	      uchar *vertUsed, Triangle *tris) 
{
   int i;

   // Figure out which ones got used
   for (i = 0; i < mesh->numVerts; i++) {
      vertUsed[i] = 0;
   }
   
   for (i = 0; i < mesh->numTris; i++) {
      vertUsed[mesh->tris[i].vindex1] = 1;
      vertUsed[mesh->tris[i].vindex2] = 1;
      vertUsed[mesh->tris[i].vindex3] = 1;
   }
   
   // Count the number used and build new vertex array
   //  and re-indexing array and delete allocations
   //  from unused verts
   int count = 0;
   int *reIndex = new int[mesh->numVerts];
   for (i = 0; i < mesh->numVerts; i++) {
      if (vertUsed[i]) {
	 reIndex[i] = count;
	 count++;
      }
   }
   
   numVertsUsed = count;
  
   // Now re-index triangles
   for (i = 0; i < mesh->numTris; i++) {
      tris[i].vindex1 = reIndex[mesh->tris[i].vindex1];
      tris[i].vindex2 = reIndex[mesh->tris[i].vindex2];
      tris[i].vindex3 = reIndex[mesh->tris[i].vindex3];
   } 

   delete [] reIndex;   
}
