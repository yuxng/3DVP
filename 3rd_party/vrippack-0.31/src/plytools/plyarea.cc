/*

Compute the area of a triangle mesh

Brian Curless, October 1997

*/

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <strings.h>
#include <ply.h>
#include <unistd.h>

#ifdef LINUX
#include <stdlib.h>
#endif


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  float nx,ny,nz;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nz), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


/*** the PLY object ***/

static int nfaces;
static Vertex **vlist;
static Face **flist;
static int nelems;
static char **elist;
static int num_comments;
static int num_obj_info;
static int file_type;

static int flip_sign = 0;       /* flip the sign of the normals? */
static int area_weight = 0;     /* use area weighted average */

void usage(char *progname);
void read_file(FILE *inFile);
float compute_area();

/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  char *inName = NULL;
  FILE *inFile = NULL;
  progname = argv[0];
  
  
  if (argc == 1) {
    inFile = stdin;
  } else if (argc == 2 && argv[1][0] != '-') {
    /* argument supplied -- assume input filename */
    inName = argv[1];
    inFile = fopen(inName, "r");
    if (inFile == NULL) {
      fprintf(stderr, "Error: Could not open input ply file %s\n", inName);
      usage(progname);
      exit(-1);
    }
  } else {
    usage (progname);
    exit (-1);     
  }

  read_file(inFile);

  float area = compute_area();

  printf("Surface area = %g\n", area);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [in.ply]\n", progname);
  fprintf (stderr, "   or: %s < in.ply\n", progname);
  exit(-1);
}


/******************************************************************************
Compute normals at the vertices.
******************************************************************************/

float
compute_area()
{
  int i,j;
  Face *face;
  Vertex *vert;
  int *verts;
  float x,y,z;
  float x0,y0,z0;
  float x1,y1,z1;
  float area, total_area;
  float recip;

  total_area = 0;

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    verts = face->verts;

    /* Compute two edge vectors */

    x0 = vlist[verts[face->nverts-1]]->x - vlist[verts[0]]->x;
    y0 = vlist[verts[face->nverts-1]]->y - vlist[verts[0]]->y;
    z0 = vlist[verts[face->nverts-1]]->z - vlist[verts[0]]->z;

    x1 = vlist[verts[1]]->x - vlist[verts[0]]->x;
    y1 = vlist[verts[1]]->y - vlist[verts[0]]->y;
    z1 = vlist[verts[1]]->z - vlist[verts[0]]->z;

    /* find cross-product between these vectors */
    x = y0 * z1 - z0 * y1;
    y = z0 * x1 - x0 * z1;
    z = x0 * y1 - y0 * x1;

    area = sqrt(x*x + y*y + z*z)/2;

    total_area += area;

  }

  return total_area;
}


/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/

void
read_file(FILE *inFile)
{
  int i,j,k;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;
  int get_nx,get_ny,get_nz;

  /*** Read in the original PLY object ***/


  ply  = ply_read (inFile, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* see if vertex holds any normal information */
      get_nx = get_ny = get_nz = 0;
      for (j = 0; j < nprops; j++) {
        if (equal_strings ("nx", plist[j]->name)) get_nx = 1;
        if (equal_strings ("ny", plist[j]->name)) get_ny = 1;
        if (equal_strings ("nz", plist[j]->name)) get_nz = 1;
      }

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      if (get_nx) ply_get_property (ply, elem_name, &vert_props[3]);
      if (get_ny) ply_get_property (ply, elem_name, &vert_props[4]);
      if (get_nz) ply_get_property (ply, elem_name, &vert_props[5]);

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      /* set up for getting face elements */

      ply_get_property (ply, elem_name, &face_props[0]);

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);
      }
    }
  }

  ply_close (ply);
}


