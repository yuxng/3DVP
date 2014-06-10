/*

Turn a PLY object with un-shared vertices into one with faces that
share their vertices.

Greg Turk, August 1994

Modified to handle multiple input files, 
Lucas Pereira, 1/99
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#ifdef LINUX
#include <string.h>
#endif


/* user's vertex and face definitions for a polygonal object */


struct _Face;
typedef struct _Flist {
  struct _Face *f;
  struct _Flist *next;
} Flist;

typedef struct _Vertex {
  float x,y,z;
  int index;
  struct _Vertex *shared;
  struct _Vertex *next;
  void *other_props;       /* other properties */
  int nfaces;
  Flist *faces;		   // linked list of faces, to detect repeats
} Vertex;

typedef struct _Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  Vertex **vptrs;          /* vertex pointer list -- instead of recasting verts! */
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

typedef struct _PlyObject {
  int nverts,nfaces;
  Vertex **vlist;
  Face **flist;
  PlyOtherElems *other_elements;
  PlyOtherProp *vert_other,*face_other;
  int nelems;
  char **elist;
  int num_comments;
  char **comments;
  int num_obj_info;
  char **obj_info;
  int file_type;
} PlyObject;

static PlyObject *os;  // We will allocate when we know how many files...
static int nFiles = 0;

static float tolerance = 0.0001;   /* what constitutes "near" */


/* hash table for near neighbor searches */
/* A few randomly chosen prime numbers... */

#define PR1  17
#define PR2 101

#define TABLE_SIZE1       5003
#define TABLE_SIZE2      17011
#define TABLE_SIZE3      53003
#define TABLE_SIZE4     107021
#define TABLE_SIZE5     233021
#define TABLE_SIZE6     472019
#define TABLE_SIZE7     600169
#define TABLE_SIZE8    1111189
#define TABLE_SIZE9    2222177
#define TABLE_SIZE10   4444147
#define TABLE_SIZE11   9374153
#define TABLE_SIZE12  20123119
#define TABLE_SIZE13  30123139
#define TABLE_SIZE14  50123011
#define TABLE_SIZE15  70123117
#define TABLE_SIZE16 100123171
#define TABLE_SIZE17 140123143
#define TABLE_SIZE18 200123111
#define TABLE_SIZE19 400123123
#define TABLE_SIZE20 800123119




typedef struct Hash_Table {	/* uniform spatial subdivision, with hash */
  int npoints;			/* number of points placed in table */
  Vertex **verts;		/* array of hash cells */
  int num_entries;		/* number of array elements in verts */
  float scale;			/* size of cell */
} Hash_Table;


Hash_Table *init_table(int, float);
void add_to_hash (Vertex *, Hash_Table *, float);
void usage(char *progname);
void write_file();
void read_file(FILE *inFile, PlyObject &o);
void set_vptrs(PlyObject &o);
void set_flists(PlyObject &o);
void share_vertices();
void remove_duplicate_faces();


