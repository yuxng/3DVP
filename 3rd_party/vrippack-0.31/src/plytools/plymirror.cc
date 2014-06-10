// plymirror
// 
// Given that we have matched a mirror object to the mesh,
// Read the mirror .xf and .mir files to figure out which
// points were seen through the mirror, and mirror 'em.
// (It also puts the scene in roughly the right place, but
// you really should do alignment, because this depends on
// how accurately we match the angle of the mirror frame,
// and how closely the mirror frame model matches the actual
// mirror (which might have the mirror surface off by several
// mm...)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <ply.h>
#include <string.h>

#include "Linear.h"


/* user's vertex and face definitions for a polygonal object */
typedef struct Vertex {
  float x,y,z;
  void *other_props;       /* other properties */
  bool valid;
  int newindex;
} Vertex;

typedef struct Face {
  unsigned char num_pts;
  int *v;
  int *origv;  // filled in if we overwrite v with new index numbers...
  void *other_props;       /* other properties */
  bool valid;
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face, v),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,num_pts)},
};


/*** the PLY object ***/
int nverts;
Vertex **vlist;

int nfaces;
Face **flist;

PlyOtherElems *other_elements = NULL;
PlyOtherProp *vert_other;
PlyOtherProp *face_other;
int nelems;
char **elist;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
int file_type;

// Mirror info
Vec3f mir[4];
Matrix4f mirxf;

// Flags
bool mirAll = FALSE;
bool xfNone = FALSE;

// helper functions
void usage(char *progName);

void read_mir(FILE *mirFile);
void read_xf(FILE *xfFile);
void read_file(FILE *inFile);
void mirror(FILE *outxfFile);
void select_rest();
void flipValidNorms();
void renumberValidVertices();
void write_file(FILE *outFile);


int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  FILE *outFile = stdout;
  FILE *outxfFile = stderr;
  FILE *restFile = NULL;  // Output for the "rest" (non-mirrored stuff)
  char *progName = argv[0];

  char *mirName = NULL;
  char *xfName = NULL;
  char *inName = NULL;
  FILE *mirFile = NULL;
  FILE *xfFile = NULL;
  FILE *inFile = stdin;


  /* Parse -flags */
  while (--argc > 0) {
    argv++;
    if (!strcmp(*argv, "-h")) {
      usage(progName);
      exit(-1);
    } else if (!strcmp(*argv, "-o")) {
      // Redirect mirrored output to file
      if (argc < 2) usage(progName);
      argc--; argv++;
      outFile = fopen(argv[0], "w");
      if (outFile == NULL) {
	fprintf(stderr, "Error: Couldn't open output file %s\n", argv[0]);
	usage(progName);
	exit(-1);
      }
      // Also open up a .xf file, if we can
      int namelen = strlen(argv[0]);
      if (namelen > 4 && !strcmp(&(argv[0][namelen-4]), ".ply")) {
	// Write over .ply extension with .xf
	sprintf(&(argv[0][namelen-4]), ".xf");
	outxfFile = fopen(argv[0], "w");
	if (outFile == NULL) {
	  fprintf(stderr, "Warning: Couldn't open output .xf file %s.\n",
		  argv[0]);
	  outxfFile = stderr;
	} else {
	  fprintf(stderr, "Writing xform to %s\n", argv[0]);
	}
      }

    } else if (!strcmp(*argv, "-r")) {
      // Redirect UNmirrored output to file
      if (argc < 2) usage(progName);
      argc--; argv++;
      restFile = fopen(argv[0], "w");
      if (outFile == NULL) {
	fprintf(stderr, "Error: Couldn't open unmirrored output file %s\n",
		argv[0]);
	usage(progName);
	exit(-1);
      }
    } else if (argv[0][0] != '-') {
      // Handle mirName, xfName, inName files....
      if (mirName == NULL) {
	mirName = argv[0];
      } else if (xfName == NULL) {
	xfName = argv[0];
      } else if (inName == NULL) {
	inName = argv[0];
      } else {
	fprintf(stderr, "Error, Unhandled arg '%s'\n", argv[0]);
	usage(progName);
	exit(-1);
      }
    } else {
      // Unhandled arg
      fprintf(stderr, "Error, Unhandled arg '%s'\n", argv[0]);
      usage(progName);
      exit(-1);
    }
  }

  // Open mir file...
  if (mirName == NULL) {
    fprintf(stderr, "Missing arg:  .mir file.\n");
    usage(progName);
    exit(-1);
  } else if (!strcmp(mirName, "all")) {
    mirAll = TRUE;
  } else {
    mirFile = fopen(mirName, "r");
    if (mirFile == NULL) {
      fprintf(stderr, "Error: Couldn't open .mir file %s\n", mirName);
      usage(progName);
      exit(-1);
    }
  }

  // Open xf file...
  if (xfName == NULL) {
    fprintf(stderr, "Missing arg:  .xf file.\n");
    usage(progName);
    exit(-1);
  } else if (!strcmp(xfName, "none")) {
    xfNone = TRUE;
  } else {
    xfFile = fopen(xfName, "r");
    if (xfFile == NULL) {
      fprintf(stderr, "Error: Couldn't open .xf file %s\n", xfName);
      usage(progName);
      exit(-1);
    }
  }

  // Open (optional) input file...
  if (inName != NULL) {
    inFile = fopen(inName, "r");
    if (inFile == NULL) {
      fprintf(stderr, "Error: Couldn't open input file %s\n", inName);
      usage(progName);
      exit(-1);
    }
  }

  
  read_mir(mirFile);
  read_xf(xfFile);
  read_file(inFile);

  mirror(outxfFile);

  write_file(outFile);
  if (restFile != NULL) {
    select_rest();
    write_file(restFile);
  }

}


