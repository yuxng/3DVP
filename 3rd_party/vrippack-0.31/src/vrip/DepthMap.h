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


#ifndef _DEPTH_MAP_
#define _DEPTH_MAP_

#include "vrip.h"

struct DepthElement {
    float z;
    float conf;
};


struct DepthTreeElement {
    float minZ;
    float maxZ;
};


struct DepthMapNorm {
    signed char nx;
    signed char ny;
    signed char nz;
};


class DepthMap {

  public:
    enum {TRIANGLE_ONE=0x1, TRIANGLE_TWO=0x2, 
	  TRIANGLE_THREE=0x4, TRIANGLE_FOUR=0x8};

    int xdim;
    int ydim;
    float resolution;
    vec2f origin;
    DepthElement *elems;
    uchar *tags;
    DepthMapNorm *norms;
    signed char *edgeSteps;
    DepthTreeElement ***treeElems;
    DepthTreeElement ***holeTreeElems;
    int *leafIndices;
    int *runs;
    int treeDepth;
    int treeGranularity;

    int origTreeDepth;
    int origXdim;
    int origYdim;

    int linePersp;
    int perspective;

    DepthMap();
    DepthMap(int,int,int);
    DepthMap(int,int,int,int);

    ~DepthMap();

    void tagCellsForResampling();
    int writePly(const char*, float noiseLevel);
    void updateNx();
    void updateNy();
    void updateNz();
    void updateEdgeSteps();
    void normalize();
    int reuse(int xd, int yd);
    void initTree(int granularity);
    void fillTree(int numLines);
    void pullUpTree(int y, int level);
    int * getDepthRuns(float y, float zmin, float zmax, int *numRuns);
    void consolidateRuns(int *leafIndices, int numLeaves, int *numRuns);
    void traverseTree(int y, int level, int off, float zmin, float zmax,
		      int *leafIndices, int *numLeaves);
    int *getDepthRunsUpperBound(float y, float zmax, int *numRuns);
    void traverseTreeUpperBound(int y, int level, int off, float zmax,
				int *leafIndices, int *numLeaves);
    int *getDepthRunsEdges(float y, float zmax, int *numRuns);
    void traverseTreeEdges(int y, int level, int off, float zmax,
				int *leafIndices, int *numLeaves);
};


#endif
