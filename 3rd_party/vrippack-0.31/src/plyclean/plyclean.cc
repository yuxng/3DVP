// 
// plyclean:
// Clean up a ply file triangle mesh, getting rid
// of degenerate triangles and slivers.
// 
// Lucas Pereira, October 1998
// A merging of Brian Curless' triedgecol and trisliver
// programs.  Combined them into one program, because
// we often want to do them both, and this cuts the file
// i/o in half.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ply.h>
#include <Linear.h>
#include <string.h>
#include <strings.h>

#include "Mesh.h"
#include "plyio.h"
#include "undo.h"

#ifdef linux
#include <float.h>
#endif

//////////////////////////////////////////////////////////////////////
// Define Variables, Helper Functions
//////////////////////////////////////////////////////////////////////

bool paranoid=FALSE;
bool verbose=FALSE;
bool quiet=FALSE;
bool superQuiet=FALSE;

// default values -- get overwritten
float lengthThresh = 0.00035;
float featureThresh = -1;
float angleThresh = -1;

// Options
// If an argument is relative, then we need to compute mean edge length
bool needMeanLength = FALSE;	
// Meanlength = sum / weight
float meanLengthSum=0.0;
float meanLengthWt=0.0;
float meanLength=0.0;

// If this is true, then don't allow an edge collapse that will cause
// a triangle to change normal drastically
bool neverFlipTris = TRUE;
float flipDotThresh = .5;

// Never move boundary vertices?
bool neverMoveBoundary = TRUE;

// Global stat counters...
int vertsDeleted = 0;
int trisDeleted = 0;
int edgesFlipped = 0;
int trisUnkinked = 0;
int trisUnfinned = 0;
int prevVertsDeleted = 0;
int prevTrisDeleted = 0;
int prevEdgesFlipped = 0;
int prevTrisUnkinked = 0;
int prevTrisUnfinned = 0;

// Usage
void usage(char *progname);
void RunCommands(Mesh *mesh, int argc, char **argv);

// triedgecol Helper functions
void collapse_mesh_edges(Mesh *mesh);
void collapse_vert_edges(Vertex *vert);

// trisliver Helper functions
void remove_mesh_slivers(Mesh *mesh);
// If returns true, then it also returns two 
// vertices to collapse
bool shouldCollapseTri(Triangle *tri, Vertex **vcol1, Vertex **vcol2);

// edgeflip Helper functions
void flip_mesh_edges(Mesh *mesh);
void flip_quad_edges(Vertex *v1, Vertex *v2, Vertex *v3, 
		     Vertex *v4, Triangle *t1, Triangle *t2);

// unkink helper functions
void unkink_mesh_slivers(Mesh *mesh);
Vertex *findSliverTop(Triangle *tri);
bool facesBackward(Triangle *tri);
void moveSliverTop(Vertex *slivertop);

// unfin helper functions
void unfin_mesh_slivers(Mesh *mesh);
Triangle *findTwin(Triangle *tri);
void removeFin(Triangle *tri1, Triangle *tri2);

// both Helper functions
Mesh *readMeshFromPly(FILE *inFile);
void detectBoundaries(Mesh *mesh);
bool updateNormal(Triangle *tri);
int  collapse_edge(Vertex *v1, Vertex *v2);
void count_verts_tris(Mesh *mesh, int *numVerts, int *numTris);
void removeTriangleRefs(Triangle *tri);
bool pruneNeighbors(Vertex *v);
void CheckVertPointers(Vertex *vert, char *comment="");
void replaceVert(Vertex *v1, Vertex *v2, Vertex *v3);
void addTriangle(Vertex *vert, Triangle *tri);
void delTriangle(Vertex *vert, Triangle *tri);
void disconnectVert(Vertex *v1, Vertex *v2);
void addVert(Vertex *v1, Vertex *v2);
void addNeighbors(Vertex *v1, Vertex *v2);
static void reallocTris(Vertex *v);
static void reallocVerts(Vertex *v);


void usage(char *progname)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [options] [command sequence] [-o out.ply] [in.ply]\n",
	  progname);
  fprintf(stderr, "   or: %s [options] [command sequence] < in.ply > out.ply\n",
	  progname);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "       -h prints usage\n");
  fprintf(stderr, "       -v turns on verbose mode (not too useful)\n");
  fprintf(stderr, "       -p turns on paranoid mode (lots of topology \n");
  fprintf(stderr, "          checks, and warns of slightest suspicions..)\n");
  fprintf(stderr, "       -q turns on quiet mode (paranoia is incompatible\n");
  fprintf(stderr, "          with being quiet.)\n");
  fprintf(stderr, "       -Q turns on superQuiet mode (paranoia is incompatible\n");
  fprintf(stderr, "          with being quiet.)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "[command sequence] contains an unlimited number of the following operations:\n");
  fprintf(stderr, "       -defaults\n");
  //  fprintf(stderr, "       -keepbound (default -- don't allow boundary vertices to move)\n");
  //  fprintf(stderr, "       -movebound (allow boundary vertices to move --NOT! (implemented))\n");
  fprintf(stderr, "       -decimate <quality>\n");
  fprintf(stderr, "          (Aggressively cleans, acting as decimator.  Quality should be a\n");
  fprintf(stderr, "           number between 0 (destroy it) and 100 (do nothing).  It has no\n");
  fprintf(stderr, "           guarantees how many polygons it will remove.)\n");
  fprintf(stderr, "       -edgecol <edge-length> <feature-angle>\n");
  fprintf(stderr, "          (collapses edges shorter than length, if surrounding tris are planar\n");
  fprintf(stderr, "           to within feature-angle (180 collapses nothing, 0 everything).)\n");
  fprintf(stderr, "       -edgeflip <feature-angle>\n");
  fprintf(stderr, "          (Flips edges when the two triangles are planar within feature-angle,\n");
  fprintf(stderr, "           and tesselating quad other way reduces max tri angle.)\n");
  fprintf(stderr, "       -sliver  <max-angle>\n");
  fprintf(stderr, "          (Edge-collapses smallest side of slivers with angle greater than max-angle)\n");
  fprintf(stderr, "       -unkink <max-angle> <max-norm-diff>\n");
  fprintf(stderr, "          (Moves 'top' vertex of slivers with angle greater than max-angle, if\n");
  fprintf(stderr, "           it differs from EVERY neighbor by at least max-norm-diff.  CAUTION:\n");
  fprintf(stderr, "           no safety checks. Might not fix problem, _might_ cause others.)\n");
  fprintf(stderr, "       -unfin\n");
  fprintf(stderr, "           removes fins (pairs of triangles that are mirrors of each other.)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, "       %s -defaults a.ply > b.ply\n", progname);
  fprintf(stderr, "       (does a set of ops designed to clean up marching cubes)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       %s -decimate 50 a.ply > b.ply\n", progname);
  fprintf(stderr, "       (does a set of ops designed to decimate mesh till it looks half as good)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       %s -edgecol .6 150 a.ply > b.ply\n", progname);
  fprintf(stderr, "       (collapses edges shorter than .6 units in length\n");
  fprintf(stderr, "        that are more planar than 150 degrees)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "       %s -edgecol 40%% 170 -sliver 160 -edgecol .7 140 < a.ply > b.ply\n", 
	  progname);
  fprintf(stderr, "       (Does a cautious edge-collapse (edges to 40%% of mean length),\n");
  fprintf(stderr, "        remove slivers, and then another edge collapse)\n");
  fprintf(stderr, "\n");
  exit(-1);
}

int
main(int argc, char **argv)
{
  int numVerts = 0, numTris = 0, i;
  char *inName = NULL;
  FILE *inFile = stdin;
  char *outName = NULL;
  FILE *outFile = stdout;
  char *progname = argv[0];

  // Print usage if no args, because this does nothing anyway
  if (argc == 1) {
    usage(progname);
  }

  // Read through the arguments, to make sure they're ok.
  // Want to check bad args before we waste time reading the file...
  // Also, identify the input file name, if given...
  for (i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-h")) {
      usage(progname);
    } 
    else if (!strcmp(argv[i], "-p")) {
      paranoid = TRUE;
      needMeanLength=TRUE;
    } 
    else if (!strcmp(argv[i], "-v")) {
      verbose = TRUE;
    } 
    else if (!strcmp(argv[i], "-q")) {
      quiet = TRUE;
    } 
    else if (!strcmp(argv[i], "-Q")) {
      quiet = TRUE;
      superQuiet = TRUE;
    } 
    else if (!strcmp(argv[i], "-o")) {
      if (argc-i < 2) usage(progname);
      if (argv[i+1][0] == '-') {
	fprintf(stderr, "Error: -o should be followed by a file name...\n");
	usage(progname);
      }
      outName = argv[i+1];
      i++;
    } 
    else if (!strcmp(argv[i], "-edgecol")) {
      if (argc-i < 3) usage(progname);
      if (!quiet) {
	fprintf(stderr, "Will run edge collapse, edge length=%s, "
		"feature angle = %s\n", argv[i+1], argv[i+2]);
      }
      if (argv[i+1][strlen(argv[i+1])-1] == '%') needMeanLength = TRUE;
      i += 2;
    } 
    else if (!strcmp(argv[i], "-sliver")) {
      if (argc-i < 2) usage(progname);
      if (!quiet) {
	fprintf(stderr, "Will run sliver removal, max tri angle=%s\n",
		argv[i+1]);
      }
      i++;
    } 
    else if (!strcmp(argv[i], "-edgeflip")) {
      if (argc-i < 2) usage(progname);
      if (!quiet) {
	fprintf(stderr, "Will run Edge flip, max feature angle = %s\n",
		argv[i+1]);
      }
      i++;
    } 
    else if (!strcmp(argv[i], "-unkink")) {
      if (argc-i < 3) usage(progname);
      if (!quiet) {
	fprintf(stderr, "Will run unkink, max tri angle = %s, "
		"max feature angle = %s\n",
		argv[i+1], argv[i+2]);
      }
      i+=2;
    } 
    else if (!strcmp(argv[i], "-unfin")) {
      if (!quiet) {
	fprintf(stderr, "Will run unfin.\n");
      }
      i += 0;
    } 
    else if (!strcmp(argv[i], "-defaults")) {
      if (!quiet) {
	fprintf(stderr, "Will run defaults (for post-marching-cubes)\n");
      }
      i += 0;
      needMeanLength = TRUE;
    } 
    else if (!strcmp(argv[i], "-decimate")) {
      if (argc-i < 2) usage(progname);
      float quality = atof(argv[i+1]);
      // Check that the quality arg is reasonable.
      if (quality <= 0 || quality >= 100) {
	fprintf(stderr, "Error, decimate quality must be between 0 and 100.\n");
	usage(progname);
	exit(-1);
      }
      if (!quiet) {
	fprintf(stderr, "Will run decimation, quality %.1f%%\n", quality);
      }
      i += 1;
      needMeanLength = TRUE;
    } 
    /*
    else if (!strcmp(argv[i], "-keepbound")) {
      // print nothing
    } 
    else if (!strcmp(argv[i], "-movebound")) {
      // print nothing
    } 
    */
    else if (argv[i][0] != '-' && inName == NULL) {
      inName = argv[i];
      inFile = fopen(inName, "r");
      if (inFile == NULL) {
	fprintf(stderr, "Error: Could not open input ply file %s\n", inName);
	usage(progname);
	exit(-1);
      }
    } 
    else {
      fprintf(stderr, "Error:  Unhandled arg: %s\n", argv[i]);
      usage(progname);
      exit(-1);
    }
  }

  // Now actually read in the input ply file...
  if (!quiet) {
    fprintf(stderr, "Reading input from %s...\n", 
	    (inName == NULL) ? "<stdin>" : inName);
  }

  Mesh *mesh = readMeshFromPly(inFile);

  // Run through the args a second time, actually executing the
  // commands
  RunCommands(mesh, argc, argv);

  // Count the number of vertices/triangles actually used
  count_verts_tris(mesh, &numVerts, &numTris);
  
  if (!quiet) {
    fprintf(stderr, "Writing file %s...\n", 
	    (outName == NULL)? "<stdout>" : outName);
  }
  
  // Write out.  Pass along active numbers for ply header.
  if (outName != NULL) {
    outFile = fopen(outName, "w");
    if (outFile == NULL) {
      fprintf(stderr, "Error: Could not open output ply file %s\n", outName);
      usage(progname);
      exit(-1);
    }	
  }
  writePlyFile(outFile, mesh, numVerts, numTris);
  
  return 0;
}

