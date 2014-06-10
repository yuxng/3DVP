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
 * sl_vertex.H
 *
 */


#ifndef SL_VERTEX_H
#define SL_VERTEX_H

#include "sl_val.H"
#include "sl_texture.H"

typedef struct IS_Vertex_None {
  Real x, y;
} IS_Vertex_None;

typedef struct IS_Vertex_Z {
  Real x, y;
  Real Z;
} IS_Vertex_Z;

typedef struct IS_Vertex_ZI {
  Real x, y;
  Real Z, I;
} IS_Vertex_ZI;

typedef struct IS_Vertex_ZCA {
  Real x, y;
  Real Z, C, A;
} IS_Vertex_ZCA;

typedef struct IS_Vertex_ZRGB {
  Real x, y;
  Real Z, R, G, B;
} IS_Vertex_ZRGB;


typedef struct IS_CAttr_C {
  Integer32 C;
} IS_CAttr_C;

typedef struct IS_CAttr_A {
  Real A;
} IS_CAttr_A;

typedef struct IS_CAttr_T {
  TextureMap T;
} IS_CAttr_T;

typedef struct IS_CAttr_None {
} IS_CAttr_None;


typedef struct IS_Vertex_ZRGBA {
  Real x, y;
  Real Z, R, G, B, A;
} IS_Vertex_ZRGBA;

typedef struct IS_Vertex_ZCAUVW {
  Real x, y;
  Real Z, C, A, U, V, W;
} IS_Vertex_ZCAUVW;

typedef struct IS_Vertex_ZRGBUVW {
  Real x, y;
  Real Z, R, G, B, U, V, W;
} IS_Vertex_ZRGBUVW;

typedef struct IS_Vertex_ZRGBAUVW {
  Real x, y;
  Real Z, R, G, B, A, U, V, W;
} IS_Vertex_ZRGBAUVW;

#endif /* VERTEX_H */
