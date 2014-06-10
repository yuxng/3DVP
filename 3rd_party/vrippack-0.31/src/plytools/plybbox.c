/*

  Compute the axis aligned bounding box that fits around the
  vertices within a ply file.

  Brian Curless
  June 1995

  Steve Marschner, December 2000
  Generalized to multiple input files.

*/


#include <ply.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <float.h>

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

typedef struct Vertex {
    float x, y, z;
    float diff_r, diff_g, diff_b;
} Vertex;

static char *elem_names[] = { 
  "vertex",
};

static PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0},
};

typedef struct BBox {
    float minx, miny, minz, maxx, maxy, maxz;
} BBox;


PlyFile *readPlyFile(FILE *inFile, Vertex **pVerts, int *pNumVerts);
void initbbox(BBox *b);
void updatebbox(BBox *b, Vertex *verts, int numVerts);
void printbbox(BBox *b);
void printUsage();


void
printusage(char *progname)
{
  fprintf (stderr, "usage: %s [in.ply ...]\n", progname);
  fprintf (stderr, "   or: %s < in.ply\n", progname);
  exit (-1);
}


int
main(int argc, char**argv)
{    
   Vertex *verts = NULL;
   int numVerts;
   char *s;
   char *progname;
   FILE *inFile = NULL;
   BBox box;
   
   progname = argv[0];
   
   /* parse -flags */
   while (--argc > 0 && (*++argv)[0]=='-') {
      for (s = argv[0]+1; *s; s++)
	 switch (*s) {
	 default:
	     printusage(progname);
	     break;
	 }
   }    
   
   initbbox(&box);

   /* optional input files (if not, read stdin ) */
   if (argc > 0) {
       while (argc > 0 && *argv[0] != '-') {
	   inFile = fopen(argv[0], "r");
	   if (inFile == NULL) {
	       fprintf(stderr, "Error: Couldn't open input file %s\n", argv[0]);
	       printusage(progname);
	   }
	   argc --;
	   argv ++;

	   readPlyFile(inFile, &verts, &numVerts);

	   if (verts == NULL) {
	       fprintf(stderr, "Obtained no vertices from %s.\n", argv[0]);
	       exit(1);
	   }

	   updatebbox(&box, verts, numVerts);
       } 
   } else {
       readPlyFile(stdin, &verts, &numVerts);

       if (verts == NULL) {
	   fprintf(stderr, "Obtained no vertices from %s.\n", argv[0]);
	   exit(1);
	   }
       
       updatebbox(&box, verts, numVerts);
   }

   if (argc > 0) {
     fprintf(stderr, "Error: Unhandled arg: %s\n", argv[0]);
     printusage(progname);
     exit(-1);
   }
   
   printbbox(&box);
   
   exit(0);
}


void
initbbox(BBox *b)
{
    b->minx = FLT_MAX;
    b->miny = FLT_MAX;
    b->minz = FLT_MAX;

    b->maxx = -FLT_MAX;
    b->maxy = -FLT_MAX;
    b->maxz = -FLT_MAX;
}


void
updatebbox(BBox *b, Vertex *verts, int numVerts)
{
    int i;

    for(i = 0; i < numVerts; i++) {
	b->minx = MIN(b->minx, verts[i].x);
	b->miny = MIN(b->miny, verts[i].y);
	b->minz = MIN(b->minz, verts[i].z);
	
	b->maxx = MAX(b->maxx, verts[i].x);
	b->maxy = MAX(b->maxy, verts[i].y);
	b->maxz = MAX(b->maxz, verts[i].z);	
    }
}


void
printbbox(BBox *b)
{
    printf("\n");
    printf("%f %f %f\n", b->minx, b->miny, b->minz);
    printf("%f %f %f\n", b->maxx, b->maxy, b->maxz);
    printf("\n");
}



PlyFile *
readPlyFile(FILE *inFile, Vertex **pVerts, int *pNumVerts)
{
    int i, j;
    int nelems, numVerts;
    char **elist;
    int file_type;
    float version;
    char *elem_name;
    int nprops, num_vert_props;
    int num_elems;
    PlyProperty **plist;
    Vertex *verts;
    PlyFile *ply;

    vert_props[0].offset = offsetof(Vertex, x);
    vert_props[1].offset = offsetof(Vertex, y);
    vert_props[2].offset = offsetof(Vertex, z);
    num_vert_props = 3;

    ply  = ply_read (inFile, &nelems, &elist);
    ply_get_info (ply, &version, &file_type);

    if (!ply)
	exit(1);

    verts = NULL;

    for (i = 0; i < nelems; i++) {

	/* get the description of the first element */
	elem_name = elist[i];
	plist = ply_get_element_description 
	    (ply, elem_name, &num_elems, &nprops);
	
	/* if we're on vertex elements, read them in */
	if (equal_strings ("vertex", elem_name)) {
	    
	    numVerts = *pNumVerts = num_elems;
	    verts = (Vertex *)malloc(sizeof(Vertex)*numVerts);
	    
	    /* set up for getting vertex elements */
	    ply_get_element_setup (ply, elem_name, num_vert_props, vert_props);
	    
	    /* grab all the vertex elements */
	    for (j = 0; j < numVerts; j++)
		ply_get_element (ply, (void *) &verts[j]);
	}
    }

    if (*pVerts) free(*pVerts);
    *pVerts = verts;

    return ply;
}


void
printUsage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, "plybbox <ply-file>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Plybbox prints the min and max x,y,z values of the vertices\n");
    fprintf(stderr, "  in the Ply file.  The output is relatively unformated:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "     <minx> <miny> <minz>\n");
    fprintf(stderr, "     <maxx> <maxy> <maxz>\n");
    fprintf(stderr, "\n");
}