/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  FILE **inFiles;

  progname = argv[0];

  /* Parse -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 't':
	  if (argc < 2) {
	    usage(progname);
	  }
          tolerance = atof (*++argv);
          argc -= 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  // Ok. Tricky logic here.  Either a single file on stdin, or else 
  // one or more files on the command line.
  if (argc == 0) {
    // one file, on stdin...
    fprintf(stderr, "%s: reading from stdin...\n", progname);
    nFiles = 1;
    inFiles = (FILE **) malloc(sizeof(FILE *));
    inFiles[0] = stdin;
  } else {
    // allocate file handles for all input files
    nFiles = argc;
    inFiles = (FILE **) malloc(nFiles * sizeof(FILE *));
    // open each of the files
    for (i=0; i < argc; i++) {
      if (argv[i][0] == '-') {
	fprintf(stderr, "Error: unhandled arg %s. (in wrong order, maybe?)\n",
		argv[i]);
	usage(progname);
	exit(-1);
      } else {
	if ((inFiles[i] = fopen(argv[i], "r")) == NULL) {
	  fprintf(stderr, "Error: Couldn't open input file %s\n", argv[i]);
	  usage(progname);
	  exit(-1);
	}
      }
    }
  }
  
  // Now allocate the ply objects, and read them
  os = (PlyObject *) malloc(nFiles * sizeof(PlyObject));
  for (i=0; i < nFiles; i++) {
    read_file(inFiles[i], os[i]);
    // Check if any extra elements
    if (i > 0 && os[i].nelems > 2) {
      fprintf(stderr, "Warning:  file %s has %d elements. Discarding extras...\n",
	      argv[i], os[i].nelems);
    }

    // Check to make sure Vertex's other_props are identical to file 0
    if (i > 0 && os[i].vert_other->nprops != os[0].vert_other->nprops) {
      fprintf(stderr, "Error: file %s has %d extra vertex properties, but %s has %d.\n",
	      argv[0], os[0].vert_other->nprops,
	      argv[i], os[i].vert_other->nprops);
      fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
      exit(-1);
    }
    if (i > 0 && os[i].vert_other->size != os[0].vert_other->size) {
      fprintf(stderr, "Error: file %s extra vertex properties have size %d, "
	      "but %s has size %d.\n",
	      argv[0], os[0].vert_other->size,
	      argv[i], os[i].vert_other->size);
      fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
      exit(-1);
    }
    if (i > 0) {
      for (j=0; j < os[i].vert_other->nprops; j++) {
	if (strcmp(os[i].vert_other->props[j]->name,
		   os[0].vert_other->props[j]->name)) {
	  fprintf(stderr, "Error: file %s extra vertex property %d is %s, "
		  " but %s extra vertex property %d is %s\n", 
		  argv[0], j, os[0].vert_other->props[j]->name,
		  argv[i], j, os[i].vert_other->props[j]->name);
	  fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
	  exit(-1);
	}
      }
    }
    
    // Check to make sure Face's other_props are identical to file 0
    if (i > 0 && os[i].face_other->nprops != os[0].face_other->nprops) {
      fprintf(stderr, "Error: file %s has %d extra face properties, but %s has %d.\n",
	      argv[0], os[0].face_other->nprops,
	      argv[i], os[i].face_other->nprops);
      fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
      exit(-1);
    }
    if (i > 0 && os[i].face_other->size != os[0].face_other->size) {
      fprintf(stderr, "Error: file %s extra face properties have size %d, "
	      "but %s has size %d.\n",
	      argv[0], os[0].face_other->size,
	      argv[i], os[i].face_other->size);
      fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
      exit(-1);
    }
    if (i > 0) {
      for (j=0; j < os[i].face_other->nprops; j++) {
	if (strcmp(os[i].face_other->props[j]->name,
		   os[0].face_other->props[j]->name)) {
	  fprintf(stderr, "Error: file %s extra face property %d is %s, "
		  " but %s extra face property %d is %s\n", 
		  argv[0], j, os[0].face_other->props[j]->name,
		  argv[i], j, os[i].face_other->props[j]->name);
	  fprintf(stderr, "Program terminated for unreconcilable differences.\n\n");
	  exit(-1);
	}
      }
    }
    
    set_vptrs(os[i]);
    set_flists(os[i]);
  }

  share_vertices();

  remove_duplicate_faces();

  write_file();
  return(0);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "\n");
  fprintf (stderr, "usage: %s [flags] < in.ply > out.ply\n", progname);
  fprintf (stderr, "usage: %s [flags] [in1.ply [in2.ply] ...] > out.ply\n", progname);
  fprintf (stderr, "flags:\n");
  fprintf (stderr, "       -t tolerance (default = %g)\n", tolerance);
  fprintf (stderr, "\n");
  fprintf (stderr, "This program takes one or more ply files, and merges \n");
  fprintf (stderr, "vertices that are within tolerance of each other.\n");
  fprintf (stderr, "This is useful to combine multiple meshes into a single\n");
  fprintf (stderr, "mesh, with smoothly continuous normals.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "If handling more than one file, the vertex and face elements\n");
  fprintf (stderr, "must have exactly the same properties (in the same order)\n");
  fprintf (stderr, "in all files or else it will print an error and exit.  If\n");
  fprintf (stderr, "the files have extra elements, they will be ignored (except \n");
  fprintf (stderr, "for the first file, which has all extra elements written out,\n");
  fprintf (stderr, "to maintain backward-compatibility with old plyshared.  It will\n");
  fprintf (stderr, "print a warning message if elements are ignored.\n");
  fprintf (stderr, "\n");
  exit(-1);
}

/******************************************************************************
Make face vptrs point to the vertices...
******************************************************************************/

