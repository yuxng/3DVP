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
 * sl_common.H
 *
 */



#ifndef SL_COMMON_H
#define SL_COMMON_H



#include "sl_val.H"
#include "sl_vertex.H"
#include "sl_texture.H"
#include "sl_framebuffer.H"
#include "sl_hack.H"




/////////////////////////////////////////////////////
//  routines for scan converting given structures  //
/////////////////////////////////////////////////////

#ifdef jsasadj


static inline void
SL_ScanConvertRL(const SL_FullEdge &fe,
		 const SL_SlopeEdge &se,
		 const SL_MinEdge &me,
		 Integer cur_y, const Integer y_mid, const Integer y_hi,
		 const IN_Slope &inddx,
		 const IS_CAttr &cattr,
		 const FB_Buffer &buffer);

static inline void
SL_ScanConvertRL(const SL_FullEdge &fe,
		 const SL_SlopeEdge &se,
		 const SL_MinEdge &me,
		 Integer cur_y, const Integer y_mid, const Integer y_hi,
		 const IN_Slope &inddx,
		 const IS_CAttr &cattr,
		 const FB_Buffer &buffer);


#endif

/////////////////////////////////////////////////////






#include "sl_common.I"

#endif /* SL_COMMON_H */
