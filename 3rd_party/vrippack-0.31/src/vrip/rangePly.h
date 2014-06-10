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


#ifndef _RANGE_GRID_
#define _RANGE_GRID_

#include "Mesh.h"
#include <limits.h>

typedef uchar vec3uc[3];

class RangeGrid {
  public:

    int nlg;		/* number of range columns (samples in x) */
    int nlt;		/* number of rows (samples in y) */
    int interlaced;       /* is it interlaced data? */
    int numSamples;       /* number of range samples */
    int numOrigSamples;       /* number of range samples */
    int maxSamples;       /* number of range samples */
    int isWarped;
    int isRightMirrorOpen;
    Vec3f viewDir;

    Vec3f *coords;	/* the range samples */
    float *confidence;    /* confidence */
    float *intensity;     /* intensity */
    vec3uc *matDiff;      /* color */
    int *indices;	        /* row x col indices to the positions */
    int hasColor;        /* color information? */
    int hasIntensity;    /* intensity information? */
    int hasConfidence;   /* confidence information? */
    int multConfidence;  /* multiply confidence? */
    char **obj_info; 
    int num_obj_info;

    ~RangeGrid();
};


int is_range_grid_file(const char *filename);
RangeGrid *readRangeGrid(const char *name);
RangeGrid *readRangeGridFillGaps(const char *name);
RangeGrid *readRangeGridExtendEdges(const char *name);
Mesh *meshFromGrid(RangeGrid *rangeGrid, int subSamp, int connectAll);
Mesh *cleanMeshFromRangeMesh(Mesh *inMesh);
void set_range_data_sigma_factor(float factor);
float get_range_data_sigma_factor();
void set_range_data_min_intensity(float intensity);
float get_range_data_min_intensity();

void find_mesh_edges(Mesh *mesh);
int vertex_edge_test(Mesh *mesh, Vertex *vert);
void addNeighbors(Vertex *v1, Vertex *v2);

#endif
