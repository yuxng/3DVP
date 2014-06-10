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
 * sl_texture.I
 *
 */


#ifndef SL_TEXTURE_I
#define SL_TEXTURE_I

#include "sl_texture.H"

static inline
void PointSample(const TextureMap &ps_tex, const Real S, const Real T,
		 Real &curR, Real &curG, Real &curB)
{
  Integer s = (Integer) S;
  Integer t = (Integer) T;
  
  IntegerPS toffset = s + (t << ps_tex.width_shift);
  toffset += (toffset << 1);
  
  TM_RGB *tsample = (TM_RGB *) ((char *) ps_tex.texture + toffset);

  curR *= (Real) tsample->R;
  curG *= (Real) tsample->G;
  curB *= (Real) tsample->B;
}






#define LERP(a, b, f) ((a) + (f) * ((b) - (a)))

#define TriLERP(a) \
l_lo = LERP(ll_lo->a, lr_lo->a, sfrac); \
u_lo = LERP(ul_lo->a, ur_lo->a, sfrac); \
l_hi = LERP(ll_hi->a, lr_hi->a, sfrac); \
u_hi = LERP(ul_hi->a, ur_hi->a, sfrac); \
\
__lo = LERP(l_lo, u_lo, tfrac); \
__hi = LERP(l_hi, u_hi, tfrac); \
\
cur##a *= LERP(__lo, __hi, levelfrac);


#define BiLERP(a) \
lower = LERP(ll->a, lr->a, sfrac); \
upper = LERP(ul->a, ur->a, sfrac); \
cur##a *= LERP(lower, upper, tfrac);

static inline
void MipMapSample(const MipMap &mipmap,
		  const Real S, const Real T, const Real D,
		  Real &curR, Real &curG, Real &curB)
{
  Integer IntD = *(Integer *) &D;

  Integer level = (IntD >> REAL_MANTISSA_BITS) - REAL_EXPONENT_BIAS;

  if (level >= mipmap.levels) {
    curR *= (Real) (mipmap.texture[mipmap.levels]->R);
    curG *= (Real) (mipmap.texture[mipmap.levels]->G);
    curB *= (Real) (mipmap.texture[mipmap.levels]->B);
    return;
  }


  Integer s = (Integer) S;
  Integer t = (Integer) T;
  
  Real sfrac = S - (Real) s;
  Real tfrac = T - (Real) t;
  
  if (level < 0) {
    Integer size = 1 << mipmap.levels;
    IntegerPS offset = s + (t << mipmap.levels);
    size += (size << 1);

    TM_RGB *ll = (TM_RGB *) ((char *) mipmap.texture[0] + offset);
    TM_RGB *lr = ll + 1;
    TM_RGB *ul = (TM_RGB *) ((char *) ll + size);
    TM_RGB *ur = ul + 1;

    Real lower, upper;

    BiLERP(R);
    BiLERP(G);
    BiLERP(B);
    
    return;
  }
  
  Real levelfrac = ((Real) REAL_POW_MANTISSA_BITS *
		    (Real) (IntD & REAL_MANTISSA_MASK));
  
  s = s >> level;
  t = t >> level;
  
  Integer sizelog2 = mipmap.levels - level;
  Integer size = 1 << sizelog2;
  
  IntegerPS offset_lo = s + (t << sizelog2);
  offset_lo += (offset_lo << 1);

  size += (size << 1);

  TM_RGB *ll_lo = (TM_RGB *) ((char *) mipmap.texture[level] + offset_lo);
  TM_RGB *lr_lo = ll_lo + 1;
  TM_RGB *ul_lo = (TM_RGB *) ((char *) ll_lo + size);
  TM_RGB *ur_lo = ul_lo + 1;
  

  IntegerPS offset_hi = (s >> 1) + (t << (sizelog2 - 1));
  offset_hi += (offset_hi << 1);

  TM_RGB *ll_hi = (TM_RGB *) ((char *) mipmap.texture[level + 1] + offset_hi);
  TM_RGB *lr_hi = ll_hi + 1;
  TM_RGB *ul_hi = (TM_RGB *) ((char *) ll_hi + size);
  TM_RGB *ur_hi = ul_hi + 1;
  
  Real l_lo, u_lo, l_hi, u_hi, __lo, __hi;
  
  TriLERP(R);
  TriLERP(G);
  TriLERP(B);
  
}

	     


#ifdef checkerboard

    if (((int) S % 2) ^ ((int) T % 2)) {
      FB_R(sample) = FB_InterpToSample(get(R));
#ifdef LevelOfDetail
      FB_G(sample) = (char) (100.0 * D);
#else
      FB_G(sample) = FB_InterpToSample(get(G));
#endif
      FB_B(sample) = FB_InterpToSample(get(B));
    }
    else {
      FB_R(sample) = (char) FB_InterpToSample(get(R)) >> 2;
#ifdef LevelOfDetail
      FB_G(sample) = (char) (100.0 * D);
#else
      FB_G(sample) = (char) FB_InterpToSample(get(G)) >> 2;
#endif
      FB_B(sample) = (char) FB_InterpToSample(get(B)) >> 2;
    }

#endif



#endif /* TEXTURE_I */