void RunCommands(Mesh *mesh, int argc, char **argv)
{
  int i;
  for (i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-edgecol")) {
      float lengthThresh_orig = atof(argv[i+1]);
      // If relative ( X% )
      if (argv[i+1][strlen(argv[i+1])-1] == '%') {
	// Make the length relative to the mean...
	lengthThresh_orig *= 0.01 * meanLength;
      }
      float angle = atof(argv[i+2]);
      featureThresh = cos(M_PI/180*(180-angle));
      lengthThresh = lengthThresh_orig;

      // reset stat counters
      prevVertsDeleted = vertsDeleted;
      prevTrisDeleted = trisDeleted;

      // Collapse the Edges
      if (!quiet)
	fprintf(stderr, "Collapsing edges (%f,%s deg)... ", lengthThresh, argv[i+2]);

      collapse_mesh_edges(mesh);
      i += 2;

      if (!quiet)
	fprintf(stderr, "%d verts (%d tris) removed.\n", 
		vertsDeleted-prevVertsDeleted, 
		trisDeleted -prevTrisDeleted);
    } 
    else if (!strcmp(argv[i], "-sliver")) {

      float angle = atof(argv[i+1]);
      angleThresh = cos(M_PI/180.*(180.-angle));
      featureThresh = -1.0;


      // reset stat counters
      prevVertsDeleted = vertsDeleted;
      prevTrisDeleted = trisDeleted;

      // Remove the Slivers
      if (!quiet)
	fprintf(stderr, "Removing slivers (%s deg)... ", argv[i+1]);
      
      remove_mesh_slivers(mesh);
      i++;

      if (!quiet)
	fprintf(stderr, "%d verts (%d tris) removed.\n", 
		vertsDeleted-prevVertsDeleted, 
		trisDeleted -prevTrisDeleted);
    } 
    else if (!strcmp(argv[i], "-edgeflip")) {
      float angle = atof(argv[i+1]);
      featureThresh = cos(M_PI/180.*(180.-angle));

      // reset stat counters
      prevEdgesFlipped = edgesFlipped;

      // Flip the Edges
      if (!quiet)
	fprintf(stderr, "Flipping Edges... ");

      flip_mesh_edges(mesh);
      i++;
      
      if (!quiet)
	fprintf(stderr, "%d edges flipped.\n", edgesFlipped);

    } 
    else if (!strcmp(argv[i], "-unkink")) {
      float angle = atof(argv[i+1]);
      angleThresh = cos(M_PI/180.*(180.-angle));
      angle = atof(argv[i+2]);
      // Note below, angle, not 180-angle
      featureThresh = cos(M_PI/180.*(angle));

      // reset stat counters
      prevTrisUnkinked = trisUnkinked;

      // Unkink...
      if (!quiet)
	fprintf(stderr, "Unkinking sliver tris... ");

      unkink_mesh_slivers(mesh);
      i+=2;
      
      if (!quiet)
	fprintf(stderr, "%d tris unkinked.\n", trisUnkinked);
    }
    else if (!strcmp(argv[i], "-unfin")) {
      // reset stat counters
      prevTrisUnfinned = trisUnfinned;
      
      // Unfin...
      if (!quiet)
	fprintf(stderr, "Unfinning twin tris... ");

      unfin_mesh_slivers(mesh);
      
      if (!quiet)
	fprintf(stderr, "%d tris unfinned.\n", trisUnfinned);

    } 
    else if (!strcmp(argv[i], "-defaults")) {
      char *newargs[] = {"plyclean", 
			 "-edgecol", "10%", "0", 
			 "-sliver", "165",
			 "-edgecol", "35%", "155", 
			 "-edgecol", "50%", "165", 
			 "-sliver", "165", 
			 "-edgecol", "65%", "170", 
			 "-edgecol", "20%", "0", 
			 "-sliver", "170", 
			 "-edgecol", "20%", "0"}; 
      int newargc = sizeof(newargs)/sizeof(char *);
      if (!quiet) 
	fprintf(stderr, "running defaults...\n");
      RunCommands(mesh, newargc, newargs);
    }
    else if (!strcmp(argv[i], "-decimate")) {
      // Get the quality number, between 0 and 100
      float quality = atof(argv[i+1]);
      // Now figure out the max edge length we'll produce:
      // qual=100 -> maxedgelen = 0 
      // qual= 50 -> maxedgelen = 100%
      // qual= 25 -> maxedgelen = 300%
      // qual=  0 -> maxedgelen = infinity
      float maxEdgeLen = 100 * ((100 - quality) / quality);
      // And figure out the max angle feature we'll mess with:
      // qual= 100 -> maxAng = 180
      // qual= 50 -> maxAng = 154
      // qual= 25 -> maxAng = 120
      float maxAng = 180 * (1.20*quality / (20+ quality));

      // Allocate arrays where we'll fill in the args...
      char edgeLens[10][100];
      char edgeAngs[10][100];

      int nEdgeIters = 7;
      // edgelength factor to do each iteration.  Make this grow
      // geometrically (each number is 43% bigger...)
      float edgeLenFactor[] = {.12, .17, .24, .34, .49, .70, 1.00};
      float edgeAngFactor[] = {1.0, .95, .90, .82, .75, .65, .50};
      for (int edgeIter = 0; edgeIter < nEdgeIters; edgeIter++) {
	// edgelen linearly increases to maxedgelen...
	sprintf(&(edgeLens[edgeIter][0]), "%f%%", maxEdgeLen * edgeLenFactor[edgeIter]);
	// edgeAngs falls, so that bigger features get less blurred
	sprintf(&(edgeAngs[edgeIter][0]), "%f", ((1.0 - edgeAngFactor[edgeIter]) * 180 +
						 (edgeAngFactor[edgeIter]) * maxAng));
      }

      // Do a couple sliver removals...
      char sliverAng[100];
      // sliver angle is 2/3 of the way from maxAng to 180...
      sprintf(&(sliverAng[0]), "%f", 120 + maxAng / 3);

      char *newargs[] = {"plyclean", 
			 "-edgecol", &(edgeLens[0][0]), &(edgeAngs[0][0]),
			 "-edgecol", &(edgeLens[1][0]), &(edgeAngs[1][0]),
			 "-sliver", &(sliverAng[0]),
			 "-edgecol", &(edgeLens[2][0]), &(edgeAngs[2][0]),
			 "-edgecol", &(edgeLens[3][0]), &(edgeAngs[3][0]),
			 "-sliver", &(sliverAng[0]),
			 "-edgecol", &(edgeLens[4][0]), &(edgeAngs[4][0]),
			 "-edgecol", &(edgeLens[5][0]), &(edgeAngs[5][0]),
			 "-sliver", &(sliverAng[0])};

      int newargc = sizeof(newargs)/sizeof(char *);
      if (!quiet) 
	fprintf(stderr, "running decimate %.1f%%...\n", quality);
      RunCommands(mesh, newargc, newargs);
    }
    else if (!strcmp(argv[i], "-keepbound")) {
      neverMoveBoundary = TRUE;
    }
    else if (!strcmp(argv[i], "-movebound")) {
      neverMoveBoundary = FALSE;
    }
    else if (!strcmp(argv[i], "-v")) {
      // verbose already turned on, skip....
    } 
    else if (!strcmp(argv[i], "-p")) {
      // paranoid already turned on, skip....
    } 
    else if (!strcmp(argv[i], "-q")) {
      // quiet already turned on, skip....
    } 
    else if (!strcmp(argv[i], "-Q")) {
      // superQuiet already turned on, skip....
    } 
    else if (!strcmp(argv[i], "-o")) {
      // output file already handled
      i++;
    } 
    else if (argv[i][0] != '-') {
      // Skip input filename arg -- already opened...
    } 
    else {
      fprintf(stderr, "Bad Arg uncaught in first pass: %s\n", argv[i]);
      usage("plyclean");
      exit(-1);
    }
  }
}


