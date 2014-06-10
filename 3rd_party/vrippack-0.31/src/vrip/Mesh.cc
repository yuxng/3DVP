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


#include "Mesh.h"
#include <limits.h>
#include "rangePly.h"
#include "plyio.h"
#include "vripGlobals.h"

#ifdef linux
#include <float.h>
#endif

static void lower_edge_confidence(Mesh *mesh);
static void lower_edge_confidence_2(Mesh *mesh);


Mesh::Mesh()
{
    numVerts = 0;
    verts = NULL;
    
    numTris = 0;
    tris = NULL;

    hasConfidence = 0;
    hasColor = 0;
}


Mesh::~Mesh()
{
    if (verts != NULL) {
	for (int i = 0; i < numVerts; i++) {
	    delete [] verts[i].verts;
	    delete [] verts[i].edgeLengths;
	    delete [] verts[i].tris;
	}

	delete [] verts;
    }

    if (tris != NULL) {
	delete [] tris;
    }
}

 

void
Mesh::computeBBox()
{
    bbox.init();

    Vertex *buf = this->verts;
    for (int i = 0; i < this->numVerts; i++, buf++) {
	bbox.update(buf->coord);
    }
}


void
Mesh::initNormals()
{
    computeTriNormals();
    computeVertNormals();
}


void
Mesh::computeTriNormals()
{
    Vec3f v1, v2, v3, norm;
    Triangle *tri;
    int i;

    for (i = 0; i < numTris; i++) {
	tri = &tris[i];
	v1.setValue(verts[tri->vindex1].coord);
	v2.setValue(verts[tri->vindex2].coord);
	v3.setValue(verts[tri->vindex3].coord);
	v2 = v1 - v2;
	v3 = v1 - v3;
	tri->norm = v2.cross(v3);
	tri->norm.normalize();
    }
}

void
Mesh::computeVertNormals()
{
    Vec3f norm;
    int i, index;

    for (i = 0; i < numVerts; i++) {
	verts[i].norm.setValue(0, 0, 0);
    }

    for (i = 0; i < numTris; i++) {
	index = tris[i].vindex1;
	verts[index].norm += tris[i].norm;
	
	index = tris[i].vindex2;
	verts[index].norm += tris[i].norm;
	
	index = tris[i].vindex3;
	verts[index].norm += tris[i].norm;
    }

    for (i = 0; i < numVerts; i++) {
	verts[i].norm.normalize();
    }
}


void
doConfidence(Mesh *mesh, int perspective)
{
    float dotLaser, angleLaser, radAngleLaser;
    float dotCCD, angleCCD, radAngleCCD;
    float weight;
    Vec3f dirLaser, dirCCD;

    if (mesh->isWarped) {
	if (mesh->isRightMirrorOpen) {
	    angleCCD = 30;
	} else {
	    angleCCD = -30;
	}
	radAngleCCD = angleCCD*M_PI/180;
	dirCCD.setValue(sin(radAngleCCD), 0, cos(radAngleCCD));

	angleLaser = 0;
	radAngleLaser = angleLaser*M_PI/180;
	dirLaser.setValue(sin(radAngleLaser), 0, cos(radAngleLaser));

	for (int i = 0; i < mesh->numVerts; i++) {

	    // No confidence for "phony" polygons and their vertices
	    if (mesh->verts[i].holeFill) {
		mesh->verts[i].confidence = 0;
	    } else {
		dotCCD = dirCCD.dot(mesh->verts[i].norm);
		dotCCD = MAX(dotCCD,0);
		dotLaser = dirLaser.dot(mesh->verts[i].norm);
		dotLaser = MAX(dotLaser,0);

		
		weight = dotLaser*dotCCD;
		
		// Dividing by 0.9 scales it back up some.  Min is about 0.8
		//weight = pow(weight, ConfidenceExponent)/0.9;
		//if (weight < 0.001)
		//   weight = 0;

		weight = dotCCD;
		weight = pow(weight, ConfidenceExponent);

		mesh->verts[i].confidence *= weight;
	    }
	}
    }
    else if (!perspective) {
	angleLaser = 0;
	radAngleLaser = angleLaser*M_PI/180;
	dirLaser.setValue(sin(radAngleLaser), 0, cos(radAngleLaser));

	for (int i = 0; i < mesh->numVerts; i++) {
	    if (mesh->verts[i].holeFill) {
		mesh->verts[i].confidence = 0;
	    } else {
		dotLaser = dirLaser.dot(mesh->verts[i].norm);
		dotLaser = MAX(dotLaser,0);
		weight = pow(dotLaser, ConfidenceExponent);
		mesh->verts[i].confidence *= weight;
	    }
	}
    } else {
	angleLaser = 0;
	radAngleLaser = angleLaser*M_PI/180;
	dirLaser.setValue(sin(radAngleLaser), 0, cos(radAngleLaser));

	for (int i = 0; i < mesh->numVerts; i++) {
	    if (mesh->verts[i].holeFill) {
		mesh->verts[i].confidence = 0;
	    } else {
	        dirLaser = PerspectiveCOP - mesh->verts[i].coord;
		dirLaser.normalize();
		dotLaser = dirLaser.dot(mesh->verts[i].norm);
		dotLaser = MAX(dotLaser,0);
		weight = pow(dotLaser, ConfidenceExponent);
		mesh->verts[i].confidence *= weight;
	    }
	}
    }

    lower_edge_confidence(mesh);

    mesh->hasConfidence = 1;

}


