/*

Apply a 3-D transformation to an object from a PLY file.
Then figure out how it breaks into subvols, and write out the
ply file in chunks, but going back to using the original
vertices.  

It throws away all vertex properties except xyz and confidence
(if confidence exists).

Lucas Pereira, 2-28-99.

Based somewhat on plyxform,
Greg Turk, August 1994
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <ply.h>

#include "Linear.h"

/* user's vertex and face definitions for a polygonal object */
bool hasConfidence = 0;
float subvolSize = 0;
float epsilon = 0;
bool writeDiced = 0;
char *baseName = NULL;
float minx =  1e30;
float miny =  1e30;
float minz =  1e30;
float maxx = -1e30;
float maxy = -1e30;
float maxz = -1e30;
float cropminx = -1e30;
float cropminy = -1e30;
float cropminz = -1e30;
float cropmaxx =  1e30;
float cropmaxy =  1e30;
float cropmaxz =  1e30;
char *outdir = NULL;

typedef struct Vertex {
  float x,y,z;            // xyz in mesh (non-transformed) coords
  float wx, wy, wz;       // xyz in world coordinates (transformed)
  int index;              // "true" index
  int svindex;            // index for this particular subvol
  float confidence;
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
} Face;



char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
  {"confidence", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,confidence), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};

/*** the PLY object ***/

int nverts, nfaces;
int validverts, validfaces;
Vertex **vlist;
Vertex **validvlist;
Face **flist;
Face **validflist;
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
void write_file(FILE *out);
void clip_and_write_subvols();
void read_file(FILE *inFile);
void transform();
int  clip_to_bounds(float svminx, float svminy, float svminz, 
		    float svmaxx, float svmaxy, float svmaxz);

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
  char *writebboxname = NULL;
  bool printbbox = FALSE;

  progname = argv[0];

  quat.q[0] = 0;
  quat.q[1] = 0;
  quat.q[2] = 0;
  quat.q[3] = 1;

  xfmat.makeIdentity();

  // With no args, this program does nothing, so print usage
  if (argc == 1) {
    usage(progname);
    exit(0);
  }

  /* Parse -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    s = argv[0];
    if (equal_strings(s, "-h")) {
      usage(progname);
      exit(0);
    } else if (equal_strings(s, "-writebbox")) {
      if (argc < 2) usage(progname);
      writebboxname = (*++argv);
      argc -= 1;
    } else if (equal_strings(s, "-outdir")) {
      if (argc < 2) usage(progname);
      outdir = (*++argv);
      argc -= 1;
    } else if (equal_strings(s, "-printbbox")) {
      printbbox = TRUE;
    } else if (equal_strings(s, "-dice")) {
      if (argc < 4) usage(progname);
      subvolSize = atof(*++argv);
      epsilon = atof(*++argv);
      writeDiced = 1;
      baseName = (*++argv);
      if (subvolSize <= 0) {
	fprintf(stderr, "Error: subvolSize must be greater than zero.\n");
	usage(progname);
      }
      argc -= 3;
    } else if (equal_strings(s, "-odice")) {
      if (argc < 4) usage(progname);
      subvolSize = atof(*++argv);
      epsilon = atof(*++argv);
      writeDiced = 0;
      baseName = (*++argv);
      if (subvolSize <= 0) {
	fprintf(stderr, "Error: subvolSize must be greater than zero.\n");
	usage(progname);
      }
      argc -= 3;
    } else if (equal_strings(s, "-crop")) {
      if (argc < 7) usage(progname);
      cropminx = atof(*++argv);
      cropminy = atof(*++argv);
      cropminz = atof(*++argv);
      cropmaxx = atof(*++argv);
      cropmaxy = atof(*++argv);
      cropmaxz = atof(*++argv);
      argc -= 6;
    } else if (equal_strings(s, "-s")) {
      if (argc < 4) usage(progname);
      xscale = atof (*++argv);
      yscale = atof (*++argv);
      zscale = atof (*++argv);
      argc -= 3;
    } else if (equal_strings(s, "-f")) {
      if (argc < 2) usage(progname);
      xfname = (*++argv);
      argc-=1;
    } else if (equal_strings(s, "-t")) {
      if (argc < 4) usage(progname);
      xtrans = atof (*++argv);
      ytrans = atof (*++argv);
      ztrans = atof (*++argv);
      argc -= 3;
    } else if (equal_strings(s, "-r")) {
      if (argc < 4) usage(progname);
      rotx = atof (*++argv) * M_PI/180;
      roty = atof (*++argv) * M_PI/180;
      rotz = atof (*++argv) * M_PI/180;
      argc -= 3;
    } else if (equal_strings(s, "-q")) {
      if (argc < 5) usage(progname);
      quat.q[0] = atof (*++argv);
      quat.q[1] = atof (*++argv);
      quat.q[2] = atof (*++argv);
      quat.q[3] = atof (*++argv);
      argc -= 4;
    } else {
      fprintf(stderr, "Error: unrecognized arg: %s.  Aborting...\n", 
	      argv[0]);
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

  if (inFile == stdin) {
    fprintf(stderr, "Reading from stdin...\n");
  }

  read_file(inFile);
  transform();

  // Print bbox to stdout?
  if (printbbox) {
    fprintf(stdout,  "%f %f %f\n%f %f %f\n", minx, miny, minz,
	    maxx, maxy, maxz);
  }

  // Write bbox to file?
  if (writebboxname) {
    FILE *bbox = fopen(writebboxname, "w");
    if (bbox == NULL) {
      fprintf(stderr, "Err, couldn't open %s for writing. Aborting...\n", 
	      writebboxname);
      exit(-1);
    }
    fprintf(bbox, "%f %f %f\n%f %f %f\n", minx, miny, minz,
	    maxx, maxy, maxz);
    fclose(bbox);
  }

  if (subvolSize > 0) {
    clip_and_write_subvols();
  }
  return(0);
}

/******************************************************************************
Write a ply file for each subvol that contains tris.
Assumes that transform has been called, so that bboxes are set....
******************************************************************************/

