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


#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/times.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>

#include "plyio.h"
#include "DepthMap.h"
#include "OccGrid.h"
#include "renderGeom.h"
#include "defines.h"
#include "Matrix4f.h"
#include "vrip.h"
#include "vripGlobals.h"
#include "linePersp.h"
#include "perspective.h"

#include "sl_export.H"
#include "sl_vertex.H"
#include "sl_texture.H"
#include "sl_framebuffer.H"


static FB_IntensityBuffer theZBuffer;

static int ClockWise;

static clock_t tm;
static void start_time();
static void end_time();
static float time_elapsed();
void Clear(FB_IntensityBuffer *buffer, Real Z, Real I);
void drawMeshConfidence(Mesh *mesh, FB_IntensityBuffer *zbuffer);
void drawMeshEdgeSteps(Mesh *mesh, FB_IntensityBuffer *zbuffer);
void drawMeshNx(Mesh *mesh, FB_IntensityBuffer *zbuffer);
void drawMeshNy(Mesh *mesh, FB_IntensityBuffer *zbuffer);
void drawMeshNz(Mesh *mesh, FB_IntensityBuffer *zbuffer);

const int SCREEN_XDIM = 1280;
const int SCREEN_YDIM = 1024;

void transformVerts(Mesh *mesh, Matrix4f *mfinal);
void transformNorms(Mesh *mesh, Matrix4f *mrot);
void transformVertsLinePersp(Mesh *mesh, Matrix4f &mmodel, Matrix4f &mvport);
void transformVertsPersp(Mesh *mesh, Matrix4f &mmodel, Matrix4f &mvport);

static void
start_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime;
}

static void
end_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime - tm;
}

static float
time_elapsed()
{
  return (double) tm / (double) CLOCKS_PER_SEC;
}


void
prepareRender(Mesh *mesh, OrthoShear *shear, BBox3f *bbox, 
	      float resolution, DepthMap *dm, int useNorms)
{
    int xdim, ydim, xorg, yorg;
    float new_xnur, new_ynur;

    xdim = int(ceil((bbox->nur.x - bbox->fll.x)/resolution) + 3);
    ydim = int(ceil((bbox->nur.y - bbox->fll.y)/resolution) + 3);

    if (dm == NULL)
	dm = new DepthMap(xdim, ydim, useNorms, DEPTH_TREE_GRANULARITY);
    else
	dm->reuse(xdim, ydim);

    dm->origin[0] = (int(bbox->fll[0]/resolution)-1)*resolution;
    dm->origin[1] = (int(bbox->fll[1]/resolution)-1)*resolution;
    dm->resolution = resolution;
    dm->linePersp = 0;
    dm->perspective = 0;

    theZBuffer.width = xdim;
    theZBuffer.height = ydim;
    theZBuffer.sampleZI = (FB_SampleZI *)dm->elems;

    new_xnur = (xdim-1)*resolution + bbox->fll.x;
    new_ynur = (ydim-1)*resolution + bbox->fll.y;

    if (Verbose)
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll.x, bbox->fll.y, bbox->fll.z, 
	       new_xnur, new_ynur, bbox->nur.z);

    Matrix4f mrot;
    mesh->quat.toMatrix(mrot);

    if (useNorms)
	transformNorms(mesh, &mrot);

    Matrix4f mtrans;
    mtrans.makeIdentity();
    mtrans.translate(mesh->trans);

    Matrix4f maxes;
    maxes.makeIdentity();
    if (shear->axis == X_AXIS) {
	maxes.m[0][0] = 0;
	maxes.m[0][2] = 1;
	maxes.m[2][2] = 0;
	maxes.m[2][0] = 1;
    } 
    else if (shear->axis == Y_AXIS) {
	maxes.m[1][1] = 0;
	maxes.m[1][2] = 1;
	maxes.m[2][2] = 0;
	maxes.m[2][1] = 1;
    }

    Matrix4f mflipz;
    mflipz.makeIdentity();
    if (shear->flip)
	mflipz.m[2][2] = -1;

    Matrix4f msh;
    msh.makeIdentity();
    msh.m[0][2] = shear->sx;
    msh.m[1][2] = shear->sy;

    //Matrix4f mztrans;
    //mztrans.makeIdentity();
    //mztrans.m[2][3] = -frontRLEGrid->origin[2];

    Matrix4f mfinal;
    mfinal.makeIdentity();
    mfinal.multLeft(mrot);
    mfinal.multLeft(mtrans);
    mfinal.multLeft(maxes);
    mfinal.multLeft(mflipz);
    //    mfinal.multLeft(mztrans);
    mfinal.multLeft(msh);

    if (Verbose) {
	printf("Transformation:\n");
	mfinal.print();
    }

    Matrix4f vtrans;
    vtrans.makeIdentity();
    vtrans.translate(-dm->origin[0], -dm->origin[1], 0);
    Matrix4f vscale;
    vscale.makeIdentity();
    vscale.setScale(1/resolution, 1/resolution, 1);

    mfinal.multLeft(vtrans);
    mfinal.multLeft(vscale);

    transformVerts(mesh, &mfinal);

    ClockWise = (shear->axis == Z_AXIS) ^ (shear->flip);
}


