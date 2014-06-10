/*

Compute vertex normals.

Greg Turk, August 1994

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <strings.h>
#include <ply.h>


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

static int nverts,nfaces;
static Vertex **vlist;
static Face **flist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

static int flip_sign = 0;       /* flip the sign of the normals? */
static int area_weight = 0;     /* use area weighted average */

void usage(char *progname);
void write_file();
void read_file(FILE *inFile);
void compute_normals();

/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  FILE *inFile = stdin;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 'f':
          flip_sign = 1;
          break;
        case 'a':
          area_weight = 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }


   /* optional input file (if not, read stdin ) */
   if (argc > 0 && *argv[0] != '-') {
       inFile = fopen(argv[0], "r");
       if (inFile == NULL) {
           fprintf(stderr, "Error: Couldn't open input file %s\n", argv[0]);
           usage(progname);
	   exit(-1);
       }
       argc --;
       argv ++;
   } 

   /* Check no extra args */
   if (argc > 0) {
     fprintf(stderr, "Error: Unhandled arg: %s\n", argv[0]);
     usage(progname);
     exit(-1);
   }

  read_file(inFile);

  compute_normals();

  write_file();
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s [flags] < in.ply > out.ply\n", progname);
  fprintf (stderr, "       -f (flip sign of normals)\n");
  fprintf (stderr, "       -a (use area weighted average)\n");
}


/******************************************************************************
Compute normals at the vertices.
******************************************************************************/

void
compute_normals()
{
  int i,j;
  Face *face;
  Vertex *vert;
  int *verts;
  float x,y,z;
  float x0,y0,z0;
  float x1,y1,z1;
  float len;
  float recip;

  /* zero out all normal information at vertices */

  for (i = 0; i < nverts; i++) {
    vlist[i]->nx = 0;
    vlist[i]->ny = 0;
    vlist[i]->nz = 0;
  }

  /* find normal of each face and add it to each vertex adjacent to the face */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    verts = face->verts;

    /* determine vectors parallel to two edges of face */

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

    /* normalize this vector */
    len = x*x + y*y + z*z;
    if (len == 0) {
      x = y = z = 0;
    }
    else {
      if (area_weight)
	  recip = 1;
      else
	  recip = 1 / sqrt (len);
      x *= recip;
      y *= recip;
      z *= recip;
    }

    /* add this normal to each vertex that is adjacent to face */
    for (j = 0; j < face->nverts; j++) {
      vlist[verts[j]]->nx += x;
      vlist[verts[j]]->ny += y;
      vlist[verts[j]]->nz += z;
    }
  }

  /* normalize all the normals at the vertices */

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    len = vert->nx * vert->nx + vert->ny * vert->ny + vert->nz * vert->nz;
    if (len == 0) {
      vert->nx = 0;
      vert->ny = 0;
      vert->nz = 0;
    }
    else {
      if (flip_sign)
        recip = -1 / sqrt (len);
      else
        recip = 1 / sqrt (len);
      vert->nx *= recip;
      vert->ny *= recip;
      vert->nz *= recip;
    }
  }
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
      nverts = num_elems;

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      if (get_nx) ply_get_property (ply, elem_name, &vert_props[3]);
      if (get_ny) ply_get_property (ply, elem_name, &vert_props[4]);
      if (get_nz) ply_get_property (ply, elem_name, &vert_props[5]);
      vert_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Vertex,other_props));

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
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);
      }
    }
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);
}


/******************************************************************************
Write out the PLY file to standard out.
******************************************************************************/

void
write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_property (ply, "vertex", &vert_props[4]);
  ply_describe_property (ply, "vertex", &vert_props[5]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