void
clip_and_write_subvols()
{
  int x, y, z;
  int minxi, minyi, minzi, maxxi, maxyi, maxzi;
  float svminx, svminy, svminz;
  float svmaxx, svmaxy, svmaxz;

  minxi = int(floor((minx - epsilon) / subvolSize));
  minyi = int(floor((miny - epsilon) / subvolSize));
  minzi = int(floor((minz - epsilon) / subvolSize));
  maxxi = int(floor((maxx + epsilon) / subvolSize));
  maxyi = int(floor((maxy + epsilon) / subvolSize));
  maxzi = int(floor((maxz + epsilon) / subvolSize));
    
  for (x = minxi; x <= maxxi; x++) {
    for (y = minyi; y <= maxyi; y++) {
      for (z = minzi; z <= maxzi; z++) {
	svminx = x*subvolSize-epsilon;
	svminy = y*subvolSize-epsilon;
	svminz = z*subvolSize-epsilon;
	svmaxx = (x+1)*subvolSize+epsilon;
	svmaxy = (y+1)*subvolSize+epsilon;
	svmaxz = (z+1)*subvolSize+epsilon;
	
	// Set bounds tighter if bounds exist?
	svminx = MAX(svminx, cropminx-epsilon);
	svminy = MAX(svminy, cropminy-epsilon);
	svminz = MAX(svminz, cropminz-epsilon);
	svmaxx = MIN(svmaxx, cropmaxx+epsilon);
	svmaxy = MIN(svmaxy, cropmaxy+epsilon);
	svmaxz = MIN(svmaxz, cropmaxz+epsilon);

	int numverts = clip_to_bounds(svminx, svminy, svminz,
				     svmaxx, svmaxy, svmaxz);
	if (numverts > 0) {
	  char fname[1000];
	  if (outdir != NULL) {
	    sprintf(fname, "%s/%s_%d_%d_%d.ply", outdir, baseName, x, y, z);
	  } else {
	    sprintf(fname, "%s_%d_%d_%d.ply", baseName, x, y, z);
	  }
	  // Print name to stdout
	  fprintf(stdout, "%s\n", fname);

	  if (writeDiced) {
	    FILE *out = fopen(fname, "w");
	    if (out == NULL) {
	      fprintf(stderr, "Err: couldn't open fname... aborting.\n");
	      exit(-1);
	    }
	    write_file(out);
	    fclose(out);
	  }

	}
      }
    }
  }
}	

/******************************************************************************
Figure out which vertices are in the bbox. Returns number of valid
triangles...
******************************************************************************/

int
clip_to_bounds(float svminx, float svminy, float svminz, 
	       float svmaxx, float svmaxy, float svmaxz)
{
  int i, j;
  Vertex *v;
  Face *f;

  validverts = 0; 
  validfaces = 0;

  // Set new id for all the vertices
  // -1 if outside volume...
  for (i=0; i < nverts; i++) {
    v = vlist[i];
    if (v->wx >= svminx &&
	v->wy >= svminy &&
	v->wz >= svminz &&
	v->wx <= svmaxx &&
	v->wy <= svmaxy &&
	v->wz <= svmaxz) {
      v->svindex = validverts;
      validvlist[validverts++] = v;
    } else {
      v->svindex = -1;
    }
  }

  // Abort right here if not at least 3 vertices...
  // fprintf(stderr, "valid verts: %d\n", validverts);
  if (validverts < 3) return 0;
  
  for (i=0; i < nfaces; i++) {
    f = flist[i];
    bool good = TRUE;
    for (j=0; j < f->nverts; j++) {
      if (vlist[f->verts[j]]->svindex == -1) {
	good = FALSE;
	break;
      }
    }
    if (good) {
      validflist[validfaces++] = f;
    }
  }

  // fprintf(stdout, "valid faces: %d\n", validfaces);

  return(validfaces);
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
    vert->wx = vec2.x;
    vert->wy = vec2.y;
    vert->wz = vec2.z;
    
    // Also compute bbox
    minx = MIN(minx, vert->wx);
    miny = MIN(miny, vert->wy);
    minz = MIN(minz, vert->wz);
    maxx = MAX(maxx, vert->wx);
    maxy = MAX(maxy, vert->wy);
    maxz = MAX(maxz, vert->wz);
  }

  // fprintf(stderr, "BBOX: (%.2f %.2f %.2f) to (%.2f %.2f %2.f)\n",
  // minx, miny, minz, maxx, maxy, maxz);
}