void
prepareRenderLinePersp(Mesh *mesh, OrthoShear *shear, BBox3f *bbox, 
		       float resolution, DepthMap *dm, int useNorms)
{
    int xdim, ydim, xorg, yorg;
    float new_xnur, new_ynur;

    xdim = int(ceil((bbox->nur.x - bbox->fll.x)/resolution) + 3);
    ydim = int(ceil((bbox->nur.y - bbox->fll.y)/resolution) + 3);

    if (dm == NULL)
	dm = new DepthMap(xdim, ydim, useNorms, DEPTH_TREE_GRANULARITY);
    else
	dm->reuse(xdim, ydim);

    dm->origin[0] = (int(bbox->fll[0]/resolution)-1)*resolution;
    dm->origin[1] = (int(bbox->fll[1]/resolution)-1)*resolution;
    dm->resolution = resolution;
    dm->linePersp = 1;
    dm->perspective = 0;

    theZBuffer.width = xdim;
    theZBuffer.height = ydim;
    theZBuffer.sampleZI = (FB_SampleZI *)dm->elems;

    new_xnur = (xdim-1)*resolution + bbox->fll.x;
    new_ynur = (ydim-1)*resolution + bbox->fll.y;

    if (Verbose)
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll.x, bbox->fll.y, bbox->fll.z, 
	       new_xnur, new_ynur, bbox->nur.z);
    
    Matrix4f mmodel, mvport, mscale, mpermute, mflipz;
    Vec3f trans;

    mpermute.makeIdentity();
    if (shear->axis == X_AXIS) {
	mpermute.m[0][0] = 0;
	mpermute.m[0][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][0] = 1;
    } 
    else if (shear->axis == Y_AXIS) {
	mpermute.m[1][1] = 0;
	mpermute.m[1][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][1] = 1;
    }

    // Set up flip matrix

    mflipz.makeIdentity();
    if (shear->flip)
	mflipz.m[2][2] = -1;

    mesh->quat.toMatrix(mmodel);

    if (useNorms)
	transformNorms(mesh, &mmodel);

    mmodel.translate(mesh->trans);
    mmodel.multLeft(mpermute);
    mmodel.multLeft(mflipz);

    trans.setValue(-dm->origin[0], -dm->origin[1], 0);
    mvport.makeIdentity();
    mvport.translate(trans);
    mscale.makeIdentity();
    mscale.scale(1/resolution, 1/resolution, 1);
    mvport.multLeft(mscale);

    transformVertsLinePersp(mesh, mmodel, mvport);

    ClockWise = (shear->axis == Z_AXIS) ^ (shear->flip);
}


