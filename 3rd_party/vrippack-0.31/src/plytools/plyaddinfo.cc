/*

Skeleton of program that manipulates a PLY object.  (Was plyxform.)

Greg Turk, August 1994

*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ply.h>
#include <limits.h>
#include <stdlib.h>

void write_file(char *filename);
void read_file(char *filename);
void usage(char *progname);

// Must have at least one element to describe or this blows up!!!!
// ply bug!!!

/* user's vertex definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  void *other_props;       /* other properties */
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

static int nverts;
static Vertex **vlist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;
static char **new_info;
static char num_new_info;


/******************************************************************************
Main program.
******************************************************************************/

main(int argc, char *argv[])
{
  int i;
  char *progname;
  char *filename;

  progname = argv[0];

  if (argc < 3) {
      usage(progname);
      exit(1);
  }

  /* Check arg1 is filename, not flag */
  if (argv[1][0] == '-') {
    if (strlen(argv[1]) == 1) {
      /* Ok.  The "-" arg means use stdin/stdout */
    } else {
      fprintf(stderr, "Error: Unhandled arg: %s\n", argv[1]);
      usage(progname);
      exit(-1);
    }
  }

  filename = argv[1];

  num_new_info = 0;
  new_info = (char **)malloc((argc - 2)*sizeof(char *));
  for (i = 2; i < argc; i++) {
     new_info[num_new_info] = (char *)malloc(PATH_MAX*sizeof(char));     
     new_info[num_new_info][0] = 0;
     strcat(new_info[num_new_info], argv[i]);
     num_new_info++;
  }

  read_file(filename);
  write_file(filename);

  return 0;
}


/******************************************************************************
Print out usage information.
******************************************************************************/
void
usage(char *progname)
{
  fprintf (stderr, "Usage: %s file.ply 'info_1' ... 'info_n'\n", progname);
  fprintf (stderr, "   or: %s - 'info_1' ... 'info_n' < in.ply > out.ply\n", 
	   progname);
}


/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/
void
read_file(char *filename)
{
  int i;
  PlyFile *ply;
  int nprops;
  int num_elems;
  char *elem_name;
  float version;


  /*** Read in the original PLY object ***/
  if (!strcmp(filename, "-")) {
    ply = ply_read (stdin, &nelems, &elist);
    file_type = ply->file_type;
  } else {
    ply = 
      ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);
  }

  if (ply == NULL) {
      fprintf(stderr, "Could not open file %s for reading.\n", filename);
      exit(1);
  }

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    ply_get_element_description (ply, elem_name, &num_elems, &nprops);

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
      for (int j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
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
write_file(char *filename)
{
  int i;
  PlyFile *ply;
  float version;

  /*** Write out the final PLY object ***/

  if (!strcmp(filename, "-")) {
    ply = ply_write(stdout, nelems, elist, file_type);
  } else {
    ply = ply_open_for_writing(filename, nelems, elist, file_type, &version);
  }


  if (ply == NULL) {
      printf("Could not open file %s for writing.\n", filename);
      exit(1);
  }

  /* describe what properties go into the vertex elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  for (i = 0; i < num_new_info; i++)
    ply_put_obj_info (ply, new_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

