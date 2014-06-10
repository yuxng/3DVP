#ifndef PLYIO_H
#define PLYIO_H

#include "Mesh.h"

// Read/write the ply file into the Mesh data structure.

Mesh *readPlyFile(FILE *inFile);
int   writePlyFile(FILE *outFile, Mesh *mesh, int numVerts, int numTris);

#endif // PLYIO_H