void
prepareRenderPersp(Mesh *mesh, OrthoShear *shear, BBox3f *bbox, 
		   float resolution, DepthMap *dm, int useNorms)
{
    int xdim, ydim, xorg, yorg;
    float new_xnur, new_ynur;

    xdim = int(ceil((bbox->nur.x - bbox->fll.x)/resolution) + 3);
    ydim = int(ceil((bbox->nur.y - bbox->fll.y)/resolution) + 3);

    if (dm == NULL) {
	dm = new DepthMap(xdim, ydim, useNorms, DEPTH_TREE_GRANULARITY);
	theDepthMap = dm;
    } 
    else {
       if (!dm->reuse(xdim, ydim)) {
	  delete dm;
	  dm = new DepthMap(xdim, ydim, useNorms, DEPTH_TREE_GRANULARITY);
	  theDepthMap = dm;
       }
    }
       
    dm->origin[0] = (int(bbox->fll[0]/resolution)-1)*resolution;
    dm->origin[1] = (int(bbox->fll[1]/resolution)-1)*resolution;
    dm->resolution = resolution;
    dm->linePersp = 0;
    dm->perspective = 1;

    theZBuffer.width = xdim;
    theZBuffer.height = ydim;
    theZBuffer.sampleZI = (FB_SampleZI *)dm->elems;

    new_xnur = (xdim-1)*resolution + bbox->fll.x;
    new_ynur = (ydim-1)*resolution + bbox->fll.y;

    if (Verbose)
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll.x, bbox->fll.y, bbox->fll.z, 
	       new_xnur, new_ynur, bbox->nur.z);
    
    Matrix4f mmodel, mvport, mscale, mpermute, mflipz;
    Vec3f trans;

    mpermute.makeIdentity();
    if (shear->axis == X_AXIS) {
	mpermute.m[0][0] = 0;
	mpermute.m[0][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][0] = 1;
    } 
    else if (shear->axis == Y_AXIS) {
	mpermute.m[1][1] = 0;
	mpermute.m[1][2] = 1;
	mpermute.m[2][2] = 0;
	mpermute.m[2][1] = 1;
    }

    // Set up flip matrix

    mflipz.makeIdentity();
    if (shear->flip)
	mflipz.m[2][2] = -1;

    mesh->quat.toMatrix(mmodel);

    if (useNorms)
	transformNorms(mesh, &mmodel);

    mmodel.translate(mesh->trans);
    mmodel.multLeft(mpermute);
    mmodel.multLeft(mflipz);

    trans.setValue(-dm->origin[0], -dm->origin[1], 0);
    mvport.makeIdentity();
    mvport.translate(trans);
    mscale.makeIdentity();
    mscale.scale(1/resolution, 1/resolution, 1);
    mvport.multLeft(mscale);

    transformVertsPersp(mesh, mmodel, mvport);

    ClockWise = (shear->axis == Z_AXIS) ^ (shear->flip);
}

void
softRenderConfidence(Mesh *mesh) 
{
    start_time();

    Clear(&theZBuffer, -MAXFLOAT, 0.0f);
    drawMeshConfidence(mesh, &theZBuffer);

    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());
}

void
softRenderEdgeSteps(Mesh *mesh) 
{
    start_time();

    Clear(&theZBuffer, -MAXFLOAT, 0.0f);
    drawMeshEdgeSteps(mesh, &theZBuffer);

    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());
}


void
softRenderNx(Mesh *mesh) 
{
    start_time();

    Clear(&theZBuffer, -MAXFLOAT, 0.0f);
    drawMeshNx(mesh, &theZBuffer);

    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());
}


void
softRenderNy(Mesh *mesh) 
{
    start_time();

    Clear(&theZBuffer, -MAXFLOAT, 0.0f);
    drawMeshNy(mesh, &theZBuffer);

    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());
}


void
softRenderNz(Mesh *mesh) 
{
    start_time();

    Clear(&theZBuffer, -MAXFLOAT, 0.0f);
    drawMeshNz(mesh, &theZBuffer);

    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());
}


void 
transformNorms(Mesh *mesh, Matrix4f *mrot)
{
    Vertex *vert;
    vert = mesh->verts;
    for (int j = 0; j < mesh->numVerts; j++, vert++) {
	mrot->multVec(vert->norm, vert->norm);
    }
}


