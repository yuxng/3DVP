#include <stdio.h>
#include <stdlib.h>
#include "Mesh.h"



// Initialize the Mesh
Mesh::Mesh()
{
    numVerts = 0;
    verts = NULL;
    
    numTris = 0;
    tris = NULL;
}

// Clean up the Mesh
Mesh::~Mesh()
{
  // Free verts, and each vertex's pointers...
  if (verts != NULL) {
    for (int i = 0; i < numVerts; i++) {
      delete [] verts[i].verts;
      delete [] verts[i].tris;
    }
    delete [] verts;
  }

  // Free tris and edges
  if (tris  != NULL) { delete [] tris;  }
}