//////////////////////////////////////////////////////////////////////
// triedgecol helper functions
//////////////////////////////////////////////////////////////////////

void
collapse_mesh_edges(Mesh *mesh)
{
  Vertex *vert;
  int i, j;
  
  /* Cycle over vertices and collapse edges */

  for (i = 0; i < mesh->numVerts; i++) {
    vert = &mesh->verts[i];
    // Invalidate vertex if it has no triangles...
    if (vert->numTris == 0) {
      vert->index = -1;
      if (vert->numVerts > 0) {
	if (paranoid)
	  fprintf(stderr, "Potential problem:  Vertex "
		  "%d has 0 tris, and %d neighbor verts...(fixing)\n", 
		  vert->index, vert->numVerts);
	for (j = 0; j < vert->numVerts; j++) {
	  disconnectVert(vert->verts[j], vert);
	  disconnectVert(vert, vert->verts[j]);
	}
	vert->numVerts = 0;
      }
    } 
    if (vert->index < 0)
      continue;

    if (paranoid) CheckVertPointers(vert, "before");

    collapse_vert_edges(vert);

    if (paranoid) CheckVertPointers(vert, "after");
  }
}


void
collapse_vert_edges(Vertex *vert)
{
  int i;
  Vertex *other;
  Vec3f vedge;
  float length;
    
  // Cycle over nearest neighbors and possibly remove edge
  // if too short
  for (i = 0; i < vert->numVerts; i++) {
    other = vert->verts[i];
    vedge = vert->coord - other->coord;
    length = vedge.length();
    if (length < lengthThresh) {
      if (collapse_edge(vert, other)) {
	// reset i -- basically a new vertex, now...
	i=-1;	
      }
    }	
  }
}

//////////////////////////////////////////////////////////////////////
// trisliver helper functions
//////////////////////////////////////////////////////////////////////

void
remove_mesh_slivers(Mesh *mesh)
{
  Triangle *tri;
  int i;
  Vertex *vcollapse1=NULL, *vcollapse2=NULL;
  
  // Check each triangle to see if it's a sliver
  for (i = 0; i < mesh->numTris; i++) {
    tri = &mesh->tris[i];

    // Check to make sure it's a valid triangle...
    if (tri->vert1 != tri->vert2 &&
	tri->vert1 != tri->vert3 &&
	tri->vert2 != tri->vert3 &&
	tri->vert1->index > -1 &&
	tri->vert2->index > -1 &&
	tri->vert3->index > -1) {

      // If it is a sliver, shouldCollapseTri returns true, and
      // fills the vertex pointers with the two vertices to be
      // collapsed.
      if (shouldCollapseTri(tri, &vcollapse1, &vcollapse2)) {
	
	if (paranoid) {
	  CheckVertPointers(vcollapse1, "v1 slivers before");
	  CheckVertPointers(vcollapse2, "v2 slivers before");
	}
	
	if (collapse_edge(vcollapse1, vcollapse2))
	  i--;
	if (paranoid) {
	  CheckVertPointers(vcollapse1, "v1 slivers after");
	  CheckVertPointers(vcollapse2, "v2 slivers after");
	}

      }
    }
  }
}


bool shouldCollapseTri(Triangle *tri, Vertex **vcol1, Vertex **vcol2)
{
   Vec3f vedge1, vedge2, vedge3;
   float len1, len2, len3, dot1, dot2, dot3;

   vedge1 = tri->vert1->coord - tri->vert2->coord;
   vedge2 = tri->vert2->coord - tri->vert3->coord;
   vedge3 = tri->vert3->coord - tri->vert1->coord;

   len1 = vedge1.length();
   len2 = vedge2.length();
   len3 = vedge3.length();

   // normalize edges...
   vedge1 /= len1;
   vedge2 /= len2;
   vedge3 /= len3;

   dot1 = vedge1.dot(vedge2);
   dot2 = vedge2.dot(vedge3);
   dot3 = vedge3.dot(vedge1);

   // If any of the 3 corners are bigger than the threshold angle,
   // then return the shortest edge.  This, by the law of signs,
   // must be adjacent to the largest angle, not opposite it....
   if (dot1 > angleThresh || dot2 > angleThresh || dot3 > angleThresh) {
     // Find the shortest edge, set the vcol pointers to be
     // the vertex on either end...
     if (len1 < len2 && len1 < len3) {
       *vcol1 = tri->vert1;
       *vcol2 = tri->vert2;
     }
     else if (len2 < len1 && len2 < len3) {
       *vcol1 = tri->vert2;
       *vcol2 = tri->vert3;
     }
     else { // (len3 < len2 && len3 < len1)
       *vcol1 = tri->vert1;
       *vcol2 = tri->vert3;
     }
     return(TRUE);
   } else {
     // All angles are below threshold -- no need to collapse
     return(FALSE);
   }
}

