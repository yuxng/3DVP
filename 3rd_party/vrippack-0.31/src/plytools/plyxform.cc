/*

Apply a 3-D transformation to an object from a PLY file.

Greg Turk, August 1994

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <ply.h>

#include "Linear.h"


/* user's vertex and face definitions for a polygonal object */

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

static float xtrans = 0;
static float ytrans = 0;
static float ztrans = 0;

static float xscale = 1;
static float yscale = 1;
static float zscale = 1;

static float rotx = 0;
static float roty = 0;
static float rotz = 0;

static Quaternion quat;

static Matrix4f xfmat;

void usage(char *progname);
void write_file();
void read_file(FILE *inFile);
void transform();

/******************************************************************************
Transform a PLY file.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  char *xfname = NULL;
  FILE *inFile = stdin;

  progname = argv[0];

  quat.q[0] = 0;
  quat.q[1] = 0;
  quat.q[2] = 0;
  quat.q[3] = 1;

  xfmat.makeIdentity();

  /* Parse -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 's':
	  if (argc < 4) usage(progname);
          xscale = atof (*++argv);
          yscale = atof (*++argv);
          zscale = atof (*++argv);
          argc -= 3;
          break;
        case 'f':
	  if (argc < 2) usage(progname);
	  xfname = (*++argv);
	  argc-=1;
          break;
        case 't':
	  if (argc < 4) usage(progname);
          xtrans = atof (*++argv);
          ytrans = atof (*++argv);
          ztrans = atof (*++argv);
          argc -= 3;
          break;
        case 'r':
	  if (argc < 4) usage(progname);
          rotx = atof (*++argv) * M_PI/180;
          roty = atof (*++argv) * M_PI/180;
          rotz = atof (*++argv) * M_PI/180;
          argc -= 3;
          break;
        case 'q':
	  if (argc < 5) usage(progname);
          quat.q[0] = atof (*++argv);
          quat.q[1] = atof (*++argv);
          quat.q[2] = atof (*++argv);
          quat.q[3] = atof (*++argv);
          argc -= 4;
          break;
        default:
	  usage(progname);
	  exit(-1);
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

  /* Read xf file if given... */
  if (xfname) {
    FILE *xf = fopen(xfname, "r");
    if (xf == NULL) {
      fprintf(stderr, "Error, couldn't open .xf file %s\n", xfname);
      usage(progname);
      exit(-1);
    }
    for (int i=0; i < 4; i++) {
      float a,b,c,d;
      fscanf(xf, "%f %f %f %f\n", &a, &b, &c, &d);
      xfmat.setElem(i,0,a);
      xfmat.setElem(i,1,b);
      xfmat.setElem(i,2,c);
      xfmat.setElem(i,3,d);
    }
    fclose(xf);
  }

  read_file(inFile);
  transform();
  write_file();
}


/******************************************************************************
Transform the PLY object.
******************************************************************************/

void
transform()
{
  int i;
  Vertex *vert;
  Vec3f vec1, vec2;
  Matrix4f mat, qmat;

  quat.toMatrix(qmat);
  mat.makeIdentity();
  mat.scale(xscale, yscale, zscale);
  mat.rotateX(rotx);
  mat.rotateY(roty);
  mat.rotateZ(rotz);
  mat.multLeft(qmat);
  mat.setTranslate(xtrans, ytrans, ztrans);
  mat.multLeft(xfmat);

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vec1.setValue(vert->x, vert->y, vert->z);
    mat.multVec(vec1, vec2);
    vert->x = vec2.x;
    vert->y = vec2.y;
    vert->z = vec2.z;
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
  char *elem_name;
  float version;


  /*** Read in the original PLY object ***/


  ply  = ply_read (inFile, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

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
      for (j = 0; j < num_elems; j++) {
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
write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  /*** Write out the transformed PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);


  /* describe what properties go into the vertex and face elements */

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

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  ply_put_other_elements (ply);

  ply_close (ply);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s [flags] < in.ply > out.ply\n", progname);
  fprintf (stderr, "       -f m.xf (a transform matrix file)\n");
  fprintf (stderr, "       -t xtrans ytrans ztrans\n");
  fprintf (stderr, "       -s xscale yscale zscale\n");
  fprintf (stderr, "       -r xangle yangle zangle (all in degrees)\n");
  fprintf (stderr, "       -q qi qj qk ql\n");
  fprintf (stderr, "  (point = m.xf * (ftrans_factor + rotz * roty * rotx * scale_factor * point))\n");
  exit (-1);
}