void
set_vptrs(PlyObject &o) {
  int i,j;
  Face *f;
  Vertex *v;

  // Malloc vptrs array, set the pointers.
  for (i=0; i < o.nfaces; i++) {
    f = o.flist[i];
    f->vptrs = (Vertex **) malloc(f->nverts * sizeof(Vertex *));
    for (j=0; j < f->nverts; j++) {
      v = o.vlist[f->verts[j]];
      f->vptrs[j] = v;
    }
  }
}


/******************************************************************************
Make vertices point to the faces...
******************************************************************************/

void
set_flists(PlyObject &o) {
  int i,j;
  Face *f;
  Vertex *v;
  Flist *fl;

  // First set all vert facecounters to 0
  for (i=0; i < o.nverts; i++) {
    v = o.vlist[i];
    v->nfaces = 0;
  }

  for (i=0; i < o.nfaces; i++) {
    f = o.flist[i];
    for (j=0; j < f->nverts; j++) {
      v = o.vlist[f->verts[j]];
      if ((fl = (Flist *) malloc(sizeof(Flist))) == NULL) {
	fprintf(stderr, "Error, out of memory.\n");
	exit(-1);
      }
      // Make fl point to the face, and add it to head of linked list
      // so that the vertex knows about this face.
      fl->f = f;
      fl->next = v->faces;
      v->faces = fl;
      v->nfaces++;
    }
  }
}


/******************************************************************************
Remove faces that are identical.
******************************************************************************/

void
remove_duplicate_faces()
{
  int f;
  int i,j;
  Vertex *v;
  Flist *f1, *f2;

  for (f=0; f<nFiles; f++) {
    for (i=0; i < os[f].nverts; i++) {
      v = os[f].vlist[i];
      // For each face in vertex list, nuke it if it's a duplicate
      for (f1 = v->faces; f1 != NULL; f1 = f1->next) {
	if (f1->f->nverts == 0) continue;
	for (f2 = f1->next; f2 != NULL; f2 = f2->next) {
	  if (f2->f->nverts == 0) continue;
	  if (f1->f->nverts != f2->f->nverts) continue;
	  for (int offset=0; offset < f1->f->nverts; offset++) {
	    bool diffFound = false;
	    for (j=0; j < f1->f->nverts; j++) {
	      Vertex *v1 = f1->f->vptrs[j];
	      Vertex *v2 = f2->f->vptrs[(j+offset)%f1->f->nverts];
	      if (v1->shared != v2->shared) {
		diffFound = true;
		break;
	      }
	    }	
	    // If we get here, this particular offset gives a match
	    if (!diffFound) f2->f->nverts = 0;
	  }
	}
      }
    }
  }
}


/******************************************************************************
Figure out which vertices are close enough to share.
******************************************************************************/

void
share_vertices()
{
  int f;
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;

  int totalfaces = 0;
  for (i=0; i < nFiles; i++) {
    totalfaces += os[i].nverts;
  }
  table = init_table (totalfaces, tolerance);

  squared_dist = tolerance * tolerance;

  /* place all vertices in the hash table, and in the process */
  /* learn which ones should be shared */

  for (f=0; f < nFiles; f++) {
    for (k = 0; k < os[f].nverts; k++) {
      add_to_hash (os[f].vlist[k], table, squared_dist);
    }
  }

  /* fix up the faces to point to the shared vertices */

  for (f = 0; f < nFiles; f++) {
    for (i = 0; i < os[i].nfaces; i++) {

      face = os[f].flist[i];
    
      /* collapse adjacent vertices that are the same */
      for (j = face->nverts-1; j >= 0; j--) {
	jj = (j+1) % face->nverts;
	if (face->vptrs[j] == face->vptrs[jj]) {
	  face->vptrs[j]->nfaces -=1;
	  for (k = j+1; k < face->nverts - 1; k++) {
	    face->verts[k] = face->verts[k+1];
	    face->vptrs[k] = face->vptrs[k+1];
	  }
	  face->nverts--;
	}
      }

      /* remove any faces with less than three vertices by setting */
      /* its vertex count to zero */

      if (face->nverts < 3) {
	for (j=0; j< face->nverts; j++) {
	  face->vptrs[j]->nfaces -=1;
	}
	face->nverts = 0;
      }
    }
  }
}


/******************************************************************************
Add a vertex to it's hash table.

Entry:
  vert    - vertex to add
  table   - hash table to add to
  sq_dist - squared value of distance tolerance
******************************************************************************/