//////////////////////////////////////////////////////////////////////
// edgeflip helper functions
//////////////////////////////////////////////////////////////////////

void flip_mesh_edges(Mesh *mesh)
{
  Vertex *v1, *v2, *v3, *v4;
  int i, j, k;
  Triangle *t;
  Triangle *t1, *t2;
    
  // Cycle over the vertices, and find each edge that is shared by
  // two triangles.  In particular, it assumes that the vertices
  // are ordered in this way:
  // 
  //     v1---v3
  //      \   | \ 
  //       \T1|T2\ 
  //        \ |   \ 
  //         v2---v4
  // 
  // And it will consider whether or not it would be better to
  // re-tesselate it as:
  //
  //     v1---v3
  //      \`. T2\ 
  //       \ `.  \ 
  //        \T1`. \ 
  //         v2--`v4
  // 

  for (i = 0; i < mesh->numVerts; i++) {
    v2 = &mesh->verts[i];
    if (v2->index == -1) continue;

    // Try each neighbor.  Consider only neighbors numbered
    // higher, so we don't consider the edge twice.
    for (j=0; j < v2->numVerts; j++) {
      v3 = v2->verts[j];
      if (v3->index <= v2->index) continue;
      if (v3->index == -1) continue;
      v1 = NULL;
      v4 = NULL;
      int numfound = 0;

      // Now try to find the other two vertices...
      for (k=0; k < v2->numTris; k++) {
	t = v2->tris[k];
	// Look for v1
	if (t->vert1->index == v2->index &&
	    t->vert2->index == v3->index) {
	  v1 = t->vert3;
	  t1 = t;
	  numfound++;
	} 
	else if (t->vert2->index == v2->index &&
		 t->vert3->index == v3->index) {
	  v1 = t->vert1;
	  t1 = t;
	  numfound++;
	} 
	else if (t->vert3->index == v2->index &&
		 t->vert1->index == v3->index) {
	  v1 = t->vert2;
	  t1 = t;
	  numfound++;
	} 
	// Look for v4
	if (t->vert2->index == v2->index &&
	    t->vert1->index == v3->index) {
	  v4 = t->vert3;
	  t2 = t;
	  numfound++;
	} 
	else if (t->vert3->index == v2->index &&
		 t->vert2->index == v3->index) {
	  v4 = t->vert1;
	  t2 = t;
	  numfound++;
	} 
	else if (t->vert1->index == v2->index &&
		 t->vert3->index == v3->index) {
	  v4 = t->vert2;
	  t2 = t;
	  numfound++;
	} 
      }

      if (numfound != 2) {
	// the edge is not shared by exactly two triangles....
	if (paranoid) {
	  fprintf(stderr, "Cannot flip edge %d %d, shared by %d tris.\n",
		  v2->index, v3->index, numfound);
	}
	continue;
      }

      flip_quad_edges(v1, v2, v3, v4, t1, t2);
    }
  }
}

void flip_quad_edges(Vertex *v1, Vertex *v2, Vertex *v3, 
		     Vertex *v4, Triangle *t1, Triangle *t2)
{
  // See diagram in the function above for the meaning of the
  // input args.  Basically, try to see if we should replace
  // the 2-3 edge with a 1-4 edge.  If so, t1 and t2 would
  // take on the new meanings...

  if (paranoid) {
    CheckVertPointers(v1, "v1 before flipedge");
    CheckVertPointers(v2, "v2 before flipedge");
    CheckVertPointers(v3, "v3 before flipedge");
    CheckVertPointers(v4, "v4 before flipedge");
  }

  // First, make sure that the two are basically coplanar
  float normdot = t1->norm.dot(t2->norm);
  if (normdot < featureThresh) {
    // Not planar enough -- throw it away
    return;
  }

  // And make sure that v1 and v4 are not already connected by
  // a triangle, because if so, and we add two more triangles,
  // we'll have an edge with more than two triangles....
  for (int i =0; i < v1->numVerts; i++) {
    if (v1->verts[i] == v4) {
      // Dohp!
      if (verbose) {
	fprintf(stderr, 
		"Cannot flip edge %d %d, because %d and %d "
		"are already connected.\n",
		v2->index, v3->index, v1->index, v4->index);
      }
      return;
    }
  }

  Vec3f vedge12 = v2->coord - v1->coord;
  Vec3f vedge24 = v4->coord - v2->coord;
  Vec3f vedge43 = v3->coord - v4->coord;
  Vec3f vedge31 = v1->coord - v3->coord;
  vedge12.normalize();
  vedge24.normalize();
  vedge43.normalize();
  vedge31.normalize();

  // Compute the cosine of the angle of the quadrilateral at
  // each vertex....
  float cos1 = -(vedge12.dot(vedge31));
  float cos2 = -(vedge24.dot(vedge12));
  float cos4 = -(vedge43.dot(vedge24));
  float cos3 = -(vedge31.dot(vedge43));

  // If the widest angle is at vertex 1 or 4...
  if ((cos1 < cos2 && cos1 < cos3) ||
      (cos4 < cos2 && cos4 < cos3)) {

    // Disconnect v2 and v3
    disconnectVert(v2, v3);
    disconnectVert(v3, v2);
    
    // Connect v1 and v4
    addVert(v1, v4);
    addVert(v4, v1);

    // Reassign the triangles to point to the new vertices
    if      (t1->vert1->index == v3->index) t1->vert1 = v4;
    else if (t1->vert2->index == v3->index) t1->vert2 = v4;
    else if (t1->vert3->index == v3->index) t1->vert3 = v4;
    else fprintf(stderr, "Hey, t1 doesn't touch v3????\n");

    if      (t2->vert1->index == v2->index) t2->vert1 = v1;
    else if (t2->vert2->index == v2->index) t2->vert2 = v1;
    else if (t2->vert3->index == v2->index) t2->vert3 = v1;
    else fprintf(stderr, "Hey, t2 doesn't touch v2????\n");

    // Reassign the vertices to point to their triangles
    addTriangle(v1, t2);
    delTriangle(v2, t2);
    delTriangle(v3, t1);
    addTriangle(v4, t1);

    // Check... should not be needed, but prints debug...
    if (paranoid) {
      bool pruned = FALSE;
      pruned |= pruneNeighbors(v1);
      pruned |= pruneNeighbors(v2);
      pruned |= pruneNeighbors(v3);
      pruned |= pruneNeighbors(v4);
      if (pruned) {
	fprintf(stderr, "Flip edges: doing edge %d--%d, pruned neighbors.\n",
		v2->index, v3->index);
      }
    }

    // Recompute the normals...
    updateNormal(t1);
    updateNormal(t2);

    edgesFlipped++;
  }

  if (paranoid) {
    CheckVertPointers(v1, "v1 after flipedge");
    CheckVertPointers(v2, "v2 after flipedge");
    CheckVertPointers(v3, "v3 after flipedge");
    CheckVertPointers(v4, "v4 after flipedge");
  }

}

//////////////////////////////////////////////////////////////////////
// unkink helper functions
//////////////////////////////////////////////////////////////////////

// Detect sliver triangles with normals that differ significantly
// from every other triangle.  If so, do something about it...
void 
unkink_mesh_slivers(Mesh *mesh)
{
  int i;
  Triangle *tri;

  // Loop through all the triangles
  for (i = 0; i < mesh->numTris; i++) {
    tri = &mesh->tris[i];

    // Check to make sure it's a valid triangle...
    if (tri->vert1 != tri->vert2 &&
	tri->vert1 != tri->vert3 &&
	tri->vert2 != tri->vert3 &&
	tri->vert1->index > -1 &&
	tri->vert2->index > -1 &&
	tri->vert3->index > -1) {

      // findSliverTOP returns NULL if it ain't a sliver
      Vertex *slivertop = findSliverTop(tri);

      if (slivertop != NULL && !(slivertop->onBoundary) &&
	  facesBackward(tri)) {
	// Move slivertop towards its neighbors.  This routine is
	// also expected to update the relevant normals of any 
	// triangles that touch the vertex that mooooooved.
	moveSliverTop(slivertop);
	trisUnkinked++;

	if (paranoid) {
	  CheckVertPointers(tri->vert1, "tri->vert1 after");
	  CheckVertPointers(tri->vert2, "tri->vert2 after");
	  CheckVertPointers(tri->vert3, "tri->vert3 after");
	}

      }
    }
  }
}

