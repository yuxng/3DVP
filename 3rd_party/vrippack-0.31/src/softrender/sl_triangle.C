/*

Homan Igehy

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


/*
 * sl_triangle.C
 *
 */

#ifndef SL_TRIANGLE_C
#define SL_TRIANGLE_C

#include <assert.h>
#include <iostream>

#include "sl_vertex.H"
#include "sl_texture.H"
#include "sl_export.H"
#include "sl_hack.H"
#include "sl_common.H"


#define SL_REGULAR   0
#define SL_DEGENERATE   1
#define SL_TOP_HORIZONTAL  2
#define SL_BOT_HORIZONTAL  4


typedef struct SL_Triangle {
  Real fe_curx, fe_dxdy;
  Real se_curx, se_dxdy;
  Real me_curx, me_dxdy;
  IN_DeclareArgs(cur)
  IN_DeclareArgs(ddy)
  IN_DeclareArgs(ddx)
  Integer y_lo, y_mid, y_hi;
#ifdef LevelOfDetail
  Integer x_lo, x_mid, x_hi;
#endif
} SL_Triangle;



#define VSWAP(a, b) { IS_Vertex *tmp = a; a = b; b = tmp; }
#define ISWAP(a, b) { Integer tmp = a; a = b; b = tmp; }


#define SetDdyCurDdx(i) \
    lo = IN_IndexP(v_lo, i); \
    ddy = (IN_IndexP(v_hi, i) - lo) * inv_ydiff_full; \
    mid = IN_IndexP(v_mid, i); \
    IN_Index(tri->ddx, i) = (((mid - lo) * inv_ydiff_slope) - ddy)*inv_xdiff; \
    IN_Index(tri->cur, i) = lo + ddy * yfrac_lo; \
    IN_Index(tri->ddy, i) = ddy; \



#define SetDdyCurDdx_BotHor(i) \
    hi = IN_IndexP(v_hi, i); \
    lo = IN_IndexP(v_lo, i); \
    ddy = (hi - lo) * inv_ydiff_full; \
    mid = IN_IndexP(v_mid, i); \
    IN_Index(tri->ddx, i) = (((hi - mid) * inv_ydiff_slope) - ddy)*inv_xdiff; \
    IN_Index(tri->cur, i) = lo + ddy * yfrac_lo; \
    IN_Index(tri->ddy, i) = ddy; \


static inline Integer8
SL_3VertexTo3Edge(SL_Triangle *tri,
		  IS_Vertex *v_lo,
		  IS_Vertex *v_mid,
		  IS_Vertex *v_hi)
{

  Integer y_lo = CeilR2I(v_lo->y);
  Integer y_mid = CeilR2I(v_mid->y);
  Integer y_hi = CeilR2I(v_hi->y);

  if (y_lo > y_mid) {
    VSWAP(v_lo, v_mid);
    ISWAP(y_lo, y_mid);
  }

  if (y_mid > y_hi) {
    VSWAP(v_mid, v_hi);
    ISWAP(y_mid, y_hi);
  }

  if (y_lo > y_mid) {
    VSWAP(v_lo, v_mid);
    ISWAP(y_lo, y_mid);
  }

  tri->y_lo  = y_lo;
  tri->y_hi  = y_hi;
  tri->y_mid = y_mid;



#ifdef LevelOfDetail

  Integer x_lo = CeilR2I(v_lo->x);
  Integer x_mid = CeilR2I(v_mid->x);
  Integer x_hi = CeilR2I(v_hi->x);

  if (x_lo > x_mid) {
    ISWAP(x_lo, x_mid);
  }

  if (x_mid > x_hi) {
    ISWAP(x_mid, x_hi);
  }

  if (x_lo > x_mid) {
    ISWAP(x_lo, x_mid);
  }

  tri->x_lo  = x_lo;
  tri->x_hi  = x_hi;
  tri->x_mid = x_mid;

#endif


  int type;

  if (y_lo == y_hi) {
    return SL_DEGENERATE;
  }

  Real inv_ydiff_full  = REAL_ONE / (v_hi->y - v_lo->y);

  Real yfrac_lo =  (Real) y_lo - v_lo->y;
  Real yfrac_mid = (Real) y_mid - v_mid->y;

  if (y_lo == y_mid)
    type = SL_BOT_HORIZONTAL;
  else if (y_mid == y_hi)
    type = SL_TOP_HORIZONTAL;
  else
    type = SL_REGULAR;

  Real lo, mid, hi, ddy;

  lo = v_lo->x;
  ddy =  (v_hi->x - lo) * inv_ydiff_full;

  Real inv_ydiff_slope;
  if (type == SL_BOT_HORIZONTAL)
    inv_ydiff_slope = REAL_ONE / (v_hi->y - v_mid->y);
  else
    inv_ydiff_slope = REAL_ONE / (v_mid->y - v_lo->y);

  tri->fe_curx = lo + ddy * yfrac_lo;
  tri->fe_dxdy = ddy;


  if (type == SL_BOT_HORIZONTAL) {
    mid = v_mid->x;
    ddy =  (v_hi->x - mid) * inv_ydiff_slope;

    Real inv_xdiff = REAL_ONE / (ddy - tri->fe_dxdy);

    tri->se_curx = mid + ddy * yfrac_mid;
    tri->se_dxdy = ddy;

    IN_DoForEach(SetDdyCurDdx_BotHor);

  }
  else {
    lo = v_lo->x;
    ddy =  (v_mid->x - lo) * inv_ydiff_slope;

    Real inv_xdiff = REAL_ONE / (ddy - tri->fe_dxdy);

    tri->se_curx = lo + ddy * yfrac_lo;
    tri->se_dxdy = ddy;

    IN_DoForEach(SetDdyCurDdx);
  }




  if (type == SL_REGULAR) {
    mid = v_mid->x;
    ddy =  (v_hi->x - mid) / (v_hi->y - v_mid->y);
    tri->me_curx = mid + ddy * yfrac_mid;
    tri->me_dxdy = ddy;
  }


  

  return type;

}


