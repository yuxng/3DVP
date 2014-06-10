/*

Name:         mcfile.h

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Include file for Marching Cubes File Package 


Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


/*
 * Marching Cubes File Format
 *
 * ----------------------
 * |       header       |
 * ----------------------
 * |    surface data    |
 * ----------------------
 */

#ifdef __cplusplus
extern "C" {
#endif


/*
 * GENERAL
 */

typedef long Boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * HEADER STUFF
 */

typedef struct {
  int mc_magic;            /* magic number */
  int mc_type;             /* type of mc file */
  int mc_tmaptype;         /* type of texture map */
  int mc_length;           /* length (in bytes) of surface data */
} mcfile;

#define MC_MAGIC         0xFEDECABA

/* mc_types */
#define MCT_STANDARD  0    /* list of triangles */
#define MCT_TREE      1    /* pruned tree file */

/* mc_tmaptypes */
#define MCTM_NONE     0    /* no texture map */
#define MCTM_STANDARD 1    /* one value (0-255) per vertex */
#define MCTM_DOUBLE   2    /* two values per vertex */

/* functions */
extern Boolean IsMCFile(FILE *fp);
extern void MC_InitHeader(FILE *fp);
extern void MC_WriteHeader(FILE *fp, mcfile header);
extern mcfile MC_ReadHeader(FILE *fp);


/*
 * SURFACE DATA STUFF
 */

typedef struct {
  float x, y, z;        /* position components */
  float nx, ny, nz;     /* normal components */
  float confidence;
  unsigned char tex1;   /* first texture value */
  unsigned char tex2;   /* second texture value */
  signed char ncx, ncy, ncz;   /* used to ensure faces are 
				  oriented consistently */
  unsigned char realData;
  unsigned char valid;
} TriangleVertex;

/* functions */
extern void MC_WriteTriangleVertex(FILE *fp,TriangleVertex vertex);
extern TriangleVertex MC_ReadTriangleVertex(FILE *fp);


#ifdef __cplusplus
}
#endif

