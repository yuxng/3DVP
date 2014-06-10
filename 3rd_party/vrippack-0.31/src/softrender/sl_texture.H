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
 * sl_texture.H
 *
 */


#ifndef SL_TEXTURE_H
#define SL_TEXTURE_H

#include "sl_val.H"

typedef struct TM_RGB { Integer8 R, G, B; } TM_RGB;

typedef struct TextureMap {
  TM_RGB *texture;
  Integer height, width;
  Integer height_shift, width_shift;
} TextureMap;


#define MIPMAP_MAX_LEVELS 12

typedef struct MipMap {
  TM_RGB *texture[MIPMAP_MAX_LEVELS];
  Integer levels;
} MipMap;




#endif /* SL_TEXTURE_H */
