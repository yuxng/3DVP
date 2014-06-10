/*

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <ply.h>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
   char *other_props;
} Vertex;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};


/*** the PLY object ***/

int nverts;
Vertex **vlist;
PlyOtherElems *other_elements = NULL;
PlyOtherProp *vert_other;
int nelems;
char **elist;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
int file_type;

static float scale;

void usage(char *progname);
void write_file();
void read_file(FILE *inFile);


/******************************************************************************
Transform a PLY file.
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

  scale = 1;

  /* Read all the -args */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 's':
          scale = atof(*++argv);
	  argc -= 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  /* Get the input file name */
  if (argc == 0) {
    /* No input name -- read from stdin */
    inFile = stdin;
  } else if (argc == 1 && argv[0][0] != '-') {
    /* Open input file */
    inName = argv[0];
    inFile = fopen(inName, "r");
    if (inFile == NULL) {
      fprintf(stderr, "Error, could not open input ply file: %s\n", inName);
      usage(progname);
      exit(-1);
    }
  } else {
    fprintf(stderr, "Unhandled argument: %s\n", argv[0]);
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
  fprintf (stderr, "Usage: %s [flags] [in.ply] > out\n", progname);
  fprintf (stderr, "   or: %s [flags] < in.ply > out\n", progname);
  fprintf (stderr, "Flags:\n");
  fprintf (stderr, "       -s scale\n");
  exit(-1);
}


/******************************************************************************
Transform the PLY object.
******************************************************************************/

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

  for (i = 0; i < nverts; i++)
     fprintf(stdout, "%d %d %d\n", (int)(scale*vlist[i]->x), 
	     (int)(scale*vlist[i]->y), (int)(scale*vlist[i]->z));
}