#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef LevelOfDetail

static inline void
SL_InitializeNumPars(SL_Triangle *tri, Real *numParXArray, Real *numParYArray)
{
  Real dudy = tri->ddyU - tri->ddxU * tri->fe_dxdy;
  Real dvdy = tri->ddyV - tri->ddxV * tri->fe_dxdy;
  Real dwdy = tri->ddyW - tri->ddxW * tri->fe_dxdy;

  Real x_frac = ((Real) tri->x_lo) - tri->fe_curx;

  Real u_slope = tri->ddxU * dwdy - dudy * tri->ddxW;
  Real v_slope = tri->ddxV * dwdy - dvdy * tri->ddxW;

  Real uy_cur = (tri->curW * dudy - tri->curU * dwdy) - u_slope * x_frac;
  Real vy_cur = (tri->curW * dvdy - tri->curV * dwdy) - v_slope * x_frac;

  Real ux_cur = tri->curW * tri->ddxU - tri->curU * tri->ddxW;
  Real vx_cur = tri->curW * tri->ddxV - tri->curV * tri->ddxW;


  for (int x = tri->x_lo;
       x != tri->x_hi;
       x++, uy_cur -= u_slope, vy_cur -= v_slope)
    numParYArray[x] = MAX(Abs(uy_cur), Abs(vy_cur));

  for (int y = tri->y_lo;
       y != tri->y_hi;
       y++, ux_cur += u_slope, vx_cur += v_slope)
    numParXArray[y] = MAX(Abs(ux_cur), Abs(vx_cur));


}

#endif


#define CopyAll(i)    { IN_Index(cur, i) = IN_Index(tri->cur, i); \
		        IN_Index(ddy, i) = IN_Index(tri->ddy, i); \
		        IN_Index(ddx, i) = IN_Index(tri->ddx, i);   }

#define IncrementY(i) { IN_Index(cur, i) += IN_Index(ddy, i); }