// This function detects slivers.  If so, it returns the vertex
// at the top (wide angle) of the sliver.  If not, returns null.
Vertex *
findSliverTop(Triangle *tri)
{
   Vec3f vedge1, vedge2, vedge3;
   float len1, len2, len3, dot1, dot2, dot3;

   vedge1 = tri->vert1->coord - tri->vert2->coord;
   vedge2 = tri->vert2->coord - tri->vert3->coord;
   vedge3 = tri->vert3->coord - tri->vert1->coord;

   len1 = vedge1.length();
   len2 = vedge2.length();
   len3 = vedge3.length();

   // normalize edges...
   vedge1 /= len1;
   vedge2 /= len2;
   vedge3 /= len3;

   // Check each angle, one by one, to see if its over thresh
   if (vedge1.dot(vedge2) > angleThresh) return tri->vert2;
   else if (vedge2.dot(vedge3) > angleThresh) return tri->vert3;
   else if (vedge3.dot(vedge1) > angleThresh) return tri->vert1;
   else return NULL;
}

// This function compares the tri to all of its neighbors.
// If it differs from ALL of its neighbors by a certain amount,
// then it is considered "backward-facing".
bool 
facesBackward(Triangle *tri)
{
  // for all the vertices
  for (int i=0; i < 3; i++) {
    Vertex *v = ((i==0) ? tri->vert1 :
		 (i==1) ? tri->vert2 :
		 tri->vert3);
    // for all the triangles
    for (int j=0; j < v->numTris; j++) {
      Triangle *tri2 = v->tris[j];
      if (tri2 == tri) continue;
      // if normals are too similar, return false (not backward)
      if (tri2->norm.dot(tri->norm) > featureThresh) {
	return false;
      }
    }
  }

  // If we make it to here, it was sufficiently different to be
  // considered backfacing
  return true;
}

// This function actually moves the vertex to be more centered
// around its neighbors, thus (in theory) fixing the sliver.
void 
moveSliverTop(Vertex *slivertop)
{
  // Compute another point, which is the average of all its neighbors
  Vec3f avg(0,0,0);

  int i;
  for (i=0; i < slivertop->numVerts; i++) {
    avg += slivertop->verts[i]->coord;
  }	
  avg /= slivertop->numVerts;
  
  // lerp slivertop and avg
  slivertop->coord = (slivertop->coord + avg);
  slivertop->coord *= 0.5;
  // Recompute normals
  for (i=0; i < slivertop->numTris; i++) {
    updateNormal(slivertop->tris[i]);
  }
}

//////////////////////////////////////////////////////////////////////
// unfin helper functions
//////////////////////////////////////////////////////////////////////

// Detect pairs of triangles that are fins (mirror images of each other),
// and remove them both.
void 
unfin_mesh_slivers(Mesh *mesh)
{
  int i;
  Triangle *tri;

  // Loop through all the triangles
  for (i = 0; i < mesh->numTris; i++) {
    tri = &mesh->tris[i];

    // Check to make sure it's a valid triangle...
    if (tri->vert1 != tri->vert2 &&
	tri->vert1 != tri->vert3 &&
	tri->vert2 != tri->vert3 &&
	tri->vert1->index > -1 &&
	tri->vert2->index > -1 &&
	tri->vert3->index > -1) {

      // findTwin returns NULL if no twin exists
      Triangle *twin = findTwin(tri);

      if (twin != NULL) {
	removeFin(tri, twin);

	if (paranoid) {
	  CheckVertPointers(tri->vert1, "tri->vert1 after");
	  CheckVertPointers(tri->vert2, "tri->vert2 after");
	  CheckVertPointers(tri->vert3, "tri->vert3 after");
	}

      }
    }
  }
}

// This function looks for a "twin" of the current triangle -- 
// e.g. a triangle that has the same vertices, but in the
// opposite order.
Triangle *
findTwin(Triangle *tri)
{
  bool twinfound = false;
  Vertex *v1 = tri->vert1;
  Vertex *v2 = tri->vert2;
  Vertex *v3 = tri->vert3;

  // Loop through all the adjacent triangles of the
  // vertex with the fewest neighbors.
  Vertex *visolated = ((v1->numTris < v2->numTris) ? 
		       ((v1->numTris < v3->numTris) ? v1 : v3) :
		       ((v2->numTris < v3->numTris) ? v2 : v3));
  Triangle *tri2;

  for (int j = 0; j < visolated->numTris; j++) {
    tri2 = visolated->tris[j];
    if (tri2 == tri) continue;

    twinfound = 
      ((v1 == tri2->vert3) && (v2 == tri2->vert2) && (v3 == tri2->vert1)) ||
      ((v1 == tri2->vert2) && (v2 == tri2->vert1) && (v3 == tri2->vert3)) ||
      ((v1 == tri2->vert1) && (v2 == tri2->vert3) && (v3 == tri2->vert2));
    if (twinfound) {
      return tri2;
    }
  }

  // if we get here, no twinfound
  return NULL;
}

// This function removes the two twins.
void
removeFin(Triangle *tri1, Triangle *tri2)
{
  
  // Disconnect the 3 pairs of vertices, if another
  // triangle doesn't exist.
  Vertex *v1 = tri1->vert1;
  Vertex *v2 = tri1->vert2;
  Vertex *v3 = tri1->vert3;
  
  // Local vars for disconnecting verts
  int i;
  Triangle *tri3;
  bool shouldDisconnect;
  Vertex *vv1, *vv2;
  
  // Run through all the triangles of vv1.  If we find a third
  // triangle connecting vv1 and vv2, don't disconnect them.
  // (Iterate 3 times, for all v1/v2....
  for (int j=0; j < 3; j++) {
    // depending on j, set virtual v1 and vv2 to be one of
    // the three combinations of pairs.
    if (j == 0)    { vv1 = v1; vv2 = v2; }
    else if (j==1) { vv1 = v1; vv2 = v3; }
    else           { vv1 = v2; vv2 = v3; }

    shouldDisconnect = true;
    for (i=0; i < vv1->numTris; i++) {
      tri3 = vv1->tris[i];
      if (tri3 != tri1 && tri3 != tri2 &&
	  (tri3->vert1 == vv2 || 
	   tri3->vert2 == vv2 ||
	   tri3->vert3 == vv2)) {
	shouldDisconnect = false;
	break;
      }
    }
    if (shouldDisconnect) {
      disconnectVert(vv1, vv2);
      disconnectVert(vv2, vv1);
    }	
  }

  removeTriangleRefs(tri1);
  removeTriangleRefs(tri2);
  trisUnfinned += 2;
  trisDeleted += 2; 
  
  // At this point, Look at each vertex.  If it has zero
  // triangles, Set the vertex index to -1...
  if (v1->numTris == 0) { v1->index = -1; vertsDeleted++; }
  if (v2->numTris == 0) { v2->index = -1; vertsDeleted++; }
  if (v3->numTris == 0) { v3->index = -1; vertsDeleted++; }
}


//////////////////////////////////////////////////////////////////////
// General functions 
//////////////////////////////////////////////////////////////////////


// count_verts_tris:
// At the very end, before we write out the ply header,
// we need to figure out how many vertices and triangles
// are left.  
void
count_verts_tris(Mesh *mesh, int *numVerts, int *numTris)
{
  Triangle *tri;
  Vertex *vert;
  int count, i;

  // Count vertices, reassign indices so they are contiguous
  count = 0;
  for (i = 0; i < mesh->numVerts; i++) {
    vert = &mesh->verts[i];
    
    if (paranoid) {
      CheckVertPointers(vert, "Counting verts");
    }

    // Toss out verts without tris
    if (vert->numTris == 0 && vert->index != -1) {
      if (paranoid) {
	fprintf(stderr, "Tossing out vertex %d, has 0 triangles...\n",
		vert->index);
      }
      vert->index = -1;
    }
    // Count it if valid
    if (vert->index != -1) {
      vert->index = count;
      count++;
    }
  }
  *numVerts = count;

  // Count triangles
  count = 0;
  for (i = 0; i < mesh->numTris; i++) {
    tri = &mesh->tris[i];
    if ((tri->vert1->index >= 0 && 
	 tri->vert2->index >= 0 && 
	 tri->vert3->index >= 0) &&
	(tri->vert1->index != tri->vert2->index &&
	 tri->vert2->index != tri->vert3->index &&
	 tri->vert3->index != tri->vert1->index)) {
      count++;
    }
  }
  *numTris = count;
  
  if (!superQuiet) {
    fprintf(stderr, "Count: %d vertices, %d triangles.\n", 
	    *numVerts, *numTris);
    int vdel = mesh->numVerts - *numVerts;
    int tdel = mesh->numTris -  *numTris;
    fprintf(stderr, "Deleted: %d vertices (%.1f%%), %d triangles (%.1f%%)\n",
	    vdel, (100.0 * vdel) / mesh->numVerts,
	    tdel, (100.0 * tdel) / mesh->numTris);
  }
}


