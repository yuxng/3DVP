/*

Converts from Paul Ning's polygon file format to text polygon description.

Greg Turk - November 1992

*/

#include <stdio.h>
#include <malloc.h>
#include "mcfile.h"
#include <math.h>
#include <string.h>
#include <Inventor/So.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/actions/SoWriteAction.h>

TriangleVertex *verts;
int npolys;


void write_polys_inventor(FILE *fp);
void write_polys(FILE *fp_out);


/******************************************************************************
Main routine.
******************************************************************************/

void
main(int, char **argv)
{
  int i,j;
  FILE *fp;
  FILE *fp_out;
  char infile[80],outfile[80];
  mcfile header;
  int index;

  strcpy (infile, argv[1]);
  strcpy (outfile, argv[2]);

  if (strlen (infile) < 3 ||
      strcmp (infile + strlen (infile) - 3, ".mc") != 0)
      strcat (infile, ".mc");

  if (strlen (outfile) < 5 ||
      strcmp (outfile + strlen (outfile) - 5, ".poly") != 0)
      strcat (outfile, ".poly");

  printf ("reading polygons from '%s'\n", infile);

  /* open the polygon input file */

  if ((fp = fopen(infile, "r")) == NULL) {
    fprintf (stderr, "bad open\n");
    exit (-1);
  }

  /* open the polygon output file */

  if ((fp_out = fopen(outfile, "w+b")) == NULL) {
    fprintf (stderr, "bad open\n");
    exit (-1);
  }

  /* read header info from the polygon file */
  header = MC_ReadHeader (fp);

  /*
  npolys = header.mc_length / (3 * sizeof (sizeof(TriangleVertex)));
  */
  npolys = header.mc_length / 42;
  printf ("%d polygons\n", npolys);
  verts = (TriangleVertex *) malloc (sizeof (TriangleVertex) * 3 * npolys);
  if (verts == 0) {
    fprintf (stderr, "can't allocate enough space\n");
    exit (-1);
  }

  /* read in the polygons */
  printf ("reading polygons...\n");
  for (i = 0; i < npolys; i++) {
    for (j = 0; j < 3; j++) {
      index = i * 3 + j;
      verts[index] = MC_ReadTriangleVertex (fp);
    }
  }

  /* write polygons to output file */
  printf ("writing polygons...\n");
  write_polys_inventor (fp_out);
  fclose (fp_out);

  printf ("done.\n");
}


/******************************************************************************
Write out polygons in Inventor format.
******************************************************************************/

void
write_polys_inventor(FILE *fp)
{
    int i,j;
    int index;
    float x,y,z;
    float nx,ny,nz;
    float len;
    
    SoDB::init();
    
    SoSeparator *root = new SoSeparator;
    root->ref();
    
    SoCoordinate3 *coord = new SoCoordinate3;
    root->addChild(coord);
    
    coord->point.insertSpace(0, npolys*3);
    
    for (i = 0; i < npolys; i++) {
	for (j = 0; j < 3; j++) {
	    index = i * 3 + j;
	    x = verts[index].x / 128.0;
	    y = verts[index].y / 128.0;
	    z = verts[index].z / 128.0;
	    coord->point.set1Value(index, x, y, z);
	}
    }    
    
/*
    SoNormal *norm = new SoNormal;
    root->addChild(norm);
    
    norm->vector.insertSpace(0, npolys*3);
    
    for (i = 0; i < npolys; i++) {
	for (j = 0; j < 3; j++) {
	    index = i * 3 + j;
	    nx = -verts[index].nx;
	    ny = -verts[index].ny;
	    nz = -verts[index].nz;
	    len = sqrt (nx*nx + ny*ny + nz*nz);
	    nx /= len;
	    ny /= len;
	    nz /= len;
	    norm->vector.set1Value(index, nx, ny, nz);
	}
    }
*/
    
    SoIndexedFaceSet *faceSet = new SoIndexedFaceSet;
    root->addChild(faceSet);
    
    faceSet->coordIndex.insertSpace(0, npolys*4);
    index = 0;
    for (i = 0; i < npolys; i++) {
	SbVec3f v1(verts[i*3].x, verts[i*3].y, verts[i*3].z);
	SbVec3f v2(verts[i*3+1].x, verts[i*3+1].y, verts[i*3+1].z);
	SbVec3f v3(verts[i*3+2].x, verts[i*3+2].y, verts[i*3+2].z);

	SbVec3f a = v2 - v1;
	SbVec3f b = v3 - v1;
	SbVec3f c = a.cross(b);
	SbVec3f norm(-verts[i*3].nx, -verts[i*3].ny, -verts[i*3].nz);
	if (c.dot(norm) > 0) {
	    faceSet->coordIndex.set1Value(index, i*3);
	    faceSet->coordIndex.set1Value(index+1, i*3+1);
	    faceSet->coordIndex.set1Value(index+2, i*3+2);
	    faceSet->coordIndex.set1Value(index+3, -1);
	}
	else {
	    faceSet->coordIndex.set1Value(index, i*3+1);
	    faceSet->coordIndex.set1Value(index+1, i*3);
	    faceSet->coordIndex.set1Value(index+2, i*3+2);
	    faceSet->coordIndex.set1Value(index+3, -1);
	}
	index += 4;
    }    

    SoWriteAction wa;
    wa.getOutput()->setFilePointer(fp);
    wa.getOutput()->setBinary(TRUE);
    wa.apply(root);
}


    /* write out the vertices */

