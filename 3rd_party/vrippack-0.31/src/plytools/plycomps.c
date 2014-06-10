/*

Determine the connected components of a PLY object, and maybe write out
only selected components of the object.

Greg Turk, August 1994

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <ply.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;                  /* coordinate of vertex */
  int index;                    /* index of vertex in vertex list */
  int nfaces;                   /* number of faces in face list */
  int max_faces;                /* maximum number of faces in list */
  struct Face **faces;          /* face list */
  struct Comp *comp;            /* pointer to component description */
  struct Vertex *next;          /* pointer for vertex queue */
  void *other_props;            /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;         /* number of vertex indices in list */
  Vertex **verts;               /* vertex index list */
  struct Comp *comp;            /* pointer to component description */
  void *other_props;            /* other properties */
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

int nverts,nfaces;
Vertex **vlist;
Face **flist;
PlyOtherElems *other_elements = NULL;
PlyOtherProp *vert_other,*face_other;
int nelems;
char **elist;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
int file_type;

#define UNTOUCHED -1
#define ON_QUEUE  -2


/* queue used to assign components */
static Vertex *queue_start = NULL;
static Vertex *queue_end = NULL;

/* the connected components */

typedef struct Comp {
  int comp_num;         /* component number stored at vertices */
  int vcount;           /* how many vertices in this component */
  int fcount;           /* how many faces in this component */
} Comp;

static int num_comps;           /* number of components */
static Comp **comp_list;        /* list of components */
static int comp_max = 100;      /* maximum number in list */

static int not_silent = 1;      /* print out info about components? */
static int top_n = 10;          /* don't print or write out 
				   more components than this */
static int wanted_top_n = 0;    /* Did the user want to limit the number
				   of copmonents printed? */

void usage(char *progname);
void write_file();
void read_file();
void find_components();
int compare_components(Comp **c1, Comp **c2);
void process_queue(int num_comps);
void on_queue(Vertex *vert);
void index_verts();
void ints_to_ptrs();
void write_more(char *filename, int more);
void write_less(char *filename, int less);


/******************************************************************************
Transform a PLY file.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  int less = -1;
  int more = -1;
  char *filename;
  struct tms buffer;
  clock_t tm;
  FILE *inFile = stdin;

  times(&buffer);
  tm = buffer.tms_utime;

  progname = argv[0];

  /* Parse all -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 't':
          top_n = atoi (*++argv);
	  wanted_top_n = 1;
          argc -= 1;
          break;
        case 's':
          not_silent = 1 - not_silent;
          break;
        case 'm':
          more = atoi (*++argv);
          filename = strdup (*++argv);
          argc -= 2;
          break;
        case 'l':
          less = atoi (*++argv);
          filename = strdup (*++argv);
          argc -= 2;
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
  index_verts();
  ints_to_ptrs();
  find_components();

  /* maybe write all components that have at least this number of vertices */
  if (more > -1)
    write_more (filename, more);

  /* maybe write all components that have at most this number of vertices */
  if (less > -1)
    write_less (filename, less);

  times(&buffer);
  tm = buffer.tms_utime - tm;

  fprintf(stdout, "%f seconds\n", tm/(double)CLOCKS_PER_SEC);

  exit(0);

}


/******************************************************************************
Print out usage information about this program.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stdout, "usage: %s [flags] [in.ply]\n", progname);
  fprintf (stdout, "usage: %s [flags] < in.ply\n", progname);
  fprintf (stdout, "       -s (silent mode)\n");
  fprintf (stdout, "       -m num filename\n");
  fprintf (stdout, "          (writes out all components with >= num verts)\n");
  fprintf (stdout, "       -l num filename\n");
  fprintf (stdout, "          (writes out all components with <= num verts)\n");
  fprintf (stdout, "       -t num (max num of components printed to screen and written to file)\n");
  exit(-1);
}


/******************************************************************************
Find the connected components of a collection of polygons.
******************************************************************************/

void
find_components()
{
  int i,j,k;
  Face *face;
  Vertex *vert,*vert2;
  int compare_components();

  /* create pointers from the vertices to the faces */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];

    /* make pointers from the vertices to this face */

    for (j = 0; j < face->nverts; j++) {
      vert = face->verts[j];
      /* make sure there is enough room for the new face pointer */
      if (vert->nfaces >= vert->max_faces) {
        vert->max_faces += 3;
        vert->faces = (Face **) 
                      realloc (vert->faces, sizeof (Face *) * vert->max_faces);
      }
      /* add the face to this vertice's list of faces */
      vert->faces[vert->nfaces] = face;
      vert->nfaces++;
    }
  }

  /* label all vertices as initially untouched */

  for (i = 0; i < nverts; i++)
    vlist[i]->comp = (Comp *) UNTOUCHED;

  /* initialize the component count list */
  comp_list = (Comp **) malloc (sizeof (Comp *) * comp_max);

  /* examine each vertex to see what component it belongs to */

  num_comps = 0;

  for (i = 0; i < nverts; i++) {

    vert = vlist[i];

    /* don't touch it if we've already assigned it a component number */
    if (vert->comp != (Comp *) UNTOUCHED)
      continue;

    /* initialize the info for this component */
    comp_list[num_comps] = (Comp *) malloc (sizeof (Comp));
    comp_list[num_comps]->comp_num = num_comps;
    comp_list[num_comps]->vcount = 0;
    comp_list[num_comps]->fcount = 0;

    /* place this vertex on the queue */
    on_queue (vert);

    /* process the queue until it is empty */
    while (queue_start)
      process_queue (num_comps);

    /* if we get here we've got a new component */
    num_comps++;
    if (num_comps >= comp_max) {
      comp_max *= 2;
      comp_list = (Comp **) realloc (comp_list, sizeof (Comp *) * comp_max);
    }
  }

  /* count the faces in each component */

  for (i = 0; i < nfaces; i++) {
    flist[i]->comp = flist[i]->verts[0]->comp;
    flist[i]->comp->fcount++;
  }

  /* sort the list of components by number of vertices */

  qsort (comp_list, num_comps, sizeof (Comp *), 
	 (int (*)(const void *, const void *))compare_components);

  /* print out info about components */

  if (not_silent) {
    fprintf (stdout, "\n");

    for (i = 0; i < num_comps; i++) {
      /*
      fprintf (stdout, "comp %d : %d verts, %d faces\n",
              comp_list[i]->comp_num,
              comp_list[i]->vcount,
              comp_list[i]->fcount);
      */
      fprintf (stdout, "%d verts, %d faces\n",
               comp_list[i]->vcount, comp_list[i]->fcount);
      if (i+1 >= top_n)
        break;
    }
    if (num_comps > top_n)
      fprintf (stdout, "...\n");

    fprintf (stdout, "(%d components)\n", num_comps);
    fprintf (stdout, "\n");
  }
}


