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
 * sl_framebuffer.H
 *
 */

#ifndef SL_FRAMEBUFFER_H
#define SL_FRAMEBUFFER_H

#include "sl_val.H"

typedef struct FB_SampleZ { Real Z; } FB_SampleZ;

typedef struct FB_SampleZI { Real Z, I; } FB_SampleZI;

typedef struct FB_SampleRGBA { Integer8 R, G, B, A; } FB_SampleRGBA;

typedef struct FB_SampleZRGBA {
  Real Z;
  Integer8 R, G, B, A;
} FB_SampleZRGBA;

typedef struct FB_SampleFloatRGBA { Real R, G, B, A; } FB_SampleFloatRGBA;

typedef struct FB_SampleFloatZRGBA {
  Real Z;
  Real R, G, B, A;
} FB_SampleFloatZRGBA;

typedef struct FB_UnifiedBuffer {
  FB_SampleZRGBA *sampleZRGBA;
  Integer16 height, width;
} FB_UnifiedBuffer;

typedef struct FB_IntensityBuffer {
  FB_SampleZI *sampleZI;
  Integer16 height, width;
} FB_IntensityBuffer;

typedef struct FB_SplitBuffer {
  FB_SampleZ *sampleZ;
  FB_SampleRGBA *sampleRGBA;
  Integer16 height, width;
} FB_SplitBuffer;


typedef struct FB_FloatUnifiedBuffer {
  FB_SampleFloatZRGBA *sampleFloatZRGBA;
  Integer16 height, width;
} FB_FloatUnifiedBuffer;

typedef struct FB_FloatSplitBuffer {
  FB_SampleZ *sampleZ;
  FB_SampleFloatRGBA *sampleFloatRGBA;
  Integer16 height, width;
} FB_FloatSplitBuffer;


#endif /* SL_FRAMEBUFFER_H */