void
transformVerts(Mesh *mesh, Matrix4f *mfinal)
{
    Vertex *vert;
    float m00,m01,m02,m03;
    float m10,m11,m12,m13;
    float m20,m21,m22,m23;
    float m30,m31,m32,m33;
    float x,y,z,w,overw;
    
    m00 = mfinal->m[0][0]; m01 = mfinal->m[0][1]; 
    m02 = mfinal->m[0][2]; m03 = mfinal->m[0][3];

    m10 = mfinal->m[1][0]; m11 = mfinal->m[1][1]; 
    m12 = mfinal->m[1][2]; m13 = mfinal->m[1][3];

    m20 = mfinal->m[2][0]; m21 = mfinal->m[2][1]; 
    m22 = mfinal->m[2][2]; m23 = mfinal->m[2][3];

    m30 = mfinal->m[3][0]; m31 = mfinal->m[3][1]; 
    m32 = mfinal->m[3][2]; m33 = mfinal->m[3][3];

    vert = mesh->verts;
    for (int j = 0; j < mesh->numVerts; j++, vert++) {
	x = vert->coord.x;
	y = vert->coord.y;
	z = vert->coord.z;
	w = 1;
	  
	overw = 1/(m30*x + m31*y + m32*z + m33*w);
	overw = 1;
	vert->coord.x = (m00*x + m01*y + m02*z + m03*w)*overw;
	vert->coord.y = (m10*x + m11*y + m12*z + m13*w)*overw;
	vert->coord.z = (m20*x + m21*y + m22*z + m23*w)*overw;	
    }
}


void
transformVertsLinePersp(Mesh *mesh, Matrix4f &mmodel, Matrix4f &mvport)
{
    Vec3f v1, v2;
    Vertex *vert;
    
    vert = mesh->verts;
    for (int j = 0; j < mesh->numVerts; j++, vert++) {
	mmodel.multVec(vert->coord, v1);
	applyLinePersp(v1, v2);
	mvport.multVec(v2, vert->coord);
    }
}


void
transformVertsPersp(Mesh *mesh, Matrix4f &mmodel, Matrix4f &mvport)
{
   Vec3f v1, v2;
   Vertex *vert;
   
   float dist;
   
   vert = mesh->verts;
   for (int j = 0; j < mesh->numVerts; j++, vert++) {
      mmodel.multVec(vert->coord, v1);
      applyPersp(v1, v2);
      mvport.multVec(v2, vert->coord);
   }
}


void
drawMeshConfidence(Mesh *mesh, FB_IntensityBuffer *zbuffer)
{
    Vertex *v0, *v1, *v2;
    float x1, y1, x2, y2, crossz;
    IS_Vertex_ZI vzi0, vzi1, vzi2;
    Triangle *tri = mesh->tris;
    for (int j = 0; j < mesh->numTris; j++, tri++) {
	v0 = mesh->verts + tri->vindex1;
	v1 = mesh->verts + tri->vindex2;
	v2 = mesh->verts + tri->vindex3;
	if (v0->coord.x < -0.5 || v0->coord.x > zbuffer->width-0.5 ||
	    v0->coord.y < -0.5 || v0->coord.y > zbuffer->height-0.5 ||
	    v1->coord.x < -0.5 || v1->coord.x > zbuffer->width-0.5 ||
	    v1->coord.y < -0.5 || v1->coord.y > zbuffer->height-0.5 ||
	    v2->coord.x < -0.5 || v2->coord.x > zbuffer->width-0.5 ||
	    v2->coord.y < -0.5 || v2->coord.y > zbuffer->height-0.5)
	    continue;

	x1 = v0->coord.x - v1->coord.x;
	y1 = v0->coord.y - v1->coord.y;

	x2 = v0->coord.x - v2->coord.x;
	y2 = v0->coord.y - v2->coord.y;

	crossz = x1*y2 - x2*y1;

	if (crossz < 0 && ClockWise) {
	    vzi0.I = -1;
	    vzi1.I = -1;
	    vzi2.I = -1;
	} else {
	    vzi0.I = v0->confidence;
	    vzi1.I = v1->confidence;
	    vzi2.I = v2->confidence;
	}

	vzi0.x = v0->coord.x;
	vzi0.y = v0->coord.y;
	vzi0.Z = v0->coord.z;

	vzi1.x = v1->coord.x;
	vzi1.y = v1->coord.y;
	vzi1.Z = v1->coord.z;

	vzi2.x = v2->coord.x;
	vzi2.y = v2->coord.y;
	vzi2.Z = v2->coord.z;

	SL_SCTriangle_DDA(&vzi0, &vzi1, &vzi2, 
			  (IS_CAttr_None *) NULL, zbuffer);
  }

  for (int i = 0; i < zbuffer->width * zbuffer->height; i++) {
      if (zbuffer->sampleZI[i].I < 0) {
	  zbuffer->sampleZI[i].Z = -MAXFLOAT;
	  zbuffer->sampleZI[i].I = 0;
      }
  }

}


