/*

Brian Curless

Computer Graphics Laboratory
Stanford University

---------------------------------------------------------------------

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#ifndef _MESH_
#define _MESH_

#include "Linear.h"
#include "BBox3f.h"

typedef uchar vec3uc[3];


struct Triangle {
    int vindex1, vindex2, vindex3;
    Vec3f norm;
};


struct Vertex {
    Vec3f coord;
    Vec3f norm;

    float confidence;
    uchar red, green, blue;

    Triangle **tris;
    uchar numTris;
    uchar maxTris;

    Vertex **verts;
    float *edgeLengths;
    uchar numVerts;
    uchar maxVerts;

    float distToBoundary;

    uchar count;
    uchar on_edge;
    uchar holeFill;
    signed char stepsToEdge;
};


class Mesh {

  public:

    int numVerts;
    Vertex *verts;

    int numTris;
    Triangle *tris;

    int isWarped;
    int isRightMirrorOpen;
    int hasConfidence;
    int hasColor;

    Quaternion quat;
    Vec3f trans;

    BBox3f bbox;

    Mesh();
    ~Mesh();

    void initNormals();
    void computeTriNormals();
    void computeVertNormals();

    void computeBBox();
};

void doConfidence(Mesh *mesh, int perspective=0);

void reallocTris(Vertex *v);

void reallocVerts(Vertex *v);

Mesh *cleanMesh(Mesh *inMesh);

#endif

