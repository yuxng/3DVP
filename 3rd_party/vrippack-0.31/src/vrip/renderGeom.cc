#include <gl/gl.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/types.h>
#include <math.h>

#include "plyio.h"
#include "DepthMap.h"
#include "OccGrid.h"
#include "renderGeom.h"
#include "defines.h"
#include "Matrix4f.h"
#include "occ.h"
#include "occGlobals.h"

#include "sl_export.H"
#include "vertex.H"
#include "texture.H"
#include "framebuffer.H"

static clock_t tm;
static void start_time();
static void end_time();
static float time_elapsed();
void Clear(FB_IntensityBuffer *buffer, Real Z, Real I);

const int SCREEN_XDIM = 1280;
const int SCREEN_YDIM = 1024;

void readDepthMap(DepthMap *dm, BBox3f *bbox);
void drawMesh(Mesh *mesh);
void setEmission(float r, float g, float b);

static DeepZ;

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


DepthMap *
renderDepthMap(Mesh *mesh, OrthoShear *shear, BBox3f *bbox, 
	       float resolution, DepthMap *dm) 
{
    int xdim, ydim, xorg, yorg;
    long winID;
    float new_xnur, new_ynur;

    xdim = int(ceil((bbox->nur.x - bbox->fll.x)/resolution) + 1);
    ydim = int(ceil((bbox->nur.y - bbox->fll.y)/resolution) + 1);

    new_xnur = (xdim-1)*resolution + bbox->fll.x;
    new_ynur = (ydim-1)*resolution + bbox->fll.y;

    xorg = SCREEN_XDIM/2 - xdim/2;
    yorg = SCREEN_YDIM/2 - ydim/2;

    if (xorg < 0 || yorg < 0) {
	fprintf(stderr, "Depth map is too large.\n");
	exit(1);
    }

    if (dm == NULL)
	dm = new DepthMap(xdim, ydim, FALSE);
    else
	dm->reuse(xdim, ydim);

    dm->origin[0] = bbox->fll[0];
    dm->origin[1] = bbox->fll[1];
    dm->resolution = resolution;

    prefposition(xorg, xorg+xdim-1, yorg, yorg+ydim-1);
    foreground();
    winID = winopen("depth map");


/*    float squareData[4][3] = {
	{0.5, -0.5, 0.0},
	{0.5, 0.5, 0.0},
	{-0.5, -0.5, 0.0},
	{-0.5, 0.5, 0.0}
    };
*/


    singlebuffer();
    readsource(SRC_ZBUFFER);
    drawmode(NORMALDRAW);
    RGBmode();
    gconfig();

    long zbits = getgdesc(GD_BITS_NORM_ZBUFFER);
    DeepZ = zbits > 24;

    mmode(MVIEWING);

    float light_model[] = {
	LMNULL
    };

    lmdef (DEFLMODEL, 1, 0, light_model);
    lmbind (LMODEL, 1);

    ortho(-1.0, 1.0, -1.0, 1.0, 0.15, -0.15);
    ortho(bbox->fll[0], new_xnur, bbox->fll[1], new_ynur, 
	  -bbox->nur[2], -bbox->fll[2]);
    if (Verbose)
	printf("BBox: (%f, %f, %f) -> (%f, %f, %f)\n", 
	       bbox->fll[0], bbox->fll[1], bbox->fll[2], 
	       new_xnur, new_ynur, bbox->nur[2]);

    cpack(0x0);
    clear();
    zclear();
    zbuffer(TRUE);


    mmode(MVIEWING);

    Matrix4f mrot;
    mesh->quat.toMatrix(mrot);

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

    Matrix4f mfinal;
    mfinal.makeIdentity();
    mfinal.multLeft(mrot);
    mfinal.multLeft(mtrans);
    mfinal.multLeft(maxes);
    mfinal.multLeft(mflipz);
    mfinal.multLeft(msh);
    mfinal.transpose();
    
    loadmatrix(mfinal.m);

    static float material[] = {
	EMISSION, 1, 1, 1,
	DIFFUSE, 0,0,0,
	AMBIENT, 0,0,0,
	SPECULAR, 0,0,0,
	LMNULL
    };

    lmdef(DEFMATERIAL, 1, 0, material);
    lmbind(MATERIAL, 1);


    start_time();
    drawMesh(mesh);

    readDepthMap(dm, bbox);
    end_time();

    if (Verbose)
	printf("Time to render and read frame buffer = %f.\n", 
	       time_elapsed());

    return dm;
}


