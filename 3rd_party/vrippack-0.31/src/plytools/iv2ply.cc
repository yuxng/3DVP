//
// iv2ply.cc
// Written by Sean Anderson.
// February 1999.
//
// Brian Curless' original code at the bottom, ifdef out.
// Brian's code was not general enough; it didn't work on nearly all
// Inventor files, but this code should.
//

#include <assert.h>
#include <stdio.h>

#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/actions/SoCallbackAction.h>

#include <vector.h> // STL


struct TriangleFace
{
   TriangleFace()
   {
      data[0] = 3; // 3 indices follow, since we have a triangle
   }
   
   void set(const int v, const int coordindex)
   {
      memcpy(data + 1 + v * sizeof(int), &coordindex, sizeof(int));
   }
   
   int get(const int v) const
   {
      int coordindex;
      memcpy(&coordindex, data + 1 + v * sizeof(int), sizeof(int));
      return coordindex;
   }
   
   // uchar int int int
   unsigned char data[sizeof(unsigned char) + 3 * sizeof(int)];
};


// When we do a traversal of the scene graph, we put the newly
// found vertices and triangles into an instance of CallbackInfo,
// so that successive calls of the callback function can return
// what they find and see what has been found so far.
class CallbackInfo
{
public:
   CallbackInfo()
   {
      currentCoord3 = NULL; 
      binary = true;
   
      cba.addTriangleCallback(SoNode::getClassTypeId(), 
			      &CallbackInfo::triangleCallback, 
			      this);
   }
   
   ~CallbackInfo()
   {
   }
   
   
   bool apply(SoNode * node)
   {
      if (node == NULL)
      {
	 return true;
      }
      
      cba.apply(node);
      return false;
   }
   
   bool write(FILE * out)
   {
      fprintf(out, 
	      "ply\n"
	      "format %s 1.0\n"
	      "element vertex %d\n"
	      "property float x\n"
	      "property float y\n"
	      "property float z\n"
	      "element face %d\n"
	      "property list uchar int vertex_indices\n"
	      "end_header\n",
	      binary ? "binary_big_endian" : "ascii",
	      coords.size(),
	      indices.size());
      
      if (binary)
      {
	 int numWritten;
	 numWritten = fwrite(coords.begin(), 
			     sizeof(*coords.begin()), coords.size(), 
			     out);
	 if (numWritten != coords.size())
	 {
	    fprintf(stderr, "error writing ply file\n");
	    return true;
	 }
	 
	 numWritten = fwrite(indices.begin(), 
			     sizeof(*indices.begin()), indices.size(), 
			     out);
	 if (numWritten != indices.size())
	 {
	    fprintf(stderr, "error writing ply file\n");
	    return true;
	 }
      }
      else // ascii 
      {
	 for (vector<SbVec3f>::iterator i = coords.begin();
	      i != coords.end();
	      i++)
	 {
	    if (0 > fprintf(out, "%g %g %g\n", (*i)[0], (*i)[1], (*i)[2]))
	    {
	       fprintf(stderr, "error writing ply file\n");
	       return true;
	    }
	 }
	 
	 for (vector<TriangleFace>::iterator j = indices.begin();
	      j != indices.end();
	      j++)
	 {
	    if (0 > fprintf(out, "3 %d %d %d\n", 
			    (*j).get(0), (*j).get(1), (*j).get(2)))
	    {
	       fprintf(stderr, "error writing ply file\n");
	       return true;
	    }
	 }
	 
      }
      
      return false;
   }
   
   static void triangleCallback(void * userData,
				SoCallbackAction * action,
				const SoPrimitiveVertex * v1,
				const SoPrimitiveVertex * v2,
				const SoPrimitiveVertex * v3)

   {
      assert(userData);
      ((CallbackInfo *) userData)->triangleCallback(action, v1, v2, v3);
   }

   void triangleCallback(SoCallbackAction * action,
			 const SoPrimitiveVertex * v1,
			 const SoPrimitiveVertex * v2,
			 const SoPrimitiveVertex * v3);
   