/******************************************************************************
Lower the confidence value on edges.

Entry:
  mesh  - mesh on which to lower the edge confidence
  level - level of mesh detail
******************************************************************************/

static void
lower_edge_confidence(Mesh *mesh)
{
  int i,j,k;
  int pass;
  int val;
  float recip, weight;
  Vertex *v;
  int chew_count;

  switch (MeshResolution) {
    case 1:
      chew_count = EdgeConfSteps;
      break;
    case 2:
      chew_count = EdgeConfSteps/2;
      break;
    case 3:
      chew_count = EdgeConfSteps/4;
      break;
    case 4:
      chew_count = EdgeConfSteps/8;
      break;
  }

  if (chew_count == 0) {
     return;
  }

  MaxStepsToEdge = chew_count;

  for (i = 0; i < mesh->numVerts; i++) {
      
      v = &mesh->verts[i];
      if (v->stepsToEdge >= 0) {
	  if (v->on_edge) {
	      v->stepsToEdge = 0;
	  } else {
	      v->stepsToEdge = chew_count;
	  }
      }
  }

  /* make several passes through the vertices */
  for (pass = 1; pass < chew_count; pass++) {

    /* propagate higher on-edge values away from edges */
    for (i = 0; i < mesh->numVerts; i++) {

      v = &mesh->verts[i];
      if (v->on_edge != 0)
	continue;

      for (j = 0; j < v->numVerts; j++) {
	if (v->verts[j]->on_edge == pass) {
	  v->on_edge = pass+1;
	  v->stepsToEdge = pass;
	  break;
	}
      }
    }
  }


  /* lower the confidences on the edge */

  recip = 1.0 / (chew_count);

  for (i = 0; i < mesh->numVerts; i++) {
      v = &mesh->verts[i];
      val = v->on_edge;
      if (val) {
	  weight = (val-1) * recip;
	  weight = pow(weight, EdgeConfExponent);
	  v->confidence *= weight;
	  v->confidence += ConfidenceBias/255.0;
	  v->confidence = MIN(v->confidence, 1.0);
	  if (val > 1)
	      v->on_edge = 0;
      }
  }
}