void
drawMeshEdgeSteps(Mesh *mesh, FB_IntensityBuffer *zbuffer)
{
    Vertex *v0, *v1, *v2;
    float x1, y1, x2, y2, crossz;
    IS_Vertex_ZI vzi0, vzi1, vzi2;
    Triangle *tri = mesh->tris;
    for (int j = 0; j < mesh->numTris; j++, tri++) {
	v0 = mesh->verts + tri->vindex1;
	v1 = mesh->verts + tri->vindex2;
	v2 = mesh->verts + tri->vindex3;
	if (v0->coord.x < -0.5 || v0->coord.x > zbuffer->width-0.5 ||
	    v0->coord.y < -0.5 || v0->coord.y > zbuffer->height-0.5 ||
	    v1->coord.x < -0.5 || v1->coord.x > zbuffer->width-0.5 ||
	    v1->coord.y < -0.5 || v1->coord.y > zbuffer->height-0.5 ||
	    v2->coord.x < -0.5 || v2->coord.x > zbuffer->width-0.5 ||
	    v2->coord.y < -0.5 || v2->coord.y > zbuffer->height-0.5)
	    continue;

	vzi0.x = v0->coord.x;
	vzi0.y = v0->coord.y;
	vzi0.Z = v0->coord.z;
	vzi0.I = float(v0->stepsToEdge)/MaxStepsToEdge;

	vzi1.x = v1->coord.x;
	vzi1.y = v1->coord.y;
	vzi1.Z = v1->coord.z;
	vzi1.I = float(v1->stepsToEdge)/MaxStepsToEdge;

	vzi2.x = v2->coord.x;
	vzi2.y = v2->coord.y;
	vzi2.Z = v2->coord.z;
	vzi2.I = float(v2->stepsToEdge)/MaxStepsToEdge;

/*
	if (vzi0.I < 0) {
	    vzi0.I = 1.0;
	}
	if (vzi1.I < 0) {
	    vzi1.I = 1.0;
	}
	if (vzi2.I < 0) {
	    vzi2.I = 1.0;
	}
	*/
	
	SL_SCTriangle_DDA(&vzi0, &vzi1, &vzi2, 
			  (IS_CAttr_None *) NULL, zbuffer);
  }

/*
  for (int i = 0; i < zbuffer->width * zbuffer->height; i++) {
      if (zbuffer->sampleZI[i].I < 0) {
	  zbuffer->sampleZI[i].Z = -MAXFLOAT;
      }
  }
  */

}


void
drawMeshNx(Mesh *mesh, FB_IntensityBuffer *zbuffer)
{
    Vertex *v0, *v1, *v2;
    IS_Vertex_ZI vzi0, vzi1, vzi2;
    Triangle *tri = mesh->tris;
    for (int j = 0; j < mesh->numTris; j++, tri++) {
	v0 = mesh->verts + tri->vindex1;
	v1 = mesh->verts + tri->vindex2;
	v2 = mesh->verts + tri->vindex3;
	if (v0->coord.x < -0.5 || v0->coord.x > zbuffer->width-0.5 ||
	    v0->coord.y < -0.5 || v0->coord.y > zbuffer->height-0.5 ||
	    v1->coord.x < -0.5 || v1->coord.x > zbuffer->width-0.5 ||
	    v1->coord.y < -0.5 || v1->coord.y > zbuffer->height-0.5 ||
	    v2->coord.x < -0.5 || v2->coord.x > zbuffer->width-0.5 ||
	    v2->coord.y < -0.5 || v2->coord.y > zbuffer->height-0.5)
	    continue;

	vzi0.x = v0->coord.x;
	vzi0.y = v0->coord.y;
	vzi0.Z = v0->coord.z;
	vzi0.I = v0->norm.x;

	vzi1.x = v1->coord.x;
	vzi1.y = v1->coord.y;
	vzi1.Z = v1->coord.z;
	vzi1.I = v1->norm.x;

	vzi2.x = v2->coord.x;
	vzi2.y = v2->coord.y;
	vzi2.Z = v2->coord.z;
	vzi2.I = v2->norm.x;

	SL_SCTriangle_DDA(&vzi0, &vzi1, &vzi2, 
			  (IS_CAttr_None *) NULL, zbuffer);
  }
}