   SoCallbackAction cba; 
   
   bool binary;
   
   vector<SbVec3f> coords;
   vector<TriangleFace> indices;
   const SbVec3f * currentCoord3;
      
};

void
CallbackInfo::triangleCallback(SoCallbackAction * action,
			       const SoPrimitiveVertex * v1,
			       const SoPrimitiveVertex * v2,
			       const SoPrimitiveVertex * v3)
{
   const SbVec3f * newCurrentCoord3 = (action->getNumCoordinates() > 0) ? 
      &action->getCoordinate3(0) : NULL;   
   
   if (newCurrentCoord3 != currentCoord3)
   {
      currentCoord3 = newCurrentCoord3;
      if (currentCoord3)
      {
	 SbMatrix m = action->getModelMatrix();
	 SbVec3f p;
	    
	 const int n = action->getNumCoordinates();
	 for (int i = 0; i < n; i++)
	 {	    
	    m.multVecMatrix(currentCoord3[i], p);
	    coords.push_back(p);
	 }	    
      }
   }
   

   const SoFaceDetail * facedetail = 
      dynamic_cast<const SoFaceDetail *>(v1->getDetail());
   
   TriangleFace f;
   
   if (currentCoord3 && facedetail)
   {
      for (int i = 0; i < 3; i++) 
      {
	 const SoPointDetail * pointdetail = facedetail->getPoint(i);
	 assert(pointdetail);
	 
	 int index = pointdetail->getCoordinateIndex();
      
	 int k;
	 if (currentCoord3[index] == v1->getPoint())
	 {
	    k = 0;
	 }
	 else if (currentCoord3[index] == v2->getPoint())
	 {
	    k = 1;
	 }
	 else 
	 {
	    assert(currentCoord3[index] == v3->getPoint());
	    k = 2;
	 }
	 f.set(k, index + coords.size() - action->getNumCoordinates());
      }
   }
   else
   {
      SbMatrix m = action->getModelMatrix();
      SbVec3f p;
      
      f.set(0, coords.size());            
      m.multVecMatrix(v1->getPoint(), p);
      coords.push_back(p);
      
      f.set(1, coords.size());           
      m.multVecMatrix(v2->getPoint(), p);
      coords.push_back(p);

      f.set(2, coords.size());           
      m.multVecMatrix(v3->getPoint(), p);
      coords.push_back(p);
   }  
   
   indices.push_back(f);
}


// Returns false on success.
bool
iv2ply(SoNode * node, FILE * plyoutput, bool binary)
{
   CallbackInfo cbi;

   cbi.binary = binary;
   
   return cbi.apply(node) || cbi.write(plyoutput);
}


#if 0
// We could paste the following into a header called iv2ply.h and
// change the above 0 to a 1 to use iv2ply as a function in a larger program.


// -- start of iv2ply.h
#ifndef iv2ply_h
#define iv2ply_h

#include <stdio.h>
#include <Inventor/nodes/SoGroup.h>

bool iv2ply(SoNode * node, FILE * plyoutput, bool binary = true);

#endif
// -- end of iv2ply.h


#else // use as stand alone program.

void
printUsage(const char * name)
{
   fprintf(stderr, 
	   "Usage:\n%s -h | [-a] [(- | infile.iv) [outfile.ply]]\n"
	   "Where:\n"
	   "  -a           write ascii ply file instead of binary\n"
	   "  -            read Inventor scene graph from standard input\n"
	   "  infile.iv    is the name of the Inventor input file\n"
	   "  outfile.ply  is the name of the plyfile output\n"
	   "               (use standard out otherwise).\n", name);
}

