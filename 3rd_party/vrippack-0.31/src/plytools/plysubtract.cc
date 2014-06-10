/*
plysubtract:
  Read in two ply files, A and B, and write out A-B, namely, 
  all triangles in A that are not in B.

Algorithm:
  Read in A and B.
  For each vertex in B, find the closest vertex in A
  (within a limited neighborhood search size).  Then, for each
  triangle in B, search A for a corresponding triangle to nuke.
  Finally, write out the non-nuked triangles.

Authors:
  Lucas Pereira, 1-13-99
  based on plyshared, written by Greg Turk, Aug 1994
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <ply.h>


/* user's vertex and face definitions for a polygonal object */


struct _Face;
typedef struct _Flist {
  struct _Face *f;
  struct _Flist *next;
} Flist;

typedef struct Vertex {
  float x,y,z;
  int index;
  bool isA;		   // True if vertex belongs to mesh A
  struct Vertex *closest;  // Points to the closest vertex in the other mesh
  struct Vertex *next;
  void *other_props;       /* other properties */
  int nfaces;
  Flist *faces;
} Vertex;

typedef struct _Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  Vertex **vptrs;           /* vertex pointer list */
  void *other_props;       /* other properties */
  bool has_match;	   // does a match exist in the other file?
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

static PlyObject A;
static PlyObject B;

static bool DO_INTERSECTION = false;  // if true, intersect, not subtract
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
void write_file(PlyObject &o);
void read_file(FILE *inFile, PlyObject &o);
void mark_mesh(PlyObject &o, bool isA);
void set_flists(PlyObject &o);
void set_vptrs(PlyObject &o);
bool find_match(Face *f, PlyObject &A, PlyObject &B);
void subtract_vertices(PlyObject &A, PlyObject &B);


/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  FILE *inA;
  FILE *inB;

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
        case 'i':
	  DO_INTERSECTION = true;
	  break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  /* both input files must be specified on the command line */
  if (argc > 1 && *argv[0] != '-' && *argv[1] != '-') {
    inA = fopen(argv[0], "r");
    if (inA == NULL) {
      fprintf(stderr, "Error: Couldn't open input file %s\n", argv[0]);
      usage(progname);
      exit(-1);
    }
    inB = fopen(argv[1], "r");
    if (inB == NULL) {
      fprintf(stderr, "Error: Couldn't open input file %s\n", argv[1]);
      usage(progname);
      exit(-1);
    }
    argc -=2;
    argv +=2;
  } else {
    usage(progname);
  }

   /* Check no extra args */
   if (argc > 0) {
     fprintf(stderr, "Error: Unhandled arg: %s\n", argv[0]);
     usage(progname);
     exit(-1);
   }

  read_file(inA, A);
  mark_mesh(A, true);
  read_file(inB, B);
  mark_mesh(B, false);

  // Make Face vptrs point to verts
  set_vptrs(A);
  set_vptrs(B);

  // Make B vertices point to B triangles
  set_flists(A);
  set_flists(B);

  // For each triangle of A
  subtract_vertices(A, B);

  write_file(A);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "\n");
  fprintf (stderr, "usage: %s [flags] [A.ply] [B.ply] > out.ply\n", progname);
  fprintf (stderr, "flags:\n");
  fprintf (stderr, "       -i           (does intersection instead of subtraction)\n");
  fprintf (stderr, "       -t tolerance (default = %g)\n", tolerance);
  fprintf (stderr, "\n");
  fprintf (stderr, "This program computes (A-B), namely, those faces in A\n");
  fprintf (stderr, "that do not occur in B.  \"Face equivalence\" is defined\n");
  fprintf (stderr, "as two faces, a' and b', such that every vertex in a' has\n");
  fprintf (stderr, "a 1-1 mapping to vertices in b'.  The distance between\n");
  fprintf (stderr, "corresponding pairs must be less than tolerance, and for each\n");
  fprintf (stderr, "vertex of b', the corresponding a' vertex must be the closest\n");
  fprintf (stderr, "vertex in A.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "With the -i flag, it outputs (A^B), with the same definition\n");
  fprintf (stderr, "for face equivalence.\n");
  fprintf (stderr, "\n");
  exit(-1);
}

/******************************************************************************
Mark vertices as belonging to A or not...
******************************************************************************/

void
mark_mesh(PlyObject &o, bool isA) {
  int i;
  for (i=0; i < o.nverts; i++) {
    o.vlist[i]->isA = isA;
  }
}


/******************************************************************************
Make face vptrs point to the vertices...
******************************************************************************/

