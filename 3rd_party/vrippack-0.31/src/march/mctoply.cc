/*

Converts from Paul Ning's polygon file format to text polygon description.

Greg Turk - November 1992

*/

#include <stdio.h>
#include <malloc.h>
#include "mcfile.h"
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "Linear.h"

#include "ply.h"

struct PlyVertex {
    float x, y, z;
    float nx, ny, nz;
};

struct PlyFace {
    unsigned char nverts;
    int *verts;
};

static PlyVertex *vert_dummy;
#define voffset(field) ((char *) (&vert_dummy->field) - (char *) vert_dummy)

static PlyFace *face_dummy;
#define foffset(field) ((char *) (&face_dummy->field) - (char *) face_dummy)

static char *elem_names[] = { 
  "vertex", "face",
};

static PlyProperty vert_props[] =  {
    {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"nx", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"ny", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
    {"nz", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0}
};

static PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};


TriangleVertex *verts;
int npolys;


void write_polys_ply(char *filename);


/******************************************************************************
Main routine.
******************************************************************************/

void
main(int, char **argv)
{
  int i,j;
  FILE *fp;
  char infile[80],outfile[80];
  mcfile header;
  int index;

  strcpy (infile, argv[1]);
  strcpy (outfile, argv[2]);

  if (strlen (infile) < 3 ||
      strcmp (infile + strlen (infile) - 3, ".mc") != 0)
      strcat (infile, ".mc");

  if (strlen (outfile) < 5 ||
      strcmp (outfile + strlen (outfile) - 5, ".ply") != 0)
      strcat (outfile, ".ply");

  printf ("reading polygons from '%s'\n", infile);

  /* open the polygon input file */

  if ((fp = fopen(infile, "r")) == NULL) {
    fprintf (stderr, "bad open\n");
    exit (-1);
  }

  /* open the polygon output file */

/*
  if ((fp_out = fopen(outfile, "w+b")) == NULL) {
    fprintf (stderr, "bad open\n");
    exit (-1);
  }
*/

  /* read header info from the polygon file */
  header = MC_ReadHeader (fp);

  /*
  npolys = header.mc_length / (3 * sizeof (sizeof(TriangleVertex)));
  */
  npolys = header.mc_length / 42;
  printf ("%d polygons\n", npolys);
  verts = (TriangleVertex *) malloc (sizeof (TriangleVertex) * 3 * npolys);
  if (verts == 0) {
    fprintf (stderr, "can't allocate enough space\n");
    exit (-1);
  }

  /* read in the polygons */
  printf ("reading polygons...\n");
  for (i = 0; i < npolys; i++) {
    for (j = 0; j < 3; j++) {
      index = i * 3 + j;
      verts[index] = MC_ReadTriangleVertex (fp);
    }
  }

  /* write polygons to output file */
  printf ("writing polygons...\n");
  write_polys_ply (outfile);

  printf ("done.\n");
}


/******************************************************************************
Write out polygons in Inventor format.
******************************************************************************/

void
write_polys_ply(char *filename)
{
    int i,j;
    int index;
    float len;
    float version;

    PlyFile *ply = ply_open_for_writing(filename, 2, elem_names, 
					PLY_BINARY_BE, &version);


    // Shouldn't this be a tri-strip-set???
    int numVerts = 3*npolys;  // Hugely inefficient!!
    int numFaces = npolys;   

    int nvp = 0;
    vert_props[nvp].offset = voffset(x); nvp++;
    vert_props[nvp].offset = voffset(y); nvp++;
    vert_props[nvp].offset = voffset(z); nvp++;

    vert_props[nvp].offset = voffset(nx); nvp++;
    vert_props[nvp].offset = voffset(ny); nvp++;
    vert_props[nvp].offset = voffset(nz); nvp++;


    face_props[0].offset = foffset(verts);
    face_props[0].count_offset = foffset(nverts);  /* count offset */
    
    ply_describe_element (ply, "vertex", numVerts, 
			  nvp, vert_props);

    ply_describe_element (ply, "face", npolys, 1, face_props);

    ply_header_complete (ply);
    
    /* set up and write the vertex elements */
    PlyVertex vert;
    ply_put_element_setup (ply, "vertex");
    for (i = 0; i < npolys; i++) {
	for (j = 0; j < 3; j++) {
	    index = i * 3 + j;
	    vert.x = verts[index].x / 128.0;
	    vert.y = verts[index].y / 128.0;
	    vert. z = verts[index].z / 128.0;
	    vert.nx = -verts[index].nx;
	    vert.ny = -verts[index].ny;
	    vert.nz = -verts[index].nz;
	    len = sqrt (vert.nx*vert.nx + vert.ny*vert.ny + vert.nz*vert.nz);
	    vert.nx /= len;
	    vert.ny /= len;
	    vert.nz /= len;
	    ply_put_element (ply, (void *) &vert);
	}
    }    

    /* set up and write the face elements */
    PlyFace face;
    int v[3];
    face.nverts = 3;
    face.verts = (int *)(v);
    ply_put_element_setup (ply, "face");
    for (i = 0; i < npolys; i++) {
	Vec3f v1(verts[i*3].x, verts[i*3].y, verts[i*3].z);
	Vec3f v2(verts[i*3+1].x, verts[i*3+1].y, verts[i*3+1].z);
	Vec3f v3(verts[i*3+2].x, verts[i*3+2].y, verts[i*3+2].z);

	Vec3f a = v2 - v1;
	Vec3f b = v3 - v1;
	Vec3f c = a.cross(b);
	Vec3f norm(-verts[i*3].nx, -verts[i*3].ny, -verts[i*3].nz);
	if (c.dot(norm) > 0) {
	    face.verts[0] = i*3;
	    face.verts[1] = i*3+1;
	    face.verts[2] = i*3+2;
	}
	else {
	    face.verts[0] = i*3+1;
	    face.verts[1] = i*3;
	    face.verts[2] = i*3+2;
	}
	ply_put_element (ply, (void *) &face);
    }    
    
    /* close the PLY file */
    ply_close (ply);    
}