void
drawMeshNy(Mesh *mesh, FB_IntensityBuffer *zbuffer)
{
    Vertex *v0, *v1, *v2;
    IS_Vertex_ZI vzi0, vzi1, vzi2;
    Triangle *tri = mesh->tris;
    for (int j = 0; j < mesh->numTris; j++, tri++) {
	v0 = mesh->verts + tri->vindex1;
	v1 = mesh->verts + tri->vindex2;
	v2 = mesh->verts + tri->vindex3;
	if (v0->coord.x < -0.5 || v0->coord.x > zbuffer->width-0.5 ||
	    v0->coord.y < -0.5 || v0->coord.y > zbuffer->height-0.5 ||
	    v1->coord.x < -0.5 || v1->coord.x > zbuffer->width-0.5 ||
	    v1->coord.y < -0.5 || v1->coord.y > zbuffer->height-0.5 ||
	    v2->coord.x < -0.5 || v2->coord.x > zbuffer->width-0.5 ||
	    v2->coord.y < -0.5 || v2->coord.y > zbuffer->height-0.5)
	    continue;

	vzi0.x = v0->coord.x;
	vzi0.y = v0->coord.y;
	vzi0.Z = v0->coord.z;
	vzi0.I = v0->norm.y;

	vzi1.x = v1->coord.x;
	vzi1.y = v1->coord.y;
	vzi1.Z = v1->coord.z;
	vzi1.I = v1->norm.y;

	vzi2.x = v2->coord.x;
	vzi2.y = v2->coord.y;
	vzi2.Z = v2->coord.z;
	vzi2.I = v2->norm.y;

	SL_SCTriangle_DDA(&vzi0, &vzi1, &vzi2, 
			  (IS_CAttr_None *) NULL, zbuffer);
  }

}

void
drawMeshNz(Mesh *mesh, FB_IntensityBuffer *zbuffer)
{
    Vertex *v0, *v1, *v2;
    IS_Vertex_ZI vzi0, vzi1, vzi2;
    Triangle *tri = mesh->tris;
    for (int j = 0; j < mesh->numTris; j++, tri++) {
	v0 = mesh->verts + tri->vindex1;
	v1 = mesh->verts + tri->vindex2;
	v2 = mesh->verts + tri->vindex3;
	if (v0->coord.x < -0.5 || v0->coord.x > zbuffer->width-0.5 ||
	    v0->coord.y < -0.5 || v0->coord.y > zbuffer->height-0.5 ||
	    v1->coord.x < -0.5 || v1->coord.x > zbuffer->width-0.5 ||
	    v1->coord.y < -0.5 || v1->coord.y > zbuffer->height-0.5 ||
	    v2->coord.x < -0.5 || v2->coord.x > zbuffer->width-0.5 ||
	    v2->coord.y < -0.5 || v2->coord.y > zbuffer->height-0.5)
	    continue;

	vzi0.x = v0->coord.x;
	vzi0.y = v0->coord.y;
	vzi0.Z = v0->coord.z;
	vzi0.I = v0->norm.z;

	vzi1.x = v1->coord.x;
	vzi1.y = v1->coord.y;
	vzi1.Z = v1->coord.z;
	vzi1.I = v1->norm.z;

	vzi2.x = v2->coord.x;
	vzi2.y = v2->coord.y;
	vzi2.Z = v2->coord.z;
	vzi2.I = v2->norm.z;

	SL_SCTriangle_DDA(&vzi0, &vzi1, &vzi2, 
			  (IS_CAttr_None *) NULL, zbuffer);
  }
}

void
Clear(FB_IntensityBuffer *buffer, Real Z, Real I)
{
  for (int i = 0; i < buffer->width * buffer->height; i++) {
    buffer->sampleZI[i].Z = Z;
    buffer->sampleZI[i].I = I;
  }
}
