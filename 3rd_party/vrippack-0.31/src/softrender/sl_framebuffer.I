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
 * sl_framebuffer.I
 *
 */

#ifndef SL_FRAMEBUFFER_I
#define SL_FRAMEBUFFER_I

#include "sl_val.H"
#include "sl_framebuffer.H"
#include "sl_hack.H"
#include "sl_texture.H"
#include "sl_texture.I"


#include <iostream>
#include <stdio.h>

#ifdef SampleCalc_Dda
#define get(i) (cur##i)
#endif

#ifdef SampleCalc_Ddx
#define get(i) (cur##i + (ddx##i * curx))
#endif


static inline void
BufferSample(
#ifdef SampleCalc_Ddx
	     const Real curx,
#endif
	     IN_FormalArgs(cur)
#ifdef SampleCalc_Ddx
	     IN_FormalConstArgs(ddx)
#endif
	     CA_FormalConstArgs(cur)
#ifdef LevelOfDetail
	     Real numParX, Real numParY,
#endif
	     FB_FormalArgs(* sample)
	     )
{

#ifdef ZBuffer
  if (get(Z) > FB_Z(sample)) {
    FB_Z(sample) = get(Z);
#endif


#ifdef Interpolate_Color
    Real R = get(R);
    Real G = get(G);
    Real B = get(B);
#endif

#ifdef TextureCoord
    Real inv_w = REAL_ONE / get(W);
    Real S = get(U) * inv_w;
    Real T = get(V) * inv_w;
#endif


#ifdef LevelOfDetail
    Real D = ((numParX > numParY) ? numParX : numParY) * inv_w * inv_w;
    G = D;
#endif


#ifdef PointSampleTexture
    PointSample(curT, S, T, R, G, B);
#endif

#ifdef Interpolate_Color

#ifndef Alpha

    FB_R(sample) = FB_InterpToSample(R);
    FB_G(sample) = FB_InterpToSample(G);
    FB_B(sample) = FB_InterpToSample(B);

#else

#ifdef Constant_A
    Real alpha = curA;
#else
    Real alpha = get(A);
#endif


    FB_R(sample) = red   + alpha * (FB_SampleToInterp(FB_R(sample)) - red  );
    FB_G(sample) = green + alpha * (FB_SampleToInterp(FB_G(sample)) - green);
    FB_B(sample) = blue  + alpha * (FB_SampleToInterp(FB_B(sample)) - blue );
    FB_A(sample) = alpha + alpha * (FB_SampleToInterp(FB_R(sample)) - alpha);


#endif /* Alpha */

#endif /* Interpolate_Color */

#ifdef Interpolate_Intensity
    FB_I(sample) = FB_InterpToSample(get(I));
#endif


#ifdef Constant_PackedColor
    FB_C(sample) = curC;
#endif


#ifdef ZBuffer
  }
#endif


}




#endif /* FRAMEBUFFER_I */