static void
lower_edge_confidence_2(Mesh *mesh)
{
  int i,j,k;
  int pass;
  int val;
  float recip, max_dist, newDist;
  Vertex *v;
  int chew_count;

#if 0
  switch (level) {
    case 0:
      chew_count = (int)(8*CONF_EDGE_COUNT_FACTOR+0.5);
      break;
    case 1:
      chew_count = (int)(4*CONF_EDGE_COUNT_FACTOR+0.5);
      break;
    case 2:
      chew_count = (int)(2*CONF_EDGE_COUNT_FACTOR+0.5);
      break;
    case 3:
      chew_count = (int)(1*CONF_EDGE_COUNT_FACTOR+0.5);
      break;
    default:
      fprintf (stderr, "lower_edge_confidence: bad switch %d\n", level);
      exit (-1);
  }
#endif


    for (i = 0; i < mesh->numVerts; i++) {
      v = &mesh->verts[i];
      if (v->on_edge) {
	  v->distToBoundary = 0;
      } else {
	  v->distToBoundary = FLT_MAX;
      }
    }

  chew_count = 16;

  max_dist = chew_count * 0.0005;
  printf("Arbitrary scale factor in edge confidence!\n");

  /* make several passes through the vertices */
  for (pass = 1; pass < chew_count; pass++) {

    /* propagate higher on-edge values away from edges */
    for (i = 0; i < mesh->numVerts; i++) {

      v = &mesh->verts[i];
      if (v->on_edge)
	continue;

      for (j = 0; j < v->numVerts; j++) {
	if (v->verts[j]->distToBoundary != FLT_MAX) {
	    newDist = v->verts[j]->distToBoundary + v->edgeLengths[j];
	    v->distToBoundary = MIN(v->distToBoundary, newDist);
	}
      }
    }
  }

  /* lower the confidences on the edge */

  for (i = 0; i < mesh->numVerts; i++) {
      v = &mesh->verts[i];
      if (v->distToBoundary < max_dist) {
	  v->confidence *= v->distToBoundary/max_dist;
      }
  }
}


void
reallocVerts(Vertex *v)
{
    int i;
    Vertex **newVerts;
    float *newLengths;

    v->maxVerts *= 2;
    newVerts = new Vertex*[v->maxVerts];
    newLengths = new float[v->maxVerts];
    for (i = 0; i < v->numVerts; i++) {
        newVerts[i] = v->verts[i];
        newLengths[i] = v->edgeLengths[i];
    }
    delete [] v->verts;
    delete [] v->edgeLengths;

    v->verts = newVerts;
    v->edgeLengths = newLengths;
}


void
reallocTris(Vertex *v)
{
    int i;
    Triangle **newTris;

    v->maxTris *= 2;
    newTris = new Triangle*[v->maxTris];
    for (i = 0; i < v->numTris; i++) {
        newTris[i] = v->tris[i];
    }
    delete [] v->tris;

    v->tris = newTris;
}


Mesh *
cleanMesh(Mesh *inMesh)
{
  int i;

  Mesh *outMesh = new Mesh;

  outMesh->numVerts = 0;
  int max_verts = inMesh->numVerts;
  outMesh->verts = new Vertex[max_verts];

  outMesh->numTris = 0;
  int max_tris = inMesh->numTris;
  outMesh->tris = new Triangle[max_tris];

  outMesh->hasConfidence = inMesh->hasConfidence;
  outMesh->hasColor = inMesh->hasColor;

  int *vertRemap = new int[inMesh->numVerts];
  int outIndex = 0;
  for (i = 0; i < inMesh->numVerts; i++) {
     if (inMesh->verts[i].confidence < MinVertexConfidence) {
	vertRemap[i] = -1;
     } else {
	outMesh->verts[outIndex].coord = inMesh->verts[i].coord;
	vertRemap[i] = outIndex;
	if (inMesh->hasConfidence)
	   outMesh->verts[outIndex].confidence = inMesh->verts[i].confidence;
	else 
	   outMesh->verts[outMesh->numVerts].confidence = 1;

	if (inMesh->hasColor) {
	   outMesh->verts[outMesh->numVerts].red = inMesh->verts[i].red;
	   outMesh->verts[outMesh->numVerts].green = inMesh->verts[i].green;
	   outMesh->verts[outMesh->numVerts].blue = inMesh->verts[i].blue;
	}	
	outIndex++;
     }
  }
  
  outMesh->numVerts = outIndex;

  /* create the triangles */
  outIndex = 0;
  for (i = 0; i < inMesh->numTris; i++) {
     if (vertRemap[inMesh->tris[i].vindex1] == -1 ||
	 vertRemap[inMesh->tris[i].vindex2] == -1 ||
	 vertRemap[inMesh->tris[i].vindex3] == -1 ) {
	   continue;
	}
  
     outMesh->tris[outIndex].vindex1 = vertRemap[inMesh->tris[i].vindex1];
     outMesh->tris[outIndex].vindex2 = vertRemap[inMesh->tris[i].vindex2];
     outMesh->tris[outIndex].vindex3 = vertRemap[inMesh->tris[i].vindex3];
     outIndex++;
  }

  outMesh->numTris = outIndex;

  outMesh->initNormals();  

  find_mesh_edges(outMesh);

  prepareMesh(outMesh);

  return (outMesh);
}