/******************************************************************************
Compare two component entries for quicksort.
******************************************************************************/

int
compare_components(Comp **c1, Comp **c2)
{
  if ((*c1)->vcount < (*c2)->vcount)
    return (1);
  else if ((*c1)->vcount > (*c2)->vcount)
    return (-1);
  else
    return (0);
}


/******************************************************************************
Process one vertex on the queue.
******************************************************************************/

void
process_queue(int num_comps)
{
  int i,j;
  Vertex *vert,*vert2;
  Face *face;

  /* pop one vertex off the queue */

  vert = queue_start;
  queue_start = vert->next;

  /* store the vertex's component number */
  vert->comp = comp_list[num_comps];

  /* count this new component */
  comp_list[num_comps]->vcount++;

  /* place this vertex's neighbors on the queue */

  for (i = 0; i < vert->nfaces; i++) {
    face = vert->faces[i];
    for (j = 0; j < face->nverts; j++) {
      vert2 = face->verts[j];
      if (vert2->comp == (Comp *) UNTOUCHED)
        on_queue (vert2);
    }
  }
}


/******************************************************************************
Place a vertex on the queue.
******************************************************************************/

void
on_queue(Vertex *vert)
{
  vert->comp = (Comp *) ON_QUEUE;
  vert->next = NULL;

  if (queue_start == NULL) {
    queue_start = vert;
    queue_end = vert;
  }
  else {
    queue_end->next = vert;
    queue_end = vert;
  }
}


/******************************************************************************
Index the vertices.
******************************************************************************/

void
index_verts()
{
  int i;
  Vertex *vert;

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vert->index = i;
    vert->nfaces = 0;
    vert->max_faces = 3;
    vert->faces = (Face **) malloc (sizeof (Face *) * vert->max_faces);
  }
}


/******************************************************************************
Change the vertex indices from integers to pointers.
******************************************************************************/

void
ints_to_ptrs()
{
  int i,j;
  Vertex **verts;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = vlist[(int) verts[j]];
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
Write out those components that contain at least a given number of vertices.

Entry:
  filename - name of file to write to
  more     - minimum number of vertices to have component be written
******************************************************************************/

void
write_more(char *filename, int more)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vsum,fsum;
  float version;
  Vertex **verts;
  int vcount;
  int comp_count;
  int new_more;

  ply = ply_open_for_writing(filename, nelems, elist, file_type, &version);


  if (wanted_top_n) {
      comp_count = top_n < num_comps ? top_n : num_comps;
      new_more = comp_list[comp_count-1]->vcount;
  }
  else {
      comp_count = num_comps;
      new_more = more;
  }

  /* count vertices and faces that will be written */
  vsum = 0;
  fsum = 0;
  for (i = 0; i < comp_count; i++) {
    if (comp_list[i]->vcount >= new_more) {
      vsum += comp_list[i]->vcount;
      fsum += comp_list[i]->fcount;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vsum);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", fsum);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* change the vertex indices from pointers to indices */

  vcount = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount >= new_more)
      vlist[i]->index = vcount++;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = (Vertex *) verts[j]->index;
  }

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount >= new_more)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    if (flist[i]->comp->vcount >= new_more)
      ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}


/******************************************************************************
Write out those components that contain at most a given number of vertices.

Entry:
  filename - name of file to write to
  less     - maximum number of vertices to have component be written
******************************************************************************/

void
write_less(char *filename, int less)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vsum,fsum;
  float version;
  Vertex **verts;
  int vcount;
  int new_less, comp_count;

  ply = ply_open_for_writing(filename, nelems, elem_names, file_type, &version);

  if (wanted_top_n) {
      comp_count = top_n < num_comps ? top_n : num_comps;
      new_less = comp_list[comp_count-1]->vcount;
  }
  else {
      comp_count = num_comps;
      new_less = less;
  }

  /* count vertices and faces that will be written */
  vsum = 0;
  fsum = 0;
  for (i = 0; i < num_comps; i++) {
    if (comp_list[i]->vcount <= new_less) {
      vsum += comp_list[i]->vcount;
      fsum += comp_list[i]->fcount;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vsum);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", fsum);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* change the vertex indices from pointers to indices */

  vcount = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount <= new_less)
      vlist[i]->index = vcount++;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = (Vertex *) verts[j]->index;
  }

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount <= new_less)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    if (flist[i]->comp->vcount <= new_less)
      ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
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

  /*** Write out the transformed PLY object ***/


  ply = ply_write (stdout, nelems, elem_names, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
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