/******************************************************************************
Transform the PLY object.
******************************************************************************/

void
select_rest() 
{
  int i;
  Vertex *vert;
  // Select all the vertices, etc OUTSIDE the mirror

  // First, set every vert flag to valid.
  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vert->valid = TRUE;
  }

  // Now remove all the points that are behind or
  // within epsilon of the mirror surface.
  // (to get rid of the frame, any supporting 
  // structure, etc...)
  Vec3f miraxes[2];
  Vec3f mirnorm;
  float mirD;    // Distance along normal to the mirror surface

  miraxes[0] = mir[1] - mir[0];
  miraxes[1] = mir[2] - mir[1];
  mirnorm = miraxes[0].cross(miraxes[1]);
  mirnorm.normalize();
  mirD = mirnorm.dot(mir[0]);

#define REST_EPSILON 0.06

  Vec3f pos;
  float d;

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    pos.setValue(vert->x, vert->y, vert->z);
    d = mirnorm.dot(pos);
    if (mirAll || d < (mirD + REST_EPSILON)) {
      vlist[i]->valid = FALSE;
    }
  }

  renumberValidVertices();
}

void
mirror(FILE *outxfFile)
{
  Vec3f miraxes[2];
  Vec3f mirnorm;
  float mirD;    // Distance along normal to the mirror surface

  miraxes[0] = mir[1] - mir[0];
  miraxes[1] = mir[2] - mir[1];
  mirnorm = miraxes[0].cross(miraxes[1]);
  mirnorm.normalize();

  // fprintf(stderr, "Mirror normal:  %f %f %f\n",
  //	  mirnorm[0], mirnorm[1], mirnorm[2]);

  mirD = mirnorm.dot(mir[0]);
  // First clip all the points to be farther than the mirror
  // (plus an epsilon)

#define EPSILON 0.03

  int i;
  Vertex *vert;
  Vec3f pos;
  float d;

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    pos.setValue(vert->x, vert->y, vert->z);
    d = mirnorm.dot(pos);
    if (!mirAll && d > (mirD - EPSILON)) {
      vlist[i]->valid = FALSE;
    }
  }

  // Now clip against the four sides of the mirror.
  Vec3f *v1, *v2, sidenorm;
  for (int side=0; side < 4; side++) {
    v1 = &(mir[side]);
    v2 = &(mir[(side+1)%4]);
    // Sidenorm points outward from mirror boundary.
    // This assumes that we're clipping outside the frustum
    // emanating from the origin (0,0,0)
    sidenorm = v1->cross(*v2);
    for (i = 0; i < nverts; i++) {
      vert = vlist[i];
      pos.setValue(vert->x, vert->y, vert->z);
      d = sidenorm.dot(pos);
      if (!mirAll && d > 0) {
	vlist[i]->valid = FALSE;
      }	
    }
  }
    
  // Now, if the point is still valid, reflect it in the mirror.
  Vec3f newpos, diff;
  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    if (vert->valid) {
      vert->x *= -1.0;
    }
  }

  // Compute the new .xf for the mirrored points:
  Matrix4f outxf;
  outxf.makeIdentity();

  // Basically, here, we put a vertex at the end of each of 
  // the basis vectors.  Then we reflect it through the mirror,
  // subtract the origin, and we are left with the new basis
  // vectors for the mirrored scan, which also form the rotation
  // matrix for the .xf file.
  Vec3f _000(0,0,0);
  Vec3f _100(1,0,0);
  Vec3f _010(0,1,0);
  Vec3f _001(0,0,1);
  float d000 = 2.0*(mirD - mirnorm.dot(_000));
  float d100 = 2.0*(mirD - mirnorm.dot(_100));
  float d010 = 2.0*(mirD - mirnorm.dot(_010));
  float d001 = 2.0*(mirD - mirnorm.dot(_001));
  // fprintf(stderr, "distances: %f %f %f %f\n", d000, d100, d010, d001);
  Vec3f t000(mirnorm);
  Vec3f t100(mirnorm);
  Vec3f t010(mirnorm);
  Vec3f t001(mirnorm);
  t000 *= d000;
  t100 *= d100;
  t010 *= d010;
  t001 *= d001;
  _000 += t000;
  _100 += t100;
  _010 += t010;
  _001 += t001;

  _100 -= _000;
  _010 -= _000;
  _001 -= _000;

  Matrix4f rot;
  rot.makeIdentity();
  rot.m[0][0] = -_100[0];
  rot.m[0][1] = _100[1];
  rot.m[0][2] = _100[2];
  rot.m[1][0] = -_010[0];
  rot.m[1][1] = _010[1];
  rot.m[1][2] = _010[2];
  rot.m[2][0] = -_001[0];
  rot.m[2][1] = _001[1];
  rot.m[2][2] = _001[2];

  outxf.multLeft(rot);

  // Set the translation of the .xf file equal to the translation
  // of the origin through the mirror. 
  Vec3f trans(mirnorm);
  trans *= (mirD * 2.0);
  outxf.setTranslate(trans);

  // Print out the new matrix for it to outxfFile
  for (i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      fprintf(outxfFile, "%f ", outxf.m[i][j]);
    }
    fprintf(outxfFile, "\n");
  }
  

  // Now we need to renumber all the valid vertices, so that
  // they form a contiguous list
  renumberValidVertices();
  flipValidNorms();
}

