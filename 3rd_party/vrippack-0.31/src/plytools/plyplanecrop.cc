/*

plyplanecrop:
crop away all points in a ply file below a specified plane.

Brian Curless, 2005.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

float A,B,C,D;

/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  int index;
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

void usage(char *progname);
void read_file(FILE *inFile);
void write_file();


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

  if (argc < 5 || argc > 6) {
     usage (progname);
     exit (-1);
  }

  A = atof(argv[1]);
  B = atof(argv[2]);
  C = atof(argv[3]);
  D = atof(argv[4]);

  /* optional input file (if not, read stdin ) */
  if (argc == 6) {
    inFile = fopen(argv[5], "r");
    if (inFile == NULL) {
      fprintf(stderr, "Error: Couldn't open input file %s\n", argv[0]);
      usage(progname);
      exit(-1);
    }
  } 

   // Read in the file
  read_file(inFile);

  // Write back out, ignoring points below the plane
  write_file();
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s <a> <b> <c> <d> [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s <a> <b> <c> <d> < in.ply > out.ply\n", progname);
  fprintf (stderr, "\n");
  fprintf (stderr, "   This program will remove all vertices (and their\n");
  fprintf (stderr, "   corresponding faces) that are outside the specified\n");
  fprintf (stderr, "   plane, i.e., vertices where a*x + b*y + c*z + d < 0.\n");
  fprintf (stderr, "\n");
  exit(-1);
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


  /*** Read in the original PLY object ***/


  ply  = ply_read (inFile, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
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
	/* DEBUG
	fprintf(stderr, "face %d: %d verts: %d %d %d\n",
		j, flist[j]->nverts,
		flist[j]->verts[0],
		flist[j]->verts[1],
		flist[j]->verts[2]);
	*/
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
Ignore all the points (and corresponding faces) below
the plane.
******************************************************************************/

void
write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);

  // count the vertices that are above the plane
  vert_count = 0;
  for (i = 0; i < nverts; i++) {
    // Set the index to either the index number, or -1...
    if (A*vlist[i]->x + B*vlist[i]->y + C*vlist[i]->z + D >= 0) {
      vlist[i]->index = vert_count;
      vert_count++;
    } else {
      vlist[i]->index = -1;
    }
  }

  // count the faces that are still valid
  face_count = 0;
  for (i = 0; i < nfaces; i++) {
    bool valid = (flist[i]->nverts > 0);
    for (j = 0; j < flist[i]->nverts; j++) {
      if (vlist[flist[i]->verts[j]]->index == -1) {
	valid = false;
	break;
      }
    }
    
    // If face not valid, set nverts to 0, so it won't
    // get written out later.
    if (valid) {
      face_count++;
    } else {
      flist[i]->nverts = 0;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
  ply_element_count (ply, "face", face_count);
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
    if (vlist[i]->index > -1)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");

  for (i = 0; i < nfaces; i++) {
    if (flist[i]->nverts == 0)
      continue;
    for (j = 0; j < flist[i]->nverts; j++)
      flist[i]->verts[j] = (vlist[flist[i]->verts[j]])->index;
    ply_put_element (ply, (void *) flist[i]);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}
