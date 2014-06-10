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
 * sl_hack.H
 *
 */

#ifndef SL_HACK_H
#define SL_HACK_H




#ifdef Constant_None

#define IS_CAttr IS_CAttr_None

#define CA_ListArgs(a)         
#define CA_FormalArgs(a)       
#define CA_FormalConstArgs(a)
#define CA_DeclareArgs(a)      

#endif




#ifdef Constant_C

#define Constant_PackedColor

#define IS_CAttr IS_CAttr_C

#define CA_ListArgs(a)         a##C,
#define CA_FormalArgs(a)       Integer a##C,
#define CA_FormalConstArgs(a)  const Integer a##C,
#define CA_DeclareArgs(a)      Integer a##C;


#endif



#ifdef Constant_A

#define Alpha

#define IS_CAttr IS_CAttr_A

#define CA_ListArgs(a)         a##A,
#define CA_FormalArgs(a)       Real a##A,
#define CA_FormalConstArgs(a)  const Real a##A,
#define CA_DeclareArgs(a)      Real a##A;


#endif




#ifdef Constant_T

#define PointSampleTexture
#define LevelOfDetail

#define IS_CAttr IS_CAttr_T

#define CA_ListArgs(a)         a##T,
#define CA_FormalArgs(a)       TextureMap a##T,
#define CA_FormalConstArgs(a)  const TextureMap a##T,
#define CA_DeclareArgs(a)      TextureMap a##T;


#endif






#ifdef Interpolate_None

#define IS_Vertex IS_Vertex_None

#define SL_FullEdge  SL_FullEdge_None
#define SL_SlopeEdge SL_SlopeEdge_None
#define SL_MinEdge   SL_MinEdge_None

#define IN_Index(a,i)          a##i
#define IN_IndexP(a,i)         a->i
#define IN_DoForEach(f)       
#define IN_ListArgs(a)        
#define IN_FormalConstArgs(a)
#define IN_FormalRefArgs(a)
#define IN_FormalArgs(a)      
#define IN_DeclareArgs(a)     

#define IN_DeclareArgs(a)  

#endif





#ifdef Interpolate_Z

#define ZBuffer

#define IS_Vertex IS_Vertex_Z

#define SL_FullEdge  SL_FullEdge_Z
#define SL_SlopeEdge SL_SlopeEdge_Z
#define SL_MinEdge   SL_MinEdge_Z

#define IN_Index(a,i)          a##i
#define IN_IndexP(a,i)         a->i
#define IN_DoForEach(f)        f(Z);
#define IN_ListArgs(a)         a##Z,
#define IN_FormalArgs(a)       Real a##Z,
#define IN_FormalConstArgs(a)  const Real a##Z,
#define IN_FormalRefArgs(a)    Real &a##Z,
#define IN_DeclareArgs(a)      Real a##Z;

#endif




#ifdef Interpolate_ZI

#define ZBuffer
#define Interpolate_Intensity

#define IS_Vertex IS_Vertex_ZI

#define IN_Index(a,i)          a##i
#define IN_IndexP(a,i)         a->i
#define IN_DoForEach(f)        f(Z); f(I);
#define IN_ListArgs(a)         a##Z, a##I,
#define IN_FormalArgs(a)       Real a##Z, Real a##I,
#define IN_FormalConstArgs(a)  const Real a##Z, const Real a##I,
#define IN_FormalRefArgs(a)    Real &a##Z, Real &a##I,
#define IN_DeclareArgs(a)      Real a##Z, a##I;

#endif




#ifdef Interpolate_ZRGB

#define ZBuffer
#define Interpolate_Color

#define IS_Vertex IS_Vertex_ZRGB

#define SL_FullEdge  SL_FullEdge_ZRGB
#define SL_SlopeEdge SL_SlopeEdge_ZRGB
#define SL_MinEdge   SL_MinEdge_ZRGB

#define IN_Index(a,i)          a##i
#define IN_IndexP(a,i)         a->i
#define IN_DoForEach(f)        f(Z) ; f(R); f(G); f(B);
#define IN_ListArgs(a)         a##Z, a##R, a##G, a##B,
#define IN_FormalArgs(a)       Real a##Z, Real a##R, Real a##G, Real a##B,
#define IN_FormalConstArgs(a)  const Real a##Z, const Real a##R,\
                               const Real a##G, const Real a##B,
#define IN_FormalRefArgs(a)    Real &a##Z, Real &a##R, Real &a##G, Real &a##B,
#define IN_DeclareArgs(a)      Real a##Z, a##R, a##G, a##B;


#endif






#ifdef Interpolate_ZRGBUVW

#define ZBuffer
#define Interpolate_Color
#define TextureCoord

#define IS_Vertex IS_Vertex_ZRGBUVW

#define SL_FullEdge  SL_FullEdge_ZRGBUVW
#define SL_SlopeEdge SL_SlopeEdge_ZRGBUVW
#define SL_MinEdge   SL_MinEdge_ZRGBUVW

#define IN_Index(a,i)          a##i
#define IN_DoForEach(f)        f(Z); f(R); f(G); f(B); \
                               f(U); f(V); f(W);