/******************************************************************************
Read in the PLY file from file / standard in.
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

      // Check to see if it has confidences...
      for (int m=0; m < nprops; m++) {
	char *this_prop = ply->elems[i]->props[m]->name;
	if (equal_strings(this_prop, "confidence")) {
	  hasConfidence = TRUE;
	  break;
	}
      }

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      validvlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      // Allocate all the memory in one chunk, instead of piecewise later
      Vertex *varray = (Vertex *) malloc(sizeof(Vertex) * nverts);
      for (int k=0; k < nverts; k++) vlist[k] = &(varray[k]);
	

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      if (hasConfidence) {
	ply_get_property (ply, elem_name, &vert_props[3]);
      }

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) vlist[j]);
	vlist[j]->index = j;
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      validflist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      // Allocate all the memory in one chunk, instead of piecewise later
      Face *farray = (Face *) malloc(sizeof(Face) * nfaces);
      for (k=0; k < nfaces; k++) flist[k] = &(farray[k]);

      /* set up for getting face elements */
      ply_get_property (ply, elem_name, &face_props[0]);

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *) flist[j]);
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
write_file(FILE *out)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  /*** Write out the transformed PLY object ***/


  ply = ply_write (out, nelems, elist, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", validverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  if (hasConfidence) {
    ply_describe_property (ply, "vertex", &vert_props[3]);
  }

  ply_element_count (ply, "face", validfaces);
  ply_describe_property (ply, "face", &face_props[0]);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < validverts; i++) {
    ply_put_element (ply, (void *) validvlist[i]);
    // fprintf(stderr, "%d ", validvlist[i]->svindex);
  }
  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  Face tempface;
  tempface.verts = (int *) malloc(sizeof(int) * 256);
  for (i = 0; i < validfaces; i++) {
    if (validflist[i]->nverts < 3) {
      fprintf(stderr, "Error! less than 3 faces. we're hozed!\n");
      continue;
    }
    // Put the new vertex indices into tempface
    tempface.nverts = validflist[i]->nverts;
    for (j = 0; j < validflist[i]->nverts; j++) {
      tempface.verts[j] = (vlist[validflist[i]->verts[j]])->svindex;
    }
    ply_put_element (ply, (void *) &tempface);
  }


  ply_close (ply);
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "\n");
  fprintf (stderr, "usage: %s [options] subvol_size epsilon [in.ply]\n", progname);
  fprintf (stderr, "   or: %s [options] subvol_size epsilon < in.ply\n", progname);
  fprintf (stderr, "\n");
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "       -writebbox bboxname (writes mesh bbox to file)\n");
  fprintf (stderr, "       -printbbox    (prints bbox to stdout)\n");
  fprintf (stderr, "       -outdir dir   (directory for storing output files..)\n");
  fprintf (stderr, "       -dice subvolsize epsilon basename\n");
  fprintf (stderr, "              Will write out subvols of the form:\n");
  fprintf (stderr, "              basename_-2_3_0.ply\n"); 
  fprintf (stderr, "              (and write the names of the files to stdout.)\n");
  fprintf (stderr, "       -odice subvolsize epsilon basename\n");
  fprintf (stderr, "              Other dice option.  Will not actually generate\n");
  fprintf (stderr, "              any ply files, but will write their names to\n");
  fprintf (stderr, "              stdout (useful to find which subvols have tris.\n");
  fprintf (stderr, "       -crop minx miny minz maxx maxy maxz (crops output)\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "As well as plyxform options:\n");
  fprintf (stderr, "       -f m.xf (a transform matrix file)\n");
  fprintf (stderr, "       -t xtrans ytrans ztrans (translation)\n");
  fprintf (stderr, "       -s xscale yscale zscale (scale)\n");
  fprintf (stderr, "       -r xangle yangle zangle (rotation, all in degrees)\n");
  fprintf (stderr, "       -q qi qj qk ql  (rotation, quaternion)\n");
  fprintf (stderr, "  (point = m.xf * (ftrans_factor + rotz * roty * rotx * scale_factor * point))\n");
  fprintf (stderr, "\n");
  exit (-1);
}