void
set_vptrs(PlyObject &o) {
  int i,j;
  Face *f;
  Vertex *v;
  Flist *fl;

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
See if face has a match in B...
******************************************************************************/

bool 
find_match(Face *f, PlyObject &A, PlyObject &B) {
  int i;
  Vertex *v;
  Flist *fl;
  Face *of;

  if (!f->nverts) return false;
  
  // Find v, the matching vertex for the first vertex in f
  v = A.vlist[f->verts[0]]->closest;
  if (v==NULL) return false;
  
  for (fl = v->faces;fl != NULL; fl = fl->next) {
    of = fl->f;
    if (of->nverts != f->nverts) continue;
    // See if f, of are matches

    for (int offset=0; offset < f->nverts; offset++) {
    bool diffFound = false;
      for (i=0; i < f->nverts; i++) {
	Vertex *v1 = A.vlist[f->verts[i]];
	Vertex *v2 = B.vlist[of->verts[(i+offset)%f->nverts]];
  	if (v1->closest != v2) {
	  diffFound = true;
	  break;
	}
      }
      // If we get here, this particular offset gives a match
      if (!diffFound) return true;
    }
  }

  // if we get here, didn't find a match, so return false
  return false;

}

/******************************************************************************
Figure out which vertices are close enough to share.
******************************************************************************/

void
subtract_vertices(PlyObject &A, PlyObject &B)
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;

  table = init_table (A.nverts+B.nverts, tolerance);

  squared_dist = tolerance * tolerance;

  /* place all vertices in the hash table, and in the process */
  /* learn which ones should be shared */

  for (i = 0; i < A.nverts; i++)
    add_to_hash (A.vlist[i], table, squared_dist);
  for (i = 0; i < B.nverts; i++)
    add_to_hash (B.vlist[i], table, squared_dist);

  // For each triangle in A, see if it has a match in B
  for (i = 0; i < A.nfaces; i++) {
    if (find_match(A.flist[i], A, B)) {
      A.flist[i]->has_match = true;
      // Decrement the face counters for each vertex 
      for (j=0; j < A.flist[i]->nverts; j++) {
	A.vlist[A.flist[i]->verts[j]]->nfaces--;
	if (DO_INTERSECTION) {
	  // Set completely to 0 for intersection, so it's marked
	  A.vlist[A.flist[i]->verts[j]]->nfaces = 0;
	}
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
  aa = int (floor (vert->x * scale));
  bb = int (floor (vert->y * scale));
  cc = int (floor (vert->z * scale));

  /* examine vertices in table to see if we're very close */

  min_dist = 1e20;
  min_ptr = NULL;

  /* look at nine cells, centered at cell containing location */
  // Do this only for points in B.  A just gets added.
  if (!(vert->isA)) {
    for (a = aa-1; a <= aa+1; a++) {
      for (b = bb-1; b <= bb+1; b++) {
	for (c = cc-1; c <= cc+1; c++) {

	  /* compute position in hash table */
	  index = (a * PR1 + b * PR2 + c) % table->num_entries;
	  if (index < 0)
	    index += table->num_entries;

	  /* examine all points hashed to this cell */
	  for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next) {
	    if (ptr->isA) {
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
	}	
      }
    }
  }

  /* If we found a match, have new vertex point to the matching vertex. */
  /* If we didn't find a match, add new vertex to the table. */

  if (min_ptr && min_dist < sq_dist) {  /* match */
    vert->closest = min_ptr;
    min_ptr->closest = vert;
  }
  else {          /* no match */
    index = (aa * PR1 + bb * PR2 + cc) % table->num_entries;
    if (index < 0)
      index += table->num_entries;
    vert->next = table->verts[index];
    table->verts[index] = vert;
    vert->closest = NULL;
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


  /*** Read in the original PLY object ***/
  o.other_elements = NULL;

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
write_file(PlyObject &o)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, o.nelems, o.elist, o.file_type);

  /* count the vertices that are in the hash table */

  vert_count = 0;
  for (i = 0; i < o.nverts; i++) {
    if ((!DO_INTERSECTION && o.vlist[i]->nfaces > 0) || 
	(DO_INTERSECTION && !(o.vlist[i]->nfaces > 0))) {
      // renumber vertices.
      o.vlist[i]->index = vert_count;
      vert_count++;
    }
  }

  /* count the faces that do not have matches */
  face_count = 0;
  for (i = 0; i < o.nfaces; i++) {
    if ((!DO_INTERSECTION && o.flist[i]->has_match == false) ||
	(DO_INTERSECTION && !(o.flist[i]->has_match == false))) {
      face_count++;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, o.vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", face_count);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, o.face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, o.other_elements);

  for (i = 0; i < o.num_comments; i++)
    ply_put_comment (ply, o.comments[i]);

  for (i = 0; i < o.num_obj_info; i++)
    ply_put_obj_info (ply, o.obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */

  ply_put_element_setup (ply, "vertex");

  for (i = 0; i < o.nverts; i++) {
    if ((!DO_INTERSECTION && o.vlist[i]->nfaces > 0) || 
	(DO_INTERSECTION && !(o.vlist[i]->nfaces > 0))) {
      ply_put_element (ply, (void *) o.vlist[i]);
    }
  }
  /* set up and write the face elements */

  ply_put_element_setup (ply, "face");

  for (i = 0; i < o.nfaces; i++) {
    if ((!DO_INTERSECTION && o.flist[i]->has_match == true) ||
	(DO_INTERSECTION && !(o.flist[i]->has_match == true))) {
      continue;
    }
    // Reset the numbering scheme
    for (j = 0; j < o.flist[i]->nverts; j++) {
      o.flist[i]->verts[j] = o.flist[i]->vptrs[j]->index;
    }
    // write out tri
    ply_put_element (ply, (void *) o.flist[i]);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

