#ifndef MESH_H
#define MESH_H

#include <Linear.h>


// Define the Data Structures, Mesh class.  
// (Merged the Mesh.h/.cc classes from triedgecol and
// trisliver...)

struct Triangle;

// Vertex -- used by both
struct Vertex {
  Vec3f coord;
  int index;	
  float confidence;	
  
  Triangle **tris;	
  uchar numTris;	

  Vertex **verts;	
  uchar numVerts;	
  
  uchar maxTris;	
  uchar maxVerts;

  bool onBoundary; // Is true if the vertex is on a mesh boundary.
};

// Triangle -- used by both
struct Triangle {
  Vertex *vert1, *vert2, *vert3;
  Vec3f norm;
};

// Mesh -- used by both
class Mesh {

  public:

  int numVerts;	
  Vertex *verts;
  
  int numTris;	
  Triangle *tris;

  Mesh();			
  ~Mesh();
};


#endif // MESH_H