void
drawMesh(Mesh *mesh)
{
/*    
float squareData[4][3] = {
	{0.05, 0.10, 0.00},
	{0.05, 0.20, 0.00},
	{-0.05, 0.10, 0.00},
	{-0.05, 0.20, 0.00}
    };
*/

    float vec[3];
    float normal[3] = {0,0,1};
    
    Vertex *vertList = mesh->verts;
    Vertex *vert;
    Triangle *tri;
    float intensity;
    float confidence;

    // Everything faces forward
    n3f(normal);

    for (int i = 0; i < mesh->numTris; i++) {
	bgnpolygon();

	tri = &mesh->tris[i];

	vert = &vertList[tri->vindex1];
	confidence = vert->confidence;
	setEmission(confidence, confidence, confidence);
	vec[0] = vert->coord.x;
	vec[1] = vert->coord.y;
	vec[2] = vert->coord.z;
	v3f(vec); 

	vert = &vertList[tri->vindex2];
	confidence = vert->confidence;
	setEmission(confidence, confidence, confidence);
	vec[0] = vert->coord.x;
	vec[1] = vert->coord.y;
	vec[2] = vert->coord.z;
	v3f(vec); 

	vert = &vertList[tri->vindex3];
	confidence = vert->confidence;
	setEmission(confidence, confidence, confidence);
	vec[0] = vert->coord.x;
	vec[1] = vert->coord.y;
	vec[2] = vert->coord.z;
	v3f(vec); 

	endpolygon();
    }
}


void
setEmission(float r, float g, float b)
{
    static float material[] = {
	EMISSION, 1, 1, 1,
	DIFFUSE, 0,0,0,
	AMBIENT, 0,0,0,
	SPECULAR, 0,0,0,
	LMNULL
    };

    material[1] = r;
    material[2] = g;
    material[3] = b;

    lmdef(DEFMATERIAL, 1, 0, material);
    lmbind(MATERIAL, 1);
}



void
readDepthMap(DepthMap *dm, BBox3f *bbox) 
{
    float zscale, ztrans;
    DepthElement *buf;

    ulong *zarray = new ulong[dm->xdim*dm->ydim];
    ulong *rgbarray = new ulong[dm->xdim*dm->ydim];

    readsource(SRC_ZBUFFER);
    lrectread(0, 0, dm->xdim-1, dm->ydim-1, zarray);

    readsource(SRC_FRONT);
    lrectread(0, 0, dm->xdim-1, dm->ydim-1, rgbarray);

    zscale = -2/(bbox->nur[2] - bbox->fll[2]);
    ztrans = (bbox->nur[2] + bbox->fll[2])/(bbox->nur[2] - bbox->fll[2]);

    buf = dm->elems;

    if (DeepZ) {
	for (int i = 0; i < dm->xdim*dm->ydim; i++, buf++) {
	    long zlong = zarray[i];
	    if (zlong&0x80000000) {
		// zlong = -(zlong^0x00ffffff +1) doesn't work!
		zlong = (zlong^0xffffffff);
		zlong = -(zlong+1);
	    }
	    if (zlong > 8388600*256) {
		buf->z = FAR_AWAY_DEPTH;
		buf->conf = 0.0;
	    } else {
		buf->z = (zlong/8388607.0/256 - ztrans)/zscale;
		buf->conf = (rgbarray[i]&0xff)/255.0;
	    }
	    //printf("%d ", zlong);
	}
    }
    else {
	for (int i = 0; i < dm->xdim*dm->ydim; i++, buf++) {
	    long zlong = zarray[i];
	    if (zlong&0x00800000) {
		// zlong = -(zlong^0x00ffffff +1) doesn't work!
		zlong = (zlong^0x00ffffff);
		zlong = -(zlong+1);
	    }
	    if (zlong > 8388600) {
		buf->z = FAR_AWAY_DEPTH;
		buf->conf = 0.0;
	    } else {
		buf->z = (zlong/8388607.0 - ztrans)/zscale;
		buf->conf = (rgbarray[i]&0xff)/255.0;
	    }
	    //printf("%d ", zlong);
	}
    }

    delete [] zarray;
    delete [] rgbarray;
}