void 
add_to_hash(Vertex *vert, Hash_Table *table, float sq_dist)
{
  int index;
  int a,b,c;
  int aa,bb,cc;
  float scale;
  Vertex *ptr;
  float dx,dy,dz;
  float sq;
  float min_dist;
  Vertex *min_ptr;

  /* determine which cell the position lies within */

  scale = table->scale;
  aa = (int) floor (vert->x * scale);
  bb = (int) floor (vert->y * scale);
  cc = (int) floor (vert->z * scale);

  /* examine vertices in table to see if we're very close */

  min_dist = 1e20;
  min_ptr = NULL;

  /* look at nine cells, centered at cell containing location */

  for (a = aa-1; a <= aa+1; a++)
  for (b = bb-1; b <= bb+1; b++)
  for (c = cc-1; c <= cc+1; c++) {

    /* compute position in hash table */
    index = (a * PR1 + b * PR2 + c) % table->num_entries;
    if (index < 0)
      index += table->num_entries;

    /* examine all points hashed to this cell */
    for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next) {

      /* distance (squared) to this point */
      dx = ptr->x - vert->x;
      dy = ptr->y - vert->y;
      dz = ptr->z - vert->z;
      sq = dx*dx + dy*dy + dz*dz;

      /* maybe we've found new closest point */
      if (sq <= min_dist) {
        min_dist = sq;
        min_ptr = ptr;
      }
    }
  }

  /* If we found a match, have new vertex point to the matching vertex. */
  /* If we didn't find a match, add new vertex to the table. */

  if (min_ptr && min_dist < sq_dist) {  /* match */
    // set shared pointer
    vert->shared = min_ptr;
    // vert->shared gets all of the flist
    while (vert->faces != NULL) {
      // remove face node from vert
      Flist *fl = vert->faces;
      vert->faces = vert->faces->next;
      // insert node in vert->shared
      fl->next = vert->shared->faces;
      vert->shared->faces = fl;
    }
  }
  else {          /* no match */
    index = (aa * PR1 + bb * PR2 + cc) % table->num_entries;
    if (index < 0)
      index += table->num_entries;
    vert->next = table->verts[index];
    table->verts[index] = vert;
    vert->shared = vert;  /* self-reference as close match */
  }
}


/******************************************************************************
Initialize a uniform spatial subdivision table.  This structure divides
3-space into cubical cells and deposits points into their appropriate
cells.  It uses hashing to make the table a one-dimensional array.

Entry:
  nverts - number of vertices that will be placed in the table
  size   - size of a cell

Exit:
  returns pointer to hash table
******************************************************************************/

Hash_Table *
init_table(int nverts, float size)
{
  int i;
  int index;
  int a,b,c;
  Hash_Table *table;
  float scale;

  /* allocate new hash table */

  table = (Hash_Table *) malloc (sizeof (Hash_Table));

  if (nverts < TABLE_SIZE1)
    table->num_entries = TABLE_SIZE1;
  else if (nverts < TABLE_SIZE2)
    table->num_entries = TABLE_SIZE2;
  else if (nverts < TABLE_SIZE3)
    table->num_entries = TABLE_SIZE3;
  else if (nverts < TABLE_SIZE4)
    table->num_entries = TABLE_SIZE4;
  else if (nverts < TABLE_SIZE5)
    table->num_entries = TABLE_SIZE5;
  else if (nverts < TABLE_SIZE6)
    table->num_entries = TABLE_SIZE6;
  else if (nverts < TABLE_SIZE7)
    table->num_entries = TABLE_SIZE7;
  else if (nverts < TABLE_SIZE8)
    table->num_entries = TABLE_SIZE8;
  else if (nverts < TABLE_SIZE9)
    table->num_entries = TABLE_SIZE9;
  else if (nverts < TABLE_SIZE10)
    table->num_entries = TABLE_SIZE10;
  else if (nverts < TABLE_SIZE11)
    table->num_entries = TABLE_SIZE11;
  else if (nverts < TABLE_SIZE12)
    table->num_entries = TABLE_SIZE12;
  else if (nverts < TABLE_SIZE13)
    table->num_entries = TABLE_SIZE13;
  else if (nverts < TABLE_SIZE14)
    table->num_entries = TABLE_SIZE14;
  else if (nverts < TABLE_SIZE15)
    table->num_entries = TABLE_SIZE15;
  else if (nverts < TABLE_SIZE16)
    table->num_entries = TABLE_SIZE16;
  else if (nverts < TABLE_SIZE17)
    table->num_entries = TABLE_SIZE17;
  else if (nverts < TABLE_SIZE18)
    table->num_entries = TABLE_SIZE18;
  else if (nverts < TABLE_SIZE19)
    table->num_entries = TABLE_SIZE19;
  else
    table->num_entries = TABLE_SIZE20;

  table->verts = (Vertex **) malloc (sizeof (Vertex *) * table->num_entries);

  /* set all table elements to NULL */
  for (i = 0; i < table->num_entries; i++)
    table->verts[i] = NULL;

  /* place each point in table */

  scale = 1 / size;
  table->scale = scale;

  return (table);
}