#define IN_ListArgs(a)         a##Z, a##R, a##G, a##B,  a##U, a##V, a##W,
#define IN_FormalArgs(a)       Real a##Z, Real a##R, Real a##G, Real a##B, \
                               Real a##U, Real a##V, Real a##W,
#define IN_FormalConstArgs(a)  const Real a##Z, const Real a##R,\
                               const Real a##G, const Real a##B,\
                               const Real a##U,\
                               const Real a##V, const Real a##W,
#define IN_FormalRefArgs(a)    Real &a##Z, Real &a##R, Real &a##G, Real &a##B,\
                               Real &a##U, Real &a##V, Real &a##W,
#define IN_DeclareArgs(a)      Real a##Z, a##R, a##G, a##B, \
                                    a##U, a##V, a##W;


#endif








#ifdef FrameBuffer_Split

#define FB_Buffer FB_SplitBuffer

#define FB_Index(a, i) a##i
#define FB_FirstIndex(a) a##Z
#define FB_DoForFirst(f) f(Z)
#define FB_DoForEach(f)  f(Z); f(RGBA);

#define FB_ListArgs(a)   a##Z, a##RGBA
#define FB_FormalArgs(a) FB_SampleZ a##Z, FB_SampleRGBA a##RGBA

#define FB_Z(a)          a##Z->Z
#define FB_R(a)          a##RGBA->R
#define FB_G(a)          a##RGBA->G
#define FB_B(a)          a##RGBA->B
#define FB_A(a)          a##RGBA->A
#define FB_C(a)       *((Integer *) &(a##RGBA->R))

#define FB_InterpToSample(x) ((Integer8) ((Integer) (x)))
#define FB_SampleToInterp(x) ((Real) (x))

#endif


#ifdef FrameBuffer_Unified

#define FB_Buffer FB_UnifiedBuffer

#define FB_Index(a, i) a##i
#define FB_FirstIndex(a) a##ZRGBA
#define FB_DoForFirst(f) f(ZRGBA);
#define FB_DoForEach(f)  f(ZRGBA);

#define FB_ListArgs(a)   a##ZRGBA
#define FB_FormalArgs(a) FB_SampleZRGBA a##ZRGBA

#define FB_Z(a)          a##ZRGBA->Z
#define FB_R(a)          a##ZRGBA->R
#define FB_G(a)          a##ZRGBA->G
#define FB_B(a)          a##ZRGBA->B
#define FB_A(a)          a##ZRGBA->A
#define FB_C(a)       *((Integer *) &(a##ZRGBA->R))

#define FB_InterpToSample(x) ((Integer8) ((Integer) (x)))
#define FB_SampleToInterp(x) ((Real) (x))

#endif





#ifdef FrameBuffer_Intensity

#define FB_Buffer FB_IntensityBuffer

#define FB_Index(a, i)   a##i
#define FB_FirstIndex(a) a##ZI
#define FB_DoForFirst(f) f(ZI);
#define FB_DoForEach(f)  f(ZI);

#define FB_ListArgs(a)   a##ZI
#define FB_FormalArgs(a) FB_SampleZI a##ZI

#define FB_Z(a)          a##ZI->Z
#define FB_I(a)          a##ZI->I

#define FB_InterpToSample(x) ((Real) (x))
#define FB_SampleToInterp(x) ((Real) (x))

#endif





#ifdef FrameBuffer_FloatSplit

#define FB_Buffer FB_FloatSplitBuffer

#define FB_Index(a, i) a##i
#define FB_FirstIndex(a) a##Z
#define FB_DoForFirst(f) f(Z);
#define FB_DoForEach(f)  f(Z); f(FloatRGBA);

#define FB_ListArgs(a)   a##Z, a##FloatRGBA
#define FB_FormalArgs(a) FB_SampleZ a##Z, FB_SampleFloatRGBA a##FloatRGBA

#define FB_Z(a)          a##Z->Z
#define FB_R(a)          a##FloatRGBA->R
#define FB_G(a)          a##FloatRGBA->G
#define FB_B(a)          a##FloatRGBA->B
#define FB_A(a)          a##FloatRGBA->A
#define FB_C(a)       *((Integer *) &(a##FloatRGBA->R))

#define FB_InterpToSample(x) (x)
#define FB_SampleToInterp(x) (x)

#endif


#ifdef FrameBuffer_FloatUnified

#define FB_Buffer FB_FloatUnifiedBuffer

#define FB_Index(a, i) a##i
#define FB_FirstIndex(a) a##FloatZRGBA
#define FB_DoForFirst(f) f(FloatZRGBA);
#define FB_DoForEach(f)  f(FloatZRGBA);

#define FB_ListArgs(a)   a##FloatZRGBA
#define FB_FormalArgs(a) FB_SampleFloatZRGBA a##FloatZRGBA

#define FB_Z(a)          a##FloatZRGBA->Z
#define FB_R(a)          a##FloatZRGBA->R
#define FB_G(a)          a##FloatZRGBA->G
#define FB_B(a)          a##FloatZRGBA->B
#define FB_A(a)          a##FloatZRGBA->A
#define FB_C(a)       *((Integer *) &(a##FloatZRGBA->R))

#define FB_InterpToSample(x) (x)
#define FB_SampleToInterp(x) (x)

#endif




#endif /* SL_HACK_H */
