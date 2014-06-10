/*
 * sl_val.H
 *
 */

#ifndef SL_VAL_H
#define SL_VAL_H

//#include <values.h>
#include <float.h>
#include <limits.h>
#include <math.h>

typedef int Integer;
typedef unsigned char UnsignedInteger8;
typedef signed char Integer8;
typedef short Integer16;
typedef int Integer32;
typedef long IntegerPS;
typedef float Real;


#ifdef LINUX

#define CeilR2I(x) ((Integer) ceil(x))
#define Abs(x)     (fabs(x))

#else

#define CeilR2I(x) ((Integer) ceilf(x))
#define Abs(x)     (fabs(x))

#endif


#define MAX_SCREEN_DIMENSION 1280

#define SMALLEST_INTEGER 0x80000000
#define REAL_ONE 1.0f

#define REAL_TOTAL_BITS 32
#define REAL_MANTISSA_BITS 23
#define REAL_NON_MANTISSA_BITS 9
#define REAL_EXPONENT_BIAS 127
#define REAL_MANTISSA_MASK 0x007fffff
#define REAL_POW_MANTISSA_BITS 1.1920928955078125e-07f

#endif /* VAL_H */