void flipValidNorms()
{
  // Mirored triangles have backwards normals, so reset them...
  int i;
  for (i=0; i < nfaces; i++) {
    Face *face = flist[i];
    if (face->valid && face->origv != NULL) {
      for (int j=0; j < face->num_pts; j++) {
	face->v[j] = vlist[face->origv[face->num_pts-(j+1)]]->newindex;
      }
    }
  }
}

void renumberValidVertices() 
{
  int i;
  Vertex *vert;
  // Go through all the vertices and reassign new index numbers
  int num=0;
  for (i=0; i < nverts; i++) {
    if (vlist[i]->valid) {
      vlist[i]->newindex = num++;
    }
  }

  // Go through all the triangles -- if they touch invalid points,
  // then invalidate the triangle...  If the point is valid, grab
  // the new index number... :-) 
  for (i=0; i < nfaces; i++) {
    Face *face = flist[i];
    // Allocate an origv array, containing the old index numbers
    if (face->origv == NULL) {
      face->origv = (int *) malloc(sizeof(int) * face->num_pts);
      for (int j=0; j < face->num_pts; j++) {
	face->origv[j] = face->v[j];
      }
    }

    // Set the face valid, and then invalidate if it touches any
    // invalid points...
    face->valid = TRUE;
    for (int j=0; j < face->num_pts; j++) {
      vert = vlist[face->v[j]];
      if (vert->valid) {
	// get the new index. 
	face->v[j] = vlist[face->origv[j]]->newindex;
      } else {
	face->valid = FALSE;
      }
    }
  }
}

/******************************************************************************
Read in the input files
******************************************************************************/