/*
  fprintf (fp, "#Inventor V1.0 ascii\n");
  fprintf (fp, "Separator {\n");

  fprintf (fp, "Coordinate3 {\n");
  fprintf (fp, "point [\n");
  for (i = 0; i < npolys; i++) {
    for (j = 0; j < 3; j++) {
      index = i * 3 + j;
      x = verts[index].x / 128.0;
      y = verts[index].y / 128.0;
      z = verts[index].z / 128.0;
      fprintf (fp, "%f %f %f,\n", x, y, z);
    }
  }
  fprintf (fp, "]\n");
  fprintf (fp, "}\n");
*/

  /* write out the vertex normals */

/*
  fprintf (fp, "Normal {\n");
  fprintf (fp, "vector [\n");
  for (i = 0; i < npolys; i++) {
    for (j = 0; j < 3; j++) {
      index = i * 3 + j;
      nx = verts[index].nx;
      ny = verts[index].ny;
      nz = verts[index].nz;
      len = sqrt (nx*nx + ny*ny + nz*nz);
      nx /= len;
      ny /= len;
      nz /= len;
      fprintf (fp, "%f %f %f,\n", -nx, -ny, -nz);
    }
  }
  fprintf (fp, "]\n");
  fprintf (fp, "}\n");
*/

  /* write out the vertex indices for each face */

/*
  fprintf (fp, "IndexedFaceSet {\n");
  fprintf (fp, "coordIndex [\n");
  for (i = 0; i < npolys; i++) {
    fprintf (fp, "%d, %d, %d, -1,\n", i*3, i*3+1, i*3+2);
  }
  fprintf (fp, "]\n");
  fprintf (fp, "}\n");

  fprintf (fp, "}\n");
*/



/******************************************************************************
Write out polygons in my format.
******************************************************************************/

void
write_polys(FILE *fp_out)
{
  int i,j;
  int index;
  float x,y,z;
  float nx,ny,nz;
  float len;

  fprintf (fp_out, "vertices: %d\n", npolys * 3);
  fprintf (fp_out, "faces: %d\n", npolys);

  /* write out the vertices */
  for (i = 0; i < npolys; i++) {
    for (j = 0; j < 3; j++) {
      index = i * 3 + j;
      x = verts[index].x / 128.0;
      y = verts[index].y / 128.0;
      z = verts[index].z / 128.0;
      nx = verts[index].nx;
      ny = verts[index].ny;
      nz = verts[index].nz;
      len = sqrt (nx*nx + ny*ny + nz*nz);
      nx /= len;
      ny /= len;
      nz /= len;
      fprintf (fp_out, "v %f %f %f  %f %f %f\n", x, y, z, nx, ny, nz);
    }
  }

  /* write out the vertex indices for each face */
  for (i = 0; i < npolys; i++) {
    fprintf (fp_out, "f %d %d %d\n", i*3+1, i*3+2, i*3+3);
  }
}

