/*

Convert a shared-vertex PLY object into one where each face has its
own vertices (un-shared vertex representation).

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
void write_file();
void read_file(FILE *inFile);


/******************************************************************************
Turn a shared-vertex PLY object into an un-shared one.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  FILE *inFile = stdin;

  progname = argv[0];

  /* Parse -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
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

  write_file();
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s < in.ply > out.ply\n", progname);
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
  int vert_count;
  int index;
  Face *face;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);

  /* count the un-shared vertices */

  vert_count = 0;
  for (i = 0; i < nfaces; i++)
    vert_count += flist[i]->nverts;

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
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

  for (i = 0; i < nfaces; i++) {
    face = flist[i];
    for (j = 0; j < face->nverts; j++)
      ply_put_element (ply, (void *) vlist[face->verts[j]]);
  }

  /* set up and write the face elements */

  ply_put_element_setup (ply, "face");

  index = 0;
  for (i = 0; i < nfaces; i++) {
    face = flist[i];
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = index++;
    ply_put_element (ply, (void *) face);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