/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/

void
read_file(FILE *inFile, PlyObject &o)
{
  int i,j,k;
  PlyFile *ply;
  int nprops;
  int num_elems;
  char *elem_name;
  float version;
  PlyProperty **plist;
  o.other_elements = NULL;

  /*** Read in the original PLY object ***/

  ply  = ply_read (inFile, &(o.nelems), &(o.elist));
  ply_get_info (ply, &(version), &(o.file_type));

  for (i = 0; i < o.nelems; i++) {

    /* get the description of the first element */
    elem_name = o.elist[i];
    plist = ply_get_element_description(ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      o.vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      o.nverts = num_elems;

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      o.vert_other = ply_get_other_properties (ply, elem_name,
						offsetof(Vertex,other_props));

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        o.vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) o.vlist[j]);
	o.vlist[j]->index = j;
	o.vlist[j]->faces = NULL;
      }
      


    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      o.flist = (Face **) malloc (sizeof (Face *) * num_elems);
      o.nfaces = num_elems;

      /* set up for getting face elements */

      ply_get_property (ply, elem_name, &face_props[0]);
      o.face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        o.flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) o.flist[j]);
      }
    }
    else
      o.other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  o.comments = ply_get_comments (ply, &(o.num_comments));
  o.obj_info = ply_get_obj_info (ply, &(o.num_obj_info));

  ply_close (ply);
}


/******************************************************************************
Write out the PLY file to standard out.
******************************************************************************/

void
write_file()
{
  int f;
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, os[0].nelems, os[0].elist, os[0].file_type);

  /* count the vertices that are in the hash table */
  vert_count = 0;
  for (f=0; f < nFiles; f++) {
    for (i = 0; i < os[f].nverts; i++) {
      if (os[f].vlist[i]->shared == os[f].vlist[i]) {
	// renumber vertices.
	os[f].vlist[i]->index = vert_count;
	vert_count++;
      }
    }
  }

  /* count the faces that do not have matches */
  face_count = 0;
  for (f=0; f < nFiles; f++) {
    for (i = 0; i < os[f].nfaces; i++) {
      if (os[f].flist[i]->nverts != 0) {
	face_count++;
      }
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, os[0].vert_other, 
				 offsetof(Vertex,other_props));

  ply_element_count (ply, "face", face_count);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, os[0].face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, os[0].other_elements);

  for (i = 0; i < os[0].num_comments; i++)
    ply_put_comment (ply, os[0].comments[i]);

  for (i = 0; i < os[0].num_obj_info; i++)
    ply_put_obj_info (ply, os[0].obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */

  ply_put_element_setup (ply, "vertex");

  for (f=0; f < nFiles; f++) {
    for (i = 0; i < os[f].nverts; i++) {
      if (os[f].vlist[i]->shared == os[f].vlist[i]) {
	ply_put_element (ply, (void *) os[f].vlist[i]);
	// fprintf(stderr, "Writing vertex %x\n", os[f].vlist[i]);
      } else {
	// fprintf(stderr, "Skipping vertex %x, shared %x\n", 
	//         os[f].vlist[i], os[f].vlist[i]->shared);
      }
    }
  }
  /* set up and write the face elements */

  ply_put_element_setup (ply, "face");

  for (f=0; f < nFiles; f++) {
    for (i = 0; i < os[f].nfaces; i++) {
      if (os[f].flist[i]->nverts == 0) {
	continue;	
      } 
      // Reset the numbering scheme
      for (j = 0; j < os[f].flist[i]->nverts; j++) {
	os[f].flist[i]->verts[j] = os[f].flist[i]->vptrs[j]->shared->index;
      }
      // write out tri
      ply_put_element (ply, (void *) os[f].flist[i]);
    }
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