static inline void
SL_ScanConvertLR(SL_Triangle *tri,
		 const IS_CAttr *cattr,
		 const FB_Buffer *buffer)
{
#ifdef LevelOfDetail

  Real numParXArray[MAX_SCREEN_DIMENSION];
  Real numParYArray[MAX_SCREEN_DIMENSION];

  SL_InitializeNumPars(tri, numParXArray, numParYArray);

#endif

  Integer cur_y = tri->y_lo;
  Integer width = buffer->width;
  Integer offset = cur_y * width;

#ifdef LevelOfDetail
  Real *numParXPtr = numParXArray + cur_y;
#endif

  Real lx = tri->fe_curx;
  Real dlx = tri->fe_dxdy;

  IN_DeclareArgs(cur);
  IN_DeclareArgs(ddy);
  IN_DeclareArgs(ddx);
  IN_DoForEach(CopyAll);

  Real rx = tri->se_curx;
  Real drx = tri->se_dxdy;

  while (cur_y != tri->y_mid) {
    cur_y++;
    SL_ScanXLeftToRight(lx, rx,
			IN_ListArgs(cur)
			IN_ListArgs(ddx)
			CA_ListArgs(cattr->)
#ifdef LevelOfDetail
			*numParXPtr, numParYArray,
#endif
			buffer, offset);
    rx += drx;
    lx += dlx;
    offset += width;
#ifdef LevelOfDetail
    numParXPtr++;
#endif
    IN_DoForEach(IncrementY);
  }

  rx = tri->me_curx;
  drx = tri->me_dxdy;

  while (cur_y != tri->y_hi) {
    cur_y++;
    SL_ScanXLeftToRight(lx, rx,
			IN_ListArgs(cur)
			IN_ListArgs(ddx)
			CA_ListArgs(cattr->)
#ifdef LevelOfDetail
			*numParXPtr, numParYArray,
#endif
			buffer, offset);
    rx += drx;
    lx += dlx;
    offset += width;
#ifdef LevelOfDetail
    numParXPtr++;
#endif
    IN_DoForEach(IncrementY);
  }
}


static inline void
SL_ScanConvertRL(SL_Triangle *tri,
		 const IS_CAttr *cattr,
		 const FB_Buffer *buffer)
{
#ifdef LevelOfDetail

  Real numParXArray[MAX_SCREEN_DIMENSION];
  Real numParYArray[MAX_SCREEN_DIMENSION];

  SL_InitializeNumPars(tri, numParXArray, numParYArray);

#endif

  Integer cur_y = tri->y_lo;
  Integer width = buffer->width;
  Integer offset = cur_y * width;

#ifdef LevelOfDetail
  Real *numParXPtr = numParXArray + cur_y;
#endif

  Real rx = tri->fe_curx;
  Real drx = tri->fe_dxdy;

  IN_DeclareArgs(cur);
  IN_DeclareArgs(ddy);
  IN_DeclareArgs(ddx);
  IN_DoForEach(CopyAll);

  Real lx = tri->se_curx;
  Real dlx = tri->se_dxdy;

  while (cur_y != tri->y_mid) {
    cur_y++;
    SL_ScanXRightToLeft(lx, rx,
			IN_ListArgs(cur)
			IN_ListArgs(ddx)
			CA_ListArgs(cattr->)
#ifdef LevelOfDetail
			*numParXPtr, numParYArray,
#endif
			buffer, offset);
    rx += drx;
    lx += dlx;
    offset += width;
#ifdef LevelOfDetail
    numParXPtr++;
#endif
    IN_DoForEach(IncrementY);
  }

  lx = tri->me_curx;
  dlx = tri->me_dxdy;

  while (cur_y != tri->y_hi) {
    cur_y++;
    SL_ScanXRightToLeft(lx, rx,
			IN_ListArgs(cur)
			IN_ListArgs(ddx)
			CA_ListArgs(cattr->)
#ifdef LevelOfDetail
			*numParXPtr, numParYArray,
#endif
			buffer, offset);
    rx += drx;
    lx += dlx;
    offset += width;
#ifdef LevelOfDetail
    numParXPtr++;
#endif
    IN_DoForEach(IncrementY);
  }
}



void
#ifdef SampleCalc_Dda
SL_SCTriangle_DDA
#endif
#ifdef SampleCalc_Ddx
SL_SCTriangle_DDX
#endif
		 (
		  IS_Vertex *v_lo,
		  IS_Vertex *v_mid,
		  IS_Vertex *v_hi,
		  IS_CAttr  *cattr,
		  FB_Buffer *buffer
		  )
{
  SL_Triangle tri;

  int type = SL_3VertexTo3Edge(&tri, v_lo, v_mid, v_hi);

  if (type == SL_DEGENERATE)
    return;

  if (type == SL_BOT_HORIZONTAL)
    tri.y_mid = tri.y_hi;

  if ((tri.fe_dxdy <= tri.se_dxdy) ^ (type == SL_BOT_HORIZONTAL)) {
    SL_ScanConvertLR(&tri, cattr, buffer);
  }
  else {
    SL_ScanConvertRL(&tri, cattr, buffer);
  }

}



#endif /* SL_TRIANGLE_C */
