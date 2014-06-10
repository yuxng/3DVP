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
 * sl_common.I
 *
 */

#ifndef SL_COMMON_I
#define SL_COMMON_I

//#include <iostream.h>
#include <assert.h>

#include "sl_val.H"
#include "sl_vertex.H"
#include "sl_texture.H"
#include "sl_hack.H"
#include "sl_common.H"
#include "sl_framebuffer.H"
#include "sl_framebuffer.I"



/////////////////////////////////////////////////////
//  routines for scan converting given structures  //
/////////////////////////////////////////////////////

#define PartialIncrement(i) { IN_Index(cur, i) += xf * IN_Index(ddx, i); }
#define IncrementX(i) { IN_Index(cur, i) += IN_Index(ddx, i); }
#define DecrementX(i) { IN_Index(cur, i) -= IN_Index(ddx, i); }

#define SetSample(i) \
FB_Index(FB_Sample, i) FB_Index(*sample, i) = \
                  FB_Index(buffer->sample, i) + begin;

#define LastSample(i) \
FB_Index(FB_Sample, i) *last_sample = FB_Index(buffer->sample, i) + end;

#define IncrementSample(i) FB_Index(sample, i)++
#define DecrementSample(i) FB_Index(sample, i)--



static inline void
SL_ScanXLeftToRight(Real lx, Real rx,
#ifdef SampleCalc_Dda
		    IN_FormalArgs(cur)
#endif
#ifdef SampleCalc_Ddx
		    IN_FormalConstArgs(cur)
#endif
		    IN_FormalConstArgs(ddx)
		    CA_FormalConstArgs(cur)
#ifdef LevelOfDetail
		    const Real numParX, Real *numParYArray,
#endif
		    const FB_Buffer *buffer, const Integer offset)
{

  Integer xr = CeilR2I(rx);
  Integer xl = CeilR2I(lx);

  if (xl >= xr)
    return;

  Real xf = (Real) xl - lx;

#ifdef SampleCalc_Dda
  IN_DoForEach(PartialIncrement);
#endif

  Integer begin = offset + xl;
  Integer end = offset + xr - 1;

#ifdef LevelOfDetail
  numParYArray += xl;
#endif

  FB_DoForEach(SetSample);
  FB_DoForFirst(LastSample);

  while (FB_FirstIndex(sample) != last_sample) {

#ifdef SampleCalc_Dda
    BufferSample(IN_ListArgs(cur)
		 CA_ListArgs(cur)
#ifdef LevelOfDetail
		 numParX, *numParYArray,
#endif
		 FB_ListArgs(sample));
    IN_DoForEach(IncrementX);
#endif

#ifdef SampleCalc_Ddx
    BufferSample(xf,
		 IN_ListArgs(cur)
		 IN_ListArgs(ddx)
		 CA_ListArgs(cur)
#ifdef LevelOfDetail
		 numParX, *numParYArray,
#endif
		 FB_ListArgs(sample));
    xf += REAL_ONE;
#endif

#ifdef LevelOfDetail
    numParYArray++;
#endif

    FB_DoForEach(IncrementSample);
  }

#ifdef SampleCalc_Dda
  BufferSample(IN_ListArgs(cur)
	       CA_ListArgs(cur)
#ifdef LevelOfDetail
	       numParX, *numParYArray,
#endif
	       FB_ListArgs(sample));
#endif

#ifdef SampleCalc_Ddx
  BufferSample(xf,
	       IN_ListArgs(cur)
	       IN_ListArgs(ddx)
	       CA_ListArgs(cur)
#ifdef LevelOfDetail
	       numParX, *numParYArray,
#endif
	       FB_ListArgs(sample));
#endif

}





static inline void
SL_ScanXRightToLeft(Real lx, Real rx,
#ifdef SampleCalc_Dda
		    IN_FormalArgs(cur)
#endif
#ifdef SampleCalc_Ddx
		    IN_FormalConstArgs(cur)
#endif
		    IN_FormalArgs(ddx)
		    CA_FormalArgs(cur)
#ifdef LevelOfDetail
		    const Real numParX, Real *numParYArray,
#endif
		    const FB_Buffer *buffer, Integer offset)
{
  Integer xl = CeilR2I(lx);
  Integer xr = CeilR2I(rx);

  if (xl >= xr)
    return;

  Real xf = (Real) (xr - 1) - rx;

#ifdef SampleCalc_Dda
  IN_DoForEach(PartialIncrement);
#endif

  Integer begin = offset + (xr - 1);
  Integer end = offset + xl;

#ifdef LevelOfDetail
  numParYArray += (xr - 1);
#endif

  FB_DoForEach(SetSample);
  FB_DoForFirst(LastSample);

  while (FB_FirstIndex(sample) != last_sample) {

#ifdef SampleCalc_Dda
    BufferSample(IN_ListArgs(cur)
		 CA_ListArgs(cur)
#ifdef LevelOfDetail
		 numParX, *numParYArray,
#endif
		 FB_ListArgs(sample));
    IN_DoForEach(DecrementX);
#endif

#ifdef SampleCalc_Ddx
    BufferSample(xf,
		 IN_ListArgs(cur)
		 IN_ListArgs(ddx)
		 CA_ListArgs(cur)
#ifdef LevelOfDetail
		 numParX, *numParYArray,
#endif
		 FB_ListArgs(sample));
    xf -= REAL_ONE;
#endif

#ifdef LevelOfDetail
  numParYArray--;
#endif

    FB_DoForEach(DecrementSample);
  }


#ifdef SampleCalc_Dda
  BufferSample(IN_ListArgs(cur)
	       CA_ListArgs(cur)
#ifdef LevelOfDetail
	       numParX, *numParYArray,
#endif
	       FB_ListArgs(sample));
#endif

#ifdef SampleCalc_Ddx
  BufferSample(xf,
	       IN_ListArgs(cur)
	       IN_ListArgs(ddx)
	       CA_ListArgs(cur)
#ifdef LevelOfDetail
	       numParX, *numParYArray,
#endif
	       FB_ListArgs(sample));
#endif


}



#endif /* SL_COMMON_I */