void read_mir(FILE *mirFile)
{

  // If we're mirroring everything, then just put the mirror
  // down the Z axis, so that it only flips x coord.
  if (mirAll) {
    mir[0].x = 0; mir[0].y = -1; mir[0].z =  1;
    mir[1].x = 0; mir[1].y = -1; mir[1].z = -1;
    mir[2].x = 0; mir[2].y =  1; mir[2].z = -1;
    mir[3].x = 0; mir[3].y =  1; mir[3].z =  1;
    return;
  }

  // Read mirror file -- must have the coordinates of the 4
  // vertices, one per line
  for (int i=0; i < 4; i++) {
    if (fscanf(mirFile, "%f %f %f\n", &(mir[i][0]), &(mir[i][1]),
	       &(mir[i][2])) != 3) {
      fprintf(stderr, "Error: did not find 3-vector on line %d of mirror"
	      " file.\n", i+1);
      exit(-1);
    }
  }
}

void read_xf(FILE *xfFile)
{

  // If we have no xform, then just load the identity matrix.
  // No need to transform the mirror to new position, cuz it ain't
  // movin.
  if (xfNone) {
    mirxf.makeIdentity();
    return;
  }

  float mat[4][4];

  // Read mirror file -- must have the coordinates of the 4
  // vertices, one per line
  int i;
  for (i=0; i < 4; i++) {
    if (fscanf(xfFile, "%f %f %f %f\n", &(mat[i][0]), &(mat[i][1]),
	       &(mat[i][2]), &(mat[i][3])) != 4) {
      fprintf(stderr, "Error: did not find 4-vector on line %d of matrix"
	      " file.\n", i+1);
      exit(-1);
    }
  }
  
  // Fill the mirror matrix
  mirxf.setValue(mat);

  // Transform the mirror to its new position...
  Vec3f corner;
  for (i=0; i < 4; i++) {
    corner.setValue(mir[i]);
    mirxf.multVec(corner, mir[i]);
  }
}

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
	// all points are initially valid, until we crop to mirror
	vlist[j]->valid = TRUE;
      }
    }

    else if (equal_strings ("face", elem_name)) {

      /* create a face list to hold all the vertices */
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
	// all faces are initially valid, until we crop to mirror
	flist[j]->valid = TRUE;
	flist[j]->origv = NULL;
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
Write out the PLY file to output file.  Write only the valid points.
(Valid points are the ones seen through the mirror.)
******************************************************************************/

void
write_file(FILE *outFile)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  ply = ply_write (outFile, nelems, elist, file_type);

  // Compute number of valid vertices
  int validverts = 0;
  for (i=0; i < nverts; i++) {
    if (vlist[i]->valid) validverts++;
  }

  // Compute number of valid faces
  int validfaces = 0;
  for (i=0; i < nfaces; i++) {
    if (flist[i]->valid) validfaces++;
  }

  /* describe what properties go into the vertex and face elements */
  ply_element_count (ply, "vertex", validverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties(ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", validfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties(ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++) {
    if (vlist[i]->valid) {
      ply_put_element (ply, (void *) vlist[i]);
    }
  }

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++) {
    if (flist[i]->valid) {
      ply_put_element (ply, (void *) flist[i]);
    }
  }

  ply_put_other_elements (ply);

  ply_close (ply);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progName)
{
  fprintf (stderr, 
	   "usage: %s in.mir in.xf [-o out.ply] [-r rest.ply] [in.ply]\n"
	   "   or: %s in.mir in.xf [-o out.ply] [-r rest.ply] < in.ply\n" 
	   "   or: %s all    none  [-o out.ply] [-r rest.ply] < in.ply\n", 
	   progName, progName, progName);
  fprintf (stderr, 
	   "where:\n"
	   "     in.mir is mirror file, four lines of 3 coords, for each\n"
	   "              corner of the mirror glass.  (the keyword\n"
	   "              'all' mirrors the entire scan in x.)\n"
	   "     in.xf is the .xf file moving the mirror file to match\n"
	   "              the position of the mirror in the input file.\n"
	   "              ('none' loads the identity matrix.)\n"
	   "     out.ply is the output file for all points seen through\n"
	   "              the mirror.\n"
	   "     rest.ply is the output file for all points NOT seen\n"
	   "              through the mirror.\n"
	   "     in.ply   is the input ply file.\n"
	   "\n");
  exit(-1);
}
