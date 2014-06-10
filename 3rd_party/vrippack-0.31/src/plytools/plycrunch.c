/*

Simplify a polygon model by collapsing multiple points together.  This
is basically Jarek Rossignac's method of simplifying polygon models.

This code borrows heavily from "plyshared".

Greg Turk, August 1994

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>


/* user's vertex and face definitions for a polygonal object */
 
typedef struct Vertex {
  float x,y,z;                  /* SUM of coordinates that contributed */
  float nx,ny,nz;               /* SUM of normals that contributed */
  uint diff_r, diff_g, diff_b;   /* SUM of diffuse colors that contributed */
  int count;                    /* number of vertices that contributed */
  int a,b,c;                    /* integer indices used in hash */
  int index;
  struct Vertex *shared;
  struct Vertex *next;
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


#define MAX_VERT_PROPS 20

static PlyProperty vert_prop_x =  
   {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_y =  
  {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_z =  
  {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_prop_nx =  
   {"nx", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_ny =  
  {"ny", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_nz =  
  {"nz", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_prop_diff_r =  
  {"diffuse_red", PLY_UCHAR, PLY_UINT, 0, 0, PLY_START_TYPE, 
   PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_g =  
  {"diffuse_green", PLY_UCHAR, PLY_UINT, 0, 0, PLY_START_TYPE, 
   PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_b =  
  {"diffuse_blue", PLY_UCHAR, PLY_UINT, 0, 0, PLY_START_TYPE, 
   PLY_START_TYPE, 0};

static int num_vert_props;
static PlyProperty vert_props[MAX_VERT_PROPS];

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


/*** the PLY object ***/

static int nverts,nfaces;
static int n_final_verts, n_final_faces;
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

static float tolerance = 0.0001;   /* what constitutes "near" */

static int HAS_DIFFUSE_COLORS = 0;
static int HAS_NORMALS = 0;


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
void read_file(FILE *inFile);
void crunch_vertices();
float compute_average_edge_length();


clock_t tm;
static void start_time();
static void end_time();
static float time_elapsed();


/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j,got_tolerance;
  char *s;
  char *progname;
  float avg_edge_length;
  FILE *inFile = stdin;

  progname = argv[0];

  /* parse -flags */
  got_tolerance = 0;
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 'd':
	  /* Make sure a second argument */
	  if (argc == 1) {
	    usage(progname);
	    exit(-1);
	  }
          tolerance = atof (*++argv);
          argc -= 1;
	  got_tolerance = 1;
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



  start_time();
  read_file(inFile);
  end_time();

#ifdef VERBOSE
  fprintf(stderr, 
	  "Input file has %d vertices and %d faces\n",
	  nverts, nfaces);
  fprintf(stderr, "Took %f seconds to read file.\n", time_elapsed());
#endif

  if (!got_tolerance) {
      start_time();
      avg_edge_length = compute_average_edge_length();
      tolerance = avg_edge_length*2;
      end_time();
#ifdef VERBOSE
      fprintf(stderr, 
	      "Took %f seconds to compute average edge length.\n", 
	      time_elapsed());
#endif
  }

  start_time();
  crunch_vertices();
  end_time();
#ifdef VERBOSE
  fprintf(stderr, "Took %f seconds to simplify mesh.\n", time_elapsed());
#endif

  start_time();
  write_file();
  end_time();
#ifdef VERBOSE
  fprintf(stderr, "Took %f seconds to write file.\n", time_elapsed());
  fprintf(stderr, 
	  "Output file has %d vertices and %d faces\n",
	  n_final_verts, n_final_faces);
#endif

  exit(0);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s [flags] < in.ply > out.ply\n", progname);
  fprintf (stderr, "       -d distance (default = %g)\n", tolerance);
}


/******************************************************************************
Figure out which vertices should be collapsed into one.
******************************************************************************/

void
crunch_vertices()
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;
  float recip, nrecip;

  table = init_table (nverts, tolerance);

  squared_dist = tolerance * tolerance;

  /* place all vertices in the hash table, and in the process */
  /* learn which ones should be collapsed */

  for (i = 0; i < nverts; i++) {
    vlist[i]->count = 1;
    add_to_hash (vlist[i], table, squared_dist);
  }

  /* compute average of all coordinates that contributed to */
  /* the vertices placed in the hash table */

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    if (vert->shared == vert) {
      recip = 1.0 / vert->count;
      vert->x *= recip;
      vert->y *= recip;
      vert->z *= recip;

      if (HAS_NORMALS) {
	  squared_dist = 
	      vert->nx*vert->nx + vert->ny*vert->ny + vert->nz*vert->nz;
	  if (squared_dist == 0)
	      nrecip = 0;
	  else 
	      nrecip = 1/sqrt(squared_dist);
	  vert->nx *= nrecip;
	  vert->ny *= nrecip;
	  vert->nz *= nrecip;
      }

      if (HAS_DIFFUSE_COLORS) {
	  vert->diff_r = (int)(vert->diff_r*recip + 0.5);
	  vert->diff_g = (int)(vert->diff_g*recip + 0.5);
	  vert->diff_b = (int)(vert->diff_b*recip + 0.5);
      }
    }
  }

  /* fix up the faces to point to the collapsed vertices */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    
    /* change all indices to pointers to the collapsed vertices */
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = (int) (vlist[face->verts[j]]->shared);

    /* collapse adjacent vertices in a face that are the same */
    for (j = face->nverts-1; j >= 0; j--) {
      jj = (j+1) % face->nverts;
      if (face->verts[j] == face->verts[jj]) {
        for (k = j+1; k < face->nverts - 1; k++)
          face->verts[k] = face->verts[k+1];
        face->nverts--;
      }
    }

    /* remove any faces with less than three vertices by setting */
    /* its vertex count to zero */

    if (face->nverts < 3)
      face->nverts = 0;

  }
}

 
/******************************************************************************
Compute the average edge length.  Currently loops through faces and
visits all shared edges twice, giving them double wieghting with respect
to boundary edges.
******************************************************************************/

float 
compute_average_edge_length()
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert1, *vert2;
  Face *face;
  float total_length;
  int num_edges;
  float avg_edge_length;

  total_length = 0;
  num_edges = 0;

  for (i = 0; i < nfaces; i++) {
    face = flist[i];
    for (j = 0; j < face->nverts-1; j++) {
	vert1 = vlist[face->verts[j]];
	vert2 = vlist[face->verts[j+1]];
	total_length += sqrt((vert1->x - vert2->x)*(vert1->x - vert2->x) +
			     (vert1->y - vert2->y)*(vert1->y - vert2->y) +
			     (vert1->z - vert2->z)*(vert1->z - vert2->z));
	num_edges++;
    }
    vert1 = vlist[face->verts[face->nverts-1]];
    vert2 = vlist[face->verts[0]];
    total_length += sqrt((vert1->x - vert2->x)*(vert1->x - vert2->x) +
			 (vert1->y - vert2->y)*(vert1->y - vert2->y) +
			 (vert1->z - vert2->z)*(vert1->z - vert2->z));
    num_edges++;
  }

  avg_edge_length = total_length/num_edges;

  return avg_edge_length;
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
  a = floor (vert->x * scale);
  b = floor (vert->y * scale);
  c = floor (vert->z * scale);
  index = (a * PR1 + b * PR2 + c) % table->num_entries;
  if (index < 0)
    index += table->num_entries;

  /* examine all points hashed to this cell, looking for */
  /* a vertex to collapse with */

  for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next)
    if (a == ptr->a && b == ptr->b && c == ptr->c) {
      /* add to sums of coordinates (that later will be averaged) */
      ptr->x += vert->x;
      ptr->y += vert->y;
      ptr->z += vert->z;

      if (HAS_NORMALS) {
	  ptr->nx += vert->nx;
	  ptr->ny += vert->ny;
	  ptr->nz += vert->nz;
      }

      if (HAS_DIFFUSE_COLORS) {
	  ptr->diff_r += vert->diff_r;
	  ptr->diff_g += vert->diff_g;
	  ptr->diff_b += vert->diff_b;
      }

      ptr->count++;
      vert->shared = ptr;
      return;
    }

  /* no match if we get here, so add new hash table entry */

  vert->next = table->verts[index];
  table->verts[index] = vert;
  vert->shared = vert;  /* self-reference as close match */
  vert->a = a;
  vert->b = b;
  vert->c = c;
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
read_file(FILE *inFile)
{
  int i,j,k;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;
  int nvp;


  /*** Read in the original PLY object ***/


  ply  = ply_read (inFile, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  nvp = 0;

  HAS_DIFFUSE_COLORS = 0;
  HAS_NORMALS = 0;

  if (ply_is_valid_property(ply, "vertex", vert_prop_x.name) &&
      ply_is_valid_property(ply, "vertex", vert_prop_y.name) &&
      ply_is_valid_property(ply, "vertex", vert_prop_z.name)) {

      vert_props[nvp] = vert_prop_x;
      vert_props[nvp].offset = offsetof(Vertex,x); nvp++;
      vert_props[nvp] = vert_prop_y;
      vert_props[nvp].offset = offsetof(Vertex,y); nvp++;
      vert_props[nvp] = vert_prop_z;
      vert_props[nvp].offset = offsetof(Vertex,z); nvp++;
  }
  
  if (ply_is_valid_property(ply, "vertex", vert_prop_nx.name) &&
      ply_is_valid_property(ply, "vertex", vert_prop_ny.name) &&
      ply_is_valid_property(ply, "vertex", vert_prop_nz.name) ) {

      HAS_NORMALS = 1;
      vert_props[nvp] = vert_prop_nx;
      vert_props[nvp].offset = offsetof(Vertex,nx); nvp++;
      vert_props[nvp] = vert_prop_ny;
      vert_props[nvp].offset = offsetof(Vertex,ny); nvp++;
      vert_props[nvp] = vert_prop_nz;
      vert_props[nvp].offset = offsetof(Vertex,nz); nvp++;
  }
  
  if (ply_is_valid_property(ply, "vertex", "diffuse_red") &&
      ply_is_valid_property(ply, "vertex", "diffuse_green") &&
      ply_is_valid_property(ply, "vertex", "diffuse_blue")) {

      HAS_DIFFUSE_COLORS = 1;
      vert_props[nvp] = vert_prop_diff_r;
      vert_props[nvp].offset = offsetof(Vertex,diff_r); nvp++;
      vert_props[nvp] = vert_prop_diff_g;
      vert_props[nvp].offset = offsetof(Vertex,diff_g); nvp++;
      vert_props[nvp] = vert_prop_diff_b;
      vert_props[nvp].offset = offsetof(Vertex,diff_b); nvp++;
    }

  num_vert_props = nvp;

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */

      /* set up for getting vertex elements */
      for (j = 0; j < num_vert_props; j++) {
	  ply_get_property (ply, elem_name, &vert_props[j]);
      }
/*
      ply_get_element_setup (ply, elem_name, num_vert_props, vert_props);
      */
/*
      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      */
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
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);

  /* count the vertices that are in the hash table */

  vert_count = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->shared == vlist[i]) {
      vlist[i]->index = vert_count;
      vert_count++;
    }
  n_final_verts = vert_count;


  /* count the faces that have not been collapsed */

  face_count = 0;
  for (i = 0; i < nfaces; i++)
    if (flist[i]->nverts != 0)
      face_count++;

  n_final_faces = face_count;

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  for (j = 0; j < num_vert_props; j++) {
      ply_describe_property (ply, "vertex", &vert_props[j]);
  }
/*
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  */
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
    if (vlist[i]->shared == vlist[i])
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */

  ply_put_element_setup (ply, "face");

  for (i = 0; i < nfaces; i++) {
    if (flist[i]->nverts == 0)
      continue;
    for (j = 0; j < flist[i]->nverts; j++)
      flist[i]->verts[j] = ((Vertex *) flist[i]->verts[j])->index;
    ply_put_element (ply, (void *) flist[i]);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}


static void
start_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime;
}

static void
end_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime - tm;
}

static float
time_elapsed()
{
    return (double) tm / (double) CLOCKS_PER_SEC;
}