int
main(int argc, char * argv[])
{
   bool binary = true;
   char * inputname = NULL;
   char * outputname = NULL;
   
   for (int c = 1; c < argc; c++)
   {
      if (!strcmp(argv[c], "-h"))
      {
	 printUsage(argv[0]);
	 return 1;
      }
      else if (!strcmp(argv[c], "-a"))
      {
	 binary = false;
      }
      else if (!inputname)
      {
	 inputname = argv[c];
      }
      else if (!outputname)
      {
	 outputname = argv[c];
      }
   }
   
   SoDB::init();
   
   SoInput in;
   if (inputname && strcmp(inputname, "-"))
   {
      if (!in.openFile(inputname))
      {
	 printUsage(argv[0]);
	 return 1;
      }
   }
   SoSeparator * root = SoDB::readAll(&in);
   if (root == NULL)
   {
      fprintf(stderr, "Error reading inventor file\n");
      printUsage(argv[0]);
      return 1;
   }
   
   root->ref();
   
   FILE * out = stdout;
   if (outputname)
   {
      out = fopen(outputname, "w");
      if (!out)
      {
	 fprintf(stderr, "Error opening ply file '%s' for writing.\n", 
		 outputname);
	 printUsage(argv[0]);
	 return 1;	 
      }
   }
   
   bool r = iv2ply(root, out, binary);
   
   root->unref();
   
   return r;
}

#endif





#if 0

//
// Brian L. Curless
// Stanford University
// August 1997
//


//  This program is a quick fix for doing some Inventor to ply conversions.
//  Known problems include: the shapes must be under a single group node,
//  and normals are not handled carefully.
//  This program should be generalized to use scene graph traversal.

#include <stdio.h>
#include <Inventor/So.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream.h>

#define MAX_VERT_PROPS 20

typedef unsigned char uchar;
typedef float Normal[3];

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    uchar diff_r, diff_g, diff_b;
    float intensity;
    float std_dev;
};

struct Face {
    uchar nverts;
    int *verts;
};

