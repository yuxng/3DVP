/*

Brian Curless

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


#ifndef _DEFINES_
#define _DEFINES_

#ifndef LINUX
#include <malloc.h>
#endif

#ifdef LINUX
#include <stdlib.h>
#endif

#include <string.h>

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38F
#endif

#ifndef MAXFLOAT
#define MAXFLOAT FLT_MAX
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef SWAP
#define SWAP(a, b, t) (t) = (a); (a) = (b); (b) = (t)
#endif

#ifndef SWAP_INT
#define SWAP_INT(a, b) {int _temp_int = (a); (a) = (b); (b) = _temp_int;}
#endif

#ifndef SWAP_FLOAT
#define SWAP_FLOAT(a, b) {float _temp_float =(a); (a) =(b); (b) =_temp_float;}
#endif

#ifndef SWAP_USHORT
#define SWAP_USHORT(a, b) {unsigned short _temp_ushort = (a); (a) = (b); \
  (b) = _temp_ushort;}
#endif

#ifndef SQUARE
#define SQUARE(x) ((x)*(x))
#endif

#ifndef ROUND_UCHAR
#define ROUND_UCHAR(x) (uchar((x)+0.5))
#endif

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif

#ifndef SIGN
#define SIGN(x) ((x) > 0 ? 1 : -1)
#endif

#ifndef DEGTORAD
#define DEGTORAD(x) ((x)*M_PI/180)
#endif

#ifndef RAD
#define RAD(x) ((x)*M_PI/180)
#endif

#ifndef RADTODEG
#define RADTODEG(x) ((x)*180/M_PI)
#endif

#ifndef LINUX

#ifndef MALLOC
#define MALLOC(x, n) ((x*)malloc((n)*sizeof(x)))
#endif

#ifndef newmalloc
#define newmalloc(x, n) ((x*)malloc((n)*sizeof(x)))
#endif /* newmalloc */

#endif /* LINUX */

#ifndef PI
#define PI 3.14159265358979323846264
#endif

#ifndef EQSTR
#define EQSTR(x, y)  (strcmp((x),(y)) == 0)
#endif

#ifndef IS_ODD
#define IS_ODD(x)  ((x)%2 != 0)
#endif

#ifndef IS_EVEN
#define IS_EVEN(x)  ((x)%2 == 0)
#endif

#ifndef ROUNT_INT
#define ROUND_INT(x)  ((int)(x+0.5))
#endif


/* Watch out for BSD incompatibility */

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;


typedef uchar byte;

/* Stop using this? */
/*typedef int Bool;*/

#endif