int
collapse_edge(Vertex *v1, Vertex *v2)
{
    Vec3f norm;
    float minDot;
    int i, j, found, mirrorfound;
    Triangle *tri2;
    Triangle *tri3;

    // If either one is a boundary, then don't collapse the edge
    if (neverMoveBoundary && (v1->onBoundary || v2->onBoundary)) {
      return 0;
    }

    if (paranoid) {
      CheckVertPointers(v1, "v1 entering collapse_edge");
      CheckVertPointers(v2, "v2 entering collapse_edge");
    }

    /* Compute an average normal (1 or 2 triangles will get counted twice) */

    norm.setValue(0,0,0);

    for (i = 0; i < v1->numTris; i++)
	norm += v1->tris[i]->norm;

    for (i = 0; i < v2->numTris; i++)
	norm += v2->tris[i]->norm;

    norm.normalize();

    /* Find the minimum dot product between face normals and average normal */

    minDot = FLT_MAX;
    for (i = 0; i < v1->numTris; i++) {
	minDot = MIN(norm.dot(v1->tris[i]->norm), minDot);
    }

    for (i = 0; i < v2->numTris; i++) {
	minDot = MIN(norm.dot(v2->tris[i]->norm), minDot);
    }


    /* If minimum dot product is too low, then don't collapse edge */
    if (minDot < featureThresh) {
      return 0;
    }

    /* Else collapse edge */
    
    // Initialize undo buffer -- commits every operation so far...
    SaveCheckpoint();

    /* Average coordinates */
    save(v1->coord);
    v1->coord += v2->coord;
    v1->coord /= 2;	

    disconnectVert(v1, v2);
    disconnectVert(v2, v1);

    // Keep track of stats
    save(vertsDeleted);
    vertsDeleted++;

    /* Loop through triangles of v2 */
    for (i = 0; i < v2->numTris; i++) {
	tri2 = v2->tris[i];

	// Check for valid triangle
	int alreadydeleted = (tri2->vert1 == tri2->vert2 ||
			      tri2->vert1 == tri2->vert3 ||
			      tri2->vert2 == tri2->vert3 ||
			      tri2->vert1->index == -1 ||
			      tri2->vert2->index == -1 ||
			      tri2->vert3->index == -1);
	if (alreadydeleted) {
	  if (0 && paranoid) {
	    fprintf(stderr, "tri (%d %d %d) already deleted.\n",
		    tri2->vert1->index, tri2->vert2->index, 
		    tri2->vert3->index);
	  }
	  break;
	}
	
	/* See if triangle is shared by v1 and v2 */
	found = 0;
	for (j = 0; j < v1->numTris; j++) {
	    found = (tri2 == v1->tris[j]);
	    if (found) break;
	}

	// See if the triangle is a mirror reflection of some other tri
	// already attached to v1...  Long logic, because we have to
	// check every possible orientation... :-/
	mirrorfound = 0;
	if (!found) {
	  for (j = 0; j < v1->numTris; j++) {
	    tri3 = v1->tris[j];
		    
	    mirrorfound = 
	      ((tri2->vert1 == tri3->vert3 &&
	        tri2->vert2 == tri3->vert2 &&
		tri2->vert3 == v2 &&
	        v1          == tri3->vert1)) ||
	      ((tri2->vert1 == tri3->vert3 &&
		tri2->vert2 == v2 &&
		v1          == tri3->vert2 &&
		tri2->vert3 == tri3->vert1)) ||
	      ((v1          == tri3->vert3 &&
		tri2->vert1 == v2 &&
		tri2->vert2 == tri3->vert2 &&
		tri2->vert3 == tri3->vert1)) ||

	      ((tri2->vert1 == tri3->vert2 &&
	        tri2->vert2 == tri3->vert1 &&
		tri2->vert3 == v2 &&
	        v1          == tri3->vert3)) ||
	      ((tri2->vert1 == tri3->vert2 &&
		tri2->vert2 == v2 &&
		v1          == tri3->vert1 &&
		tri2->vert3 == tri3->vert3)) ||
	      ((v1          == tri3->vert2 &&
		tri2->vert1 == v2 &&
		tri2->vert2 == tri3->vert1 &&
		tri2->vert3 == tri3->vert3)) ||

	      ((tri2->vert1 == tri3->vert1 &&
	        tri2->vert2 == tri3->vert3 &&
		tri2->vert3 == v2 &&
	        v1          == tri3->vert2)) ||
	      ((tri2->vert1 == tri3->vert1 &&
		tri2->vert2 == v2 &&
		v1          == tri3->vert3 &&
		tri2->vert3 == tri3->vert2)) ||
	      ((v1          == tri3->vert1 &&
		tri2->vert1 == v2 &&
		tri2->vert2 == tri3->vert3 &&
		tri2->vert3 == tri3->vert2));
	    if (mirrorfound) {
	      break;
	    }
	  }
	}

	/* If shared, remove all references to this triangle */
	if (found) {
	  // Remove links to/from disappearing vertex...
	  if (tri2->vert1 != v1 && tri2->vert1 != v2) {
	    disconnectVert(tri2->vert1, v2);
	    disconnectVert(v2, tri2->vert1);
	  }
	  else if (tri2->vert2 != v1 && tri2->vert2 != v2) {
	    disconnectVert(tri2->vert2, v2);
	    disconnectVert(v2, tri2->vert2);
	  }
	  else if (tri2->vert3 != v1 && tri2->vert3 != v2) {
	    disconnectVert(tri2->vert3, v2);
	    disconnectVert(v2, tri2->vert3);
	  } 
	  else {
	    fprintf(stderr, "Umm, deleting v2, not in tri...\n");
	  }

	  removeTriangleRefs(tri2);
	  i--;

	  // Keep track of stats
	  save(trisDeleted);
	  trisDeleted++;
	}
	
	// If mirrored, nuke em both....
	else if (mirrorfound) {
	  // Disconnect the common edge joining the two vertices
	  // (other than v1 and v2).  They are no longer directly
	  // connected by an edge...  Only need to check tri2,
	  // since tri3 shares the same edge.
	  if (tri2->vert1 == v1 || tri2->vert1 == v2) {
	    disconnectVert(tri2->vert2, tri2->vert3);
	    disconnectVert(tri2->vert3, tri2->vert2);
	  } 
	  else if (tri2->vert2 == v1 || tri2->vert2 == v2) {
	    disconnectVert(tri2->vert1, tri2->vert3);
	    disconnectVert(tri2->vert3, tri2->vert1);
	  } 
	  else if (tri2->vert3 == v1 || tri2->vert3 == v2) {
	    disconnectVert(tri2->vert1, tri2->vert2);
	    disconnectVert(tri2->vert2, tri2->vert1);
	  } else {
	    fprintf(stderr, "Mirrorfound tri2: no common edge?!?!?\n");
	  }

	  removeTriangleRefs(tri2);
	  removeTriangleRefs(tri3);
	  i--;

	  // Keep track of stats
	  save(trisDeleted);
	  trisDeleted += 2;

	}

	/* Else, substitute v1 for v2 and add triangle to v1's list*/
	else {
	    
	  /* If a vertex of the triangle is v1,
	     then simply replace it with v2.
	     Otherwise, replace v2 with v1 in the neighboring
	     vertex lists and add the neighboring vertex to v1's list */

	  if (tri2->vert1 == v2) {
	    save(tri2->vert1);
	    tri2->vert1 = v1;
	  } else {
	    replaceVert(tri2->vert1, v2, v1);
	    addVert(v1, tri2->vert1);
	    disconnectVert(v2, tri2->vert1);
	  }

	  if (tri2->vert2 == v2) {
	    save(tri2->vert2);
	    tri2->vert2 = v1;
	  } else {
	    replaceVert(tri2->vert2, v2, v1);
	    addVert(v1, tri2->vert2);
	    disconnectVert(v2, tri2->vert2);
	  }

	  if (tri2->vert3 == v2) {
	    save(tri2->vert3);
	    tri2->vert3 = v1;
	  } else {
	    replaceVert(tri2->vert3, v2, v1);
	    addVert(v1, tri2->vert3);
	    disconnectVert(v2, tri2->vert3);
	  }

	  addTriangle(v1, tri2);
	  delTriangle(v2, tri2);
	  i--;
	}
    }

    // Ok, now check and make sure all the edge connections are
    // supported by a triangle.  (For example, mirror polygons
    // that nuke each other might leave dangling vertex neighbor
    // pointers, but it's hard to figure out until now, when we've
    // nuked all the triangles..
    pruneNeighbors(v1);
    // Remove all neighbor pointers for v2, too... :-)
    pruneNeighbors(v2);

    /* Update normals of triangles around v1 */
    for (i = 0; i < v1->numTris; i++) {
      if (updateNormal(v1->tris[i])) {
	// A normal changed too much
	undo();
	return(0);	
      }
    }

    // Make sure every edge is mentioned twice...
    // Otherwise, we made a topology change
    if (!(v1->onBoundary)) {
      int bedges = 0;
      for (j=0; j < v1->numVerts; j++) {
	Vertex *n = v1->verts[j];
	// Count how many times a triangle refers to this vertex
	int nrefs = 0;
	for (int k=0; k < v1->numTris; k++) {
	  if (v1->tris[k]->vert1 == n ||
	      v1->tris[k]->vert2 == n ||
	      v1->tris[k]->vert3 == n) {
	    nrefs++;
	  }
	}
	if (nrefs != 2) {
	  if (verbose) {
	    fprintf(stderr, 
		    "Undoing collapse of %d and %d, because it would change the\n"
		    "topology such that new edge %d -- %d would have %d triangles.\n",
		    v1->index, v2->index,
		    v1->index, n->index, nrefs);
	  }
	  undo();
	  return(0);
	}
      }
    }

    if (paranoid) {
      // Check to make sure that edge and triangle numbers agree...
      CheckVertPointers(v1, "v1 exiting collapse_edge");
      CheckVertPointers(v2, "v2 exiting collapse_edge");
    }

    /* v2 no longer exists */
    save(v2->index);
    v2->index = -1;

    // We're committed to this change -- turn saving off
    SaveOff();

    return 1;
}