struct FaceNorms {
    uchar nnorms;
    int *norms;
};



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
static PlyProperty vert_prop_intens =  
  {"intensity", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_std_dev =  
  {"std_dev", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_r =  
  {"diffuse_red", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_g =  
  {"diffuse_green", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_b =  
  {"diffuse_blue", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_props[MAX_VERT_PROPS];


static PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};


/* dummy variables and associated macros for computing field offsets */

Vertex *vert_dummy;
#define voffset(field) ((char *) (&vert_dummy->field) - (char *) vert_dummy)
Face *face_dummy;
#define foffset(field) ((char *) (&face_dummy->field) - (char *) face_dummy)


static int DIFFUSE_COLORS;
static int INTENSITIES;
static int FOUND_NORMALS;

// Function definitions
int readInventorFile(FILE *inFile, Vertex **pVerts, Face **pFaces, 
		     int *pNumVerts, int *pNumFaces);
int writePlyFile(FILE *outFile, Vertex *verts, Face *faces, 
		 int numVerts, int numFaces);
void printUsage();


void
main(int argc, char**argv)
{    
    Vertex *verts = NULL;
    Face *faces = NULL;
    int numVerts, numFaces, i;

    char *ivName = NULL;
    FILE *ivIn = NULL;


    // Process input parameters
    if (argc == 1) {
      // No args, use stdin
      ivIn = stdin;
    } else if (argc == 2 && argv[1][0] != '-') {
      // 1 arg, doesn't begin with "-", use as input iv file
      ivName = argv[1];
      ivIn = fopen(ivName, "r");
      if (ivIn == NULL) {
	fprintf(stderr, "Error: Cannot open input Inventor file %s\n", ivName);
	printUsage();
	exit(-1);
      }
    } else {
      printUsage();
      exit(-1);
    }
		       
    readInventorFile(ivIn, &verts, &faces, &numVerts, &numFaces);

    if (verts == NULL) {
	fprintf(stderr, "Obtained no vertices.\n");
	exit(1);
    }

    writePlyFile(stdout, verts, faces, numVerts, numFaces);

    exit(0);
}


int
readInventorFile(FILE *inFile, Vertex **pVerts, Face **pFaces, 
		 int *pNumVerts, int *pNumFaces)
{
   int i, j, k;

    // Initialize Inventor
    SoDB::init();

    SoInput in;
    
    // Set input file to read from
    in.setFilePointer(inFile);

    SoNode *node;
    SbBool ok;
    ok = SoDB::read(&in, node);
    if (!ok || node == NULL) 
       return 0;

    Vertex *verts = NULL;
    Normal *norms = NULL;
    Face *faces = NULL;
    FaceNorms *faceNorms = NULL;
    int numFaceNorms;
    SoSFEnum binding;

    SoGroup *group = (SoGroup *)node;
    for (i = 0; i < group->getNumChildren(); i++) {       
       if (group->getChild(i)->
	   isOfType(SoIndexedFaceSet::getClassTypeId())) {

	  SoMFInt32 &coordIndices = 
	     ((SoIndexedFaceSet *)group->getChild(i))->coordIndex;
	  int numIndices = coordIndices.getNum();
	  *pNumFaces = 0;
	  for (j = 0; j < numIndices; j++) {
	     if (coordIndices[j] == -1)
		*pNumFaces = *pNumFaces + 1;
	  }
	  faces = *pFaces = new Face[*pNumFaces];
	  int curFaceIndex = 0;
	  uchar curVerts = 0;
	  for (j = 0; j < numIndices; j++) {
	     if (coordIndices[j] == -1) {
		faces[curFaceIndex].nverts = curVerts;
		faces[curFaceIndex].verts = new int[curVerts];
		for (k = 0; k < curVerts; k++) {
		   faces[curFaceIndex].verts[k] = coordIndices[k+j-curVerts];
		}
		curFaceIndex++;
		curVerts = 0;
	     } else {
		curVerts++;
	     }
	  }

	  SoMFInt32 &normIndices = 
	     ((SoIndexedFaceSet *)group->getChild(i))->normalIndex;	  
	  numIndices = normIndices.getNum();
	  if (numIndices == 1 && normIndices[0] == -1) {
	     continue;
	  }
	  numFaceNorms = 0;
	  for (j = 0; j < numIndices; j++) {
	     if (normIndices[j] == -1)
		numFaceNorms++;
	  }
	  faceNorms = new FaceNorms[numFaceNorms];
	  curFaceIndex = 0;
	  curVerts = 0;
	  for (j = 0; j < numIndices; j++) {
	     if (normIndices[j] == -1) {
		faceNorms[curFaceIndex].nnorms = curVerts;
		faceNorms[curFaceIndex].norms = new int[curVerts];
		for (k = 0; k < curVerts; k++) {
		   faceNorms[curFaceIndex].norms[k] = 
		      normIndices[k+j-curVerts];
		}
		curFaceIndex++;
		curVerts = 0;
	     } else {
		curVerts++;
	     }
	  }

       }
       else if (group->getChild(i)->
	       isOfType(SoCoordinate3::getClassTypeId())) {
	  SoMFVec3f &points = ((SoCoordinate3 *)group->getChild(i))->point;
	  if (verts == NULL) {
	     *pNumVerts = points.getNum();
	     verts = *pVerts = new Vertex[*pNumVerts];
	  }
	  for (j = 0; j < *pNumVerts; j++) {
	     const float *vec = points[j].getValue();
	     verts[j].x = vec[0];
	     verts[j].y = vec[1];
	     verts[j].z = vec[2];
	  }
       }

       else if (group->getChild(i)->
	       isOfType(SoNormal::getClassTypeId())) {
	  SoMFVec3f &normals = ((SoNormal *)group->getChild(i))->vector;
	  FOUND_NORMALS = TRUE;
	  int numNorms = normals.getNum();
	  norms = new Normal[numNorms];
	  for (j = 0; j < numNorms; j++) {
	     const float *vec = normals[j].getValue();
	     norms[j][0] = vec[0];
	     norms[j][1] = vec[1];
	     norms[j][2] = vec[2];
	  }
       }
       else if (group->getChild(i)->
	       isOfType(SoNormalBinding::getClassTypeId())) {
	  binding = ((SoNormalBinding *)group->getChild(i))->value;
       }
    }

    if (FOUND_NORMALS && (binding.getValue() == SoNormalBinding::PER_VERTEX_INDEXED)) {
       if (faceNorms == NULL) {
	  for (j = 0; j < *pNumVerts; j++) {
	     verts[j].nx = norms[j][0];
	     verts[j].ny = norms[j][1];
	     verts[j].nz = norms[j][2];
	  }
       }
       else {
	  for (j = 0; j < numFaceNorms; j++) {
	     for (k = 0; k < faceNorms[j].nnorms; k++) {
		verts[faces[j].verts[k]].nx = norms[faceNorms[j].norms[k]][0];
		verts[faces[j].verts[k]].ny = norms[faceNorms[j].norms[k]][1];
		verts[faces[j].verts[k]].nz = norms[faceNorms[j].norms[k]][2];
	     }
	  }
       }
    }
    else {
       FOUND_NORMALS = FALSE;
    }

    return 1;
}


int
writePlyFile(FILE *outFile, Vertex *verts, Face *faces, 
		 int numVerts, int numFaces)
{
    int i, j;
    int nelems;
    char **elist;
    int file_type;
    float version;
    char *elem_name;
    int nprops, num_vert_props;
    int num_elems;
    PlyProperty **plist;
    int nvp;
    char *elem_names[] = {"vertex", "face"};

    PlyFile *ply = ply_write (stdout, 2, elem_names, PLY_BINARY_BE);

    if (ply == NULL) {
      fprintf(stderr, "Error, couldn't write output.\n");
      exit(-1);
    }

    nvp = 0;

    vert_props[nvp] = vert_prop_x;
    vert_props[nvp].offset = offsetof(Vertex,x); nvp++;
    vert_props[nvp] = vert_prop_y;
    vert_props[nvp].offset = offsetof(Vertex,y); nvp++;
    vert_props[nvp] = vert_prop_z;
    vert_props[nvp].offset = offsetof(Vertex,z); nvp++;

    if (FOUND_NORMALS) {
       vert_props[nvp] = vert_prop_nx;
       vert_props[nvp].offset = offsetof(Vertex,nx); nvp++;
       vert_props[nvp] = vert_prop_ny;
       vert_props[nvp].offset = offsetof(Vertex,ny); nvp++;
       vert_props[nvp] = vert_prop_nz;
       vert_props[nvp].offset = offsetof(Vertex,nz); nvp++;       
    }

    num_vert_props = nvp;

    face_props[0].offset = offsetof(Face, verts);
    face_props[0].count_offset = offsetof(Face, nverts);  /* count offset */
    
    ply_describe_element (ply, "vertex", numVerts, 
			  num_vert_props, vert_props);

    ply_describe_element (ply, "face", numFaces, 1, face_props);

    ply_header_complete (ply);
    
    /* set up and write the vertex elements */

    ply_put_element_setup (ply, "vertex");

    for (i = 0; i < numVerts; i++) {
	ply_put_element (ply, (void *) &verts[i]);
    }

    int vertIndices[3];

    ply_put_element_setup (ply, "face");

    for (i = 0; i < numFaces; i++) {
	ply_put_element (ply, (void *) &faces[i]);
    }
    
    return 1;
}


void
printUsage()
{
    printf("\n");
    printf("Usage: iv2ply [in.ply] > out.ply\n");
    printf("   or: iv2ply < in.ply > out.ply\n");
    printf("\n");
    printf("  Iv2ply converts an Inventor file containing a Group node\n");
    printf("  above a Coordinate3 and IndexedFaceSet node into a ply file.\n");
    printf("  Normals are preserved, but not color, etc.\n");
    printf("\n");
    exit(-1);
}

#endif
