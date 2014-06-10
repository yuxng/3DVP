#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>
#include <limits.h>
#include <ply.h>

#define MAX_VERT_PROPS 20

#include "plyio.h"

static void reallocTris(Vertex *v);

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
readPlyFile(FILE *inFile)
{
    int j;
    int nelems;
    char **elist;
    char *elem_name;
    int file_type;
    float version;
    int nprops, num_vert_props;
    int num_elems;
    PlyProperty **plist;

    face_props[0].offset = foffset(verts);
    face_props[0].count_offset = foffset(nverts);  /* count offset */
    
    PlyFile *ply  = ply_read (inFile, &nelems, &elist);
    ply_get_info (ply, &version, &file_type);

    if (!ply) {
      fprintf(stderr, "Error in ply_read, aborting...\n");
      exit(-1);
    }

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
    }
    
    num_vert_props = nvp;

    Mesh *mesh = new Mesh;
    Vertex *vert;
    Triangle *tri;
    PlyVertex plyVert;
    PlyFace plyFace;

    for (int i = 0; i < nelems; i++) {

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
	    
	    /* grab all the vertex elements */
	    for (j = 0; j < mesh->numVerts; j++) {
		ply_get_element (ply, (void *) &plyVert);
		vert = &mesh->verts[j];
		vert->coord.x = plyVert.x;
		vert->coord.y = plyVert.y;
		vert->coord.z = plyVert.z;
		vert->confidence = plyVert.confidence;
		vert->index = j;
	    }
	}

	if (equal_strings ("face", elem_name)) {

	    ply_get_element_setup (ply, elem_name, 1, face_props);

	    mesh->numTris = num_elems;

	    if (mesh->numTris == 0)
		continue;

	    mesh->tris = new Triangle[mesh->numTris];

	    // Assumes vertices preceed faces in the file!

	    for (j = 0; j < mesh->numTris; j++) {		
		ply_get_element (ply, (void *) &plyFace);
		tri = &mesh->tris[j];
		tri->vert1 = &mesh->verts[plyFace.verts[0]];
		tri->vert2 = &mesh->verts[plyFace.verts[1]];
		tri->vert3 = &mesh->verts[plyFace.verts[2]];
		free(plyFace.verts);
	    }
	}
    }

    return mesh;
}


int
writePlyFile(FILE *outFile, Mesh *mesh, int numVerts, int numTris)
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

    PlyFile *ply = ply_write(outFile, 2, elem_names, PLY_BINARY_BE);

    if (ply == NULL)
	return 0;


    nvp = 0;

    vert_props[nvp] = vert_prop_x;
    vert_props[nvp].offset = offsetof(PlyVertex,x); nvp++;
    vert_props[nvp] = vert_prop_y;
    vert_props[nvp].offset = offsetof(PlyVertex,y); nvp++;
    vert_props[nvp] = vert_prop_z;
    vert_props[nvp].offset = offsetof(PlyVertex,z); nvp++;

    vert_props[nvp] = vert_prop_confidence;
    vert_props[nvp].offset = offsetof(PlyVertex, confidence); nvp++;
    
    num_vert_props = nvp;

    face_props[0].offset = offsetof(PlyFace, verts);
    face_props[0].count_offset = offsetof(PlyFace, nverts);  /* count offset */
    
    ply_describe_element (ply, "vertex", numVerts, num_vert_props, vert_props);

    ply_describe_element (ply, "face", numTris, 1, face_props);

    ply_header_complete (ply);
    
    /* set up and write the vertex elements */
    PlyVertex plyVert;
    Vertex *vert;

    ply_put_element_setup (ply, "vertex");

    for (i = 0; i < mesh->numVerts; i++) {
	vert = &mesh->verts[i];
	if (vert->index < 0)
	    continue;

	plyVert.x = vert->coord.x;
	plyVert.y = vert->coord.y;
	plyVert.z = vert->coord.z;
	plyVert.confidence = vert->confidence;
	
	ply_put_element (ply, (void *) &plyVert);
    }

    PlyFace plyFace;
    Triangle *tri;
    int vertIndices[3];

    ply_put_element_setup (ply, "face");

    for (i = 0; i < mesh->numTris; i++) {
	tri = &mesh->tris[i];
	// If the triangle has 3 unique points, and they
	// haven't been deleted by triedgecol, write it out...
	if (tri->vert1 != tri->vert2 &&
	    tri->vert1 != tri->vert3 &&
	    tri->vert2 != tri->vert3 &&
	    tri->vert1->index > -1 &&
	    tri->vert2->index > -1 &&
	    tri->vert3->index > -1) {

	   plyFace.nverts = 3;
	   vertIndices[0] = tri->vert1->index;
	   vertIndices[1] = tri->vert2->index;
	   vertIndices[2] = tri->vert3->index;
	   plyFace.verts = (int *)vertIndices;

	   ply_put_element (ply, (void *) &plyFace);
	}
    }

    /* close the PLY file */
    ply_close (ply);    

    return 1;
}