void 
CheckVertPointers(Vertex *vert, char *comment)
{
  // Check to make sure that every triangle mentioned by this
  // vertex includes this vertex just once.
  int i;
  for (i=0; i < vert->numTris; i++) {
    int count = 
      ((vert->tris[i]->vert1 == vert)?1:0) +
      ((vert->tris[i]->vert2 == vert)?1:0) +
      ((vert->tris[i]->vert3 == vert)?1:0);
    if (count != 1) {
      fprintf(stderr, "%s vert %d: tri (%d %d %d) refs %d times...\n", 
	      comment, vert->index, vert->tris[i]->vert1->index,
	      vert->tris[i]->vert2->index, vert->tris[i]->vert3->index,
	      count);
    }
  }

  // Check to make sure that every vertex mentioned as a neighbor
  // points back to this guy.
  for (i=0; i < vert->numVerts; i++) {
    Vertex *v2  = vert->verts[i];
    int count=0;
    for (int j=0; j < v2->numVerts; j++) {
      if (v2->verts[j]->index == vert->index) {
	count++;
      }
    }
    if (count != 1) {
      fprintf(stderr, "%s vert %d: neighbor %d points back %d times.\n", 
	      comment, vert->index, v2->index, count);
    }
  }

  // Check to make sure that either:
  //  - it is not a boundary vertex, and every edge has two tris, or
  //  - it is a boundary vertex, and edges have 1 or two tris.
  for (i=0; i < vert->numVerts; i++) {
    Vertex *v2  = vert->verts[i];
    int count=0;
    for (int j=0; j < vert->numTris; j++) {
      Triangle *t = vert->tris[j];
      if (t->vert1 == v2 || t->vert2 == v2 || t->vert3 == v2) {
	count++;
      }
    }
    if ((!(vert->onBoundary)) && (count != 2)) {
      fprintf(stderr, "%s internal vert %d: neighbor %d shares %d tris.\n", 
	      comment, vert->index, v2->index, count);
    } else if (vert->onBoundary && (count == 0) || (count > 2)) {
      fprintf(stderr, "%s boundary vert %d: neighbor %d shares %d tris.\n", 
	      comment, vert->index, v2->index, count);
    }
  }  
}


// This routine removes any neighbors not actually connected by a
// triangle.  Returns TRUE if it made any prunings.
bool 
pruneNeighbors(Vertex *v)
{
  int i, j;
  Vertex *v2;
  Triangle *t2;
  bool pruned = FALSE;

  for (i=0; i < v->numVerts; i++) {	
    v2 = v->verts[i];
    bool found = FALSE;
    for (j=0; j < v->numTris; j++) {
      t2 = v->tris[j];
      if (t2->vert1 == v2 ||
	  t2->vert2 == v2 || 
	  t2->vert3 == v2) {
	found = TRUE;
	break;
      }
    }
    if (!found) {
      disconnectVert(v, v2);
      disconnectVert(v2, v);
      pruned = TRUE;
    }
  }
  return(pruned);
}


void
disconnectVert(Vertex *v1, Vertex *v2)
{
    int i;

    for (i = 0; i < v1->numVerts; i++) {
	if (v1->verts[i] == v2) {
	    save(v1->verts[i]);
	    v1->verts[i] = v1->verts[v1->numVerts-1];
	    save(v1->numVerts);
	    v1->numVerts--;
	    break;
	}
    }
}


void
addVert(Vertex *v1, Vertex *v2)
{
    int i;

    for (i = 0; i < v1->numVerts; i++) {
	if (v1->verts[i] == v2)
	    return;
    }

    if (v1->numVerts == v1->maxVerts)
	reallocVerts(v1);

    save(v1->verts[v1->numVerts]);
    v1->verts[v1->numVerts] = v2;
    save(v1->numVerts);
    v1->numVerts++;
}

void
removeTriangleRefs(Triangle *tri)
{
  int i;
  Vertex *vert;

  /* Find each reference to the triangle in the vertex lists
       and replace it with the last triangle in the list
       and decrement the list length */

  vert = tri->vert1;
  for (i = 0; i < vert->numTris; i++) {
    if (vert->tris[i] == tri) {
      save(vert->tris[i]);
      vert->tris[i] = vert->tris[vert->numTris-1];
      save (vert->numTris);
      vert->numTris--;
      break;
    }
  }

  vert = tri->vert2;
  for (i = 0; i < vert->numTris; i++) {
    if (vert->tris[i] == tri) {
      save(vert->tris[i]);
      vert->tris[i] = vert->tris[vert->numTris-1];
      save(vert->numTris);
      vert->numTris--;
      break;
    }
  }

  vert = tri->vert3;
  for (i = 0; i < vert->numTris; i++) {
    if (vert->tris[i] == tri) {
      save(vert->tris[i]);
      vert->tris[i] = vert->tris[vert->numTris-1];
      save(vert->numTris);
      vert->numTris--;
      break;
    }
  }
}


void
replaceVert(Vertex *v1, Vertex *v2, Vertex *v3)
{
  int i, pos2, found3;

  found3 = 0;
  pos2 = -1;
  for (i = 0; i < v1->numVerts; i++) {
    if (v1->verts[i] == v2)
      pos2 = i;
    if (v1->verts[i] == v3)
      found3 = 1;
  }

  if (pos2 < 0)
    return;
  else if (!found3) {
    save(v1->verts[pos2]);
    v1->verts[pos2] = v3;
  } else {
    save(v1->verts[pos2]);
    v1->verts[pos2] = v1->verts[v1->numVerts-1];
    save(v1->numVerts);
    v1->numVerts--;
  }
}


void
addTriangle(Vertex *vert, Triangle *tri)
{
    if (vert->numTris == vert->maxTris)
	reallocTris(vert);

    save(vert->tris[vert->numTris]);
    vert->tris[vert->numTris] = tri;
    save(vert->numTris);
    vert->numTris++;
}

// This does the opposite of addTriangle -- removes
// a pointer from a vertex to a triangle....
void
delTriangle(Vertex *vert, Triangle *tri)
{
  int i;
  for (i=0; i < vert->numTris; i++) {
    if (vert->tris[i] == tri) {
      save(vert->tris[i]);
      vert->tris[i] = vert->tris[vert->numTris-1];
      save(vert->numTris);
      vert->numTris--;
      break;
    }
  }
}



bool
updateNormal(Triangle *tri)
{
    Vec3f v1, v2, v3;
    Vec3f oldnorm = tri->norm;

    v1.setValue(tri->vert1->coord);
    v2.setValue(tri->vert2->coord);
    v3.setValue(tri->vert3->coord);

    v2.setValue(v1 - v2);
    v3 = v1 - v3;
    save(tri->norm);
    tri->norm = v3.cross(v2);
    tri->norm.normalize();

    if ((oldnorm.x || oldnorm.y || oldnorm.z) &&
	tri->norm.dot(oldnorm) < flipDotThresh) {
      // Aborting collapse -- mindot too small
      return(TRUE);
    } else {
      return(FALSE);
    }
    
}


Mesh *
readMeshFromPly(FILE *inFile)
{
    int i, j, k;
    Mesh *mesh;

    mesh = readPlyFile(inFile);
    if (mesh == NULL)
	return NULL;
    
    // Initialize Vertex variables
    for (i = 0; i < mesh->numVerts; i++) {
      // Stuff for edgecol
      mesh->verts[i].numVerts = 0;
      mesh->verts[i].maxVerts = 8;
      mesh->verts[i].verts = new Vertex*[mesh->verts[i].maxVerts];
      
      mesh->verts[i].numTris = 0;    
      mesh->verts[i].maxTris = 8;
      mesh->verts[i].tris = new Triangle*[mesh->verts[i].maxTris];
      
      // Clear onBoundary flag -- we'll detect it below...
      mesh->verts[i].onBoundary = FALSE;
    }

    // Initialize Triangle variables
    for (i = 0; i < mesh->numTris; i++) {
	Triangle *tri = &mesh->tris[i];

	updateNormal(tri);

	Vertex *vert1 = tri->vert1;
	Vertex *vert2 = tri->vert2;
	Vertex *vert3 = tri->vert3;	
	
	// stuff for edgecol
	if (vert1->numTris == vert1->maxTris) {
	    reallocTris(vert1);
	}
	if (vert2->numTris == vert2->maxTris) {
	    reallocTris(vert2);
	}
	if (vert3->numTris == vert3->maxTris) {
	    reallocTris(vert3);
	}

	vert1->tris[vert1->numTris++] = tri;
	vert2->tris[vert2->numTris++] = tri;
	vert3->tris[vert3->numTris++] = tri;

	// Make each vertex point to each other as neighbors.
	addNeighbors(vert1,vert2);
	addNeighbors(vert1,vert3);
	addNeighbors(vert2,vert3);
    }

    // Print stats
    if (!superQuiet) {
      fprintf(stderr, "Loaded %d vertices (%d triangles).\n",
	      mesh->numVerts, mesh->numTris);
    }

    // Compute mean edge length, if necessary....
    if (needMeanLength) {
      if (!quiet)
	fprintf(stderr, "Computing mean length...\n");
      for (i = 0; i < mesh->numVerts; i++) {
	Vertex *v = &(mesh->verts[i]);
	for (j=0; j < v->numVerts; j++) {
	  Vertex *n = v->verts[j];
	  if (n->index > v->index) {
	    Vec3f vedge = n->coord - v->coord;
	    meanLengthSum += vedge.length();
	    meanLengthWt += 1.0;
	  }
	}
      }
      meanLength = meanLengthSum / meanLengthWt;
    }

    // Set the onBoundary flag, and do a few sanity checks
    // on the connectivity for each vertex.
    detectBoundaries(mesh);

    // If paranoid, check vertex pointers right away
    if (paranoid) {
      for (i=0; i < mesh->numVerts; i++) {
	Vertex *v = &(mesh->verts[i]);
	CheckVertPointers(v, "Reading file");
      }
    }

    return mesh;
}


void detectBoundaries(Mesh *mesh)
{
  int i, j, k;

  if (!quiet) 
    fprintf(stderr, "Detecting boundary vertices...\n");

  // Always compute the boundary.  It's a goodThing(tm)
  // if (!neverMoveBoundary) return;
  
  // Set the onBoundary flag for each vertex
  // If numVerts == numTris, it is not on Boundary.
  // Optionally, do heavier-duty check, to make sure every
  // edge is mentioned once or twice...
    
  // Do this the quick way -- check the count for the number
  // of edges, and the number of triangles -- if the number
  // of edges is 1 greater than the number of triangles, then
  // it is an edge vertex.  If it's not 0 or 1 difference,
  // then something bad is happening....
    
  for (i = 0; i < mesh->numVerts; i++) {
    Vertex *v = &(mesh->verts[i]);
      
    if (v->numTris == v->numVerts) {
      v->onBoundary = FALSE;
    } else if (v->numTris + 1 == v->numVerts) {
      v->onBoundary = TRUE;
    } else {
      if (paranoid) {
	fprintf(stderr, "2D-manifold warn: Vert %d: %d edges, "
		"%d tris. Marking as boundary...\n", 
		v->index, v->numVerts, v->numTris);
      }
      v->onBoundary = TRUE;
    }

    // Optionally, do heavier-duty check, to make sure every
    // edge is mentioned once or twice...
    if (paranoid) {
      int bedges = 0;
      for (j=0; j < v->numVerts; j++) {
	Vertex *n = v->verts[j];
	// Count how many times a triangle refers to this vertex
	int nrefs = 0;
	for (k=0; k < v->numTris; k++) {
	  if (v->tris[k]->vert1 == n ||
	      v->tris[k]->vert2 == n ||
	      v->tris[k]->vert3 == n) {
	    nrefs++;
	  }
	}
	if (nrefs ==1) {
	  // Count the boundary edges
	  bedges++;	
	} else if (nrefs == 2) {
	  // do nothing -- just fine for internal edges
	} else {
	  fprintf(stderr, "Error:  Vertex %d, neighbor %d, is mentioned "
		  "by %d triangles....\n", v->index, n->index, nrefs);
	}	
      }
	
      // Check bedges is sane -- at very least, even-numbered
      // (Since we already printed the 2D-manifold warn above)
      if (bedges%2) {
	fprintf(stderr, "Error: Vertex %d has %d boundary edges. (ODD?)\n",
		v->index, bedges);
      }
    }
  }
}


static void
reallocTris(Vertex *v)
{
    int i;
    Triangle **newTris;

    save(v->maxTris);
    v->maxTris *= 2;
    newTris = new Triangle*[v->maxTris];
    for (i = 0; i < v->numTris; i++) {
	newTris[i] = v->tris[i];
    }
    
    // BUGBUG: Memory leak for now...
    // delete [] v->tris;

    save(v->tris);
    v->tris = newTris;
}


// This function doesn't save(), since it
// is only called when loading in the mesh.
// Thus it can't be undone.
void
addNeighbors(Vertex *v1, Vertex *v2)
{
  int found = 0;
  
  for (int i = 0; i < v1->numVerts; i++) {
    if (v1->verts[i] == v2) {
      found = 1;
      break;
    }
  }
  
  if (!found) {
    if (v1->numVerts == v1->maxVerts) {
      reallocVerts(v1);
    }
    if (v2->numVerts == v2->maxVerts) {
      reallocVerts(v2);
    }
    v1->verts[v1->numVerts++] = v2;
    v2->verts[v2->numVerts++] = v1;
  }
}


static void
reallocVerts(Vertex *v)
{
    int i;
    Vertex **newVerts;

    save(v->maxVerts);
    v->maxVerts *= 2;
    newVerts = new Vertex*[v->maxVerts];
    for (i = 0; i < v->numVerts; i++) {
	newVerts[i] = v->verts[i];
    }
 
    //  BUGBUG: Memory leak for now
    // delete [] v->verts;

    save(v->verts);
    v->verts = newVerts;
}


