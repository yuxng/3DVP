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


#ifndef _VEC3F_ 
#define _VEC3F_

#include <math.h>
#include "defines.h"
#include <iostream>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
//
//  Class: Vec3f
//
//  3D vector used to represet points or directions. Each component of
//  the vector is a float.
//
//////////////////////////////////////////////////////////////////////////////

class Vec3f {

//  protected:

  public:
    float       x, y, z;

    Vec3f()                                           { }
    Vec3f(const float v[3])                           { setValue(v); }
    Vec3f(float vx, float vy, float vz)               { setValue(vx, vy, vz); }

    float       dot(const Vec3f &v) const
        {return (x*v.x + y*v.y + z*v.z);}

    Vec3f       cross(const Vec3f &v) const
        {return Vec3f(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);}

    Vec3f       max(const Vec3f &v) const
        {return Vec3f(MAX(x, v.x), MAX(y, v.y), MAX(z, v.z));}

    Vec3f       min(const Vec3f &v) const
        {return Vec3f(MIN(x, v.x), MIN(y, v.y), MIN(z, v.z));}

    float       length() const {return sqrtf(this->dot(*this));}

    float       lengthSquared() const {return this->dot(*this);}

    void        normalize()
        { float len = length(); if (len > 0) *this /= len; }

    void        negate() {*this *= -1;}

    void        getValue(float &vx, float &vy, float &vz) 
	const {vx = x; vy = y; vz = z;}

    void        getValue(float *v) 
	const {v[0] = x; v[1] = y; v[2] = z;}

    void print()
        {printf("%f  %f  %f\n", x, y, z);}

    Vec3f & clear()
        {setValue(0,0,0); return *this;}

    Vec3f &   setValue(const float v[3])
        {  // handle: v = 0;
	   if (v) { x = v[0]; y = v[1]; z = v[2]; } else x = y = z = 0; 
	   return *this; 
	}

    Vec3f &   setValue(float vx, float vy, float vz)
        { x = vx; y = vy; z = vz; return *this; }

    Vec3f &   setValue(const Vec3f &v1)
        { x = v1.x; y = v1.y; z = v1.z; return *this; }

    Vec3f &    translate(float vx, float vy, float vz)
        { x += vx; y += vy; z += vz; return *this; }

    Vec3f &     operator *=(float d)
        { x *= d; y *= d; z *= d; return *this; }

    Vec3f &     operator /=(float d)
        { return *this *= (1.0 / d); }

    Vec3f &     operator +=(const Vec3f &v1)
        { x += v1.x; y += v1.y; z += v1.z; return *this; }

    Vec3f &     operator -=(const Vec3f &v1)
        { x -= v1.x; y -= v1.y; z -= v1.z; return *this; }

    const float & operator [](int i) const
        { return (*(&x + i)); }

    friend int    operator ==(const Vec3f &v1, const Vec3f &v2)
        { return ((v1.x == v2.x)&&(v1.y == v2.y)
		  &&(v1.z == v2.z)); }

    friend int    operator !=(const Vec3f &v1, const Vec3f &v2)
        { return !(v1 == v2); }

    friend Vec3f      operator +(const Vec3f &v1, const Vec3f &v2)
        { return Vec3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }

    friend Vec3f      operator -(const Vec3f &v1, const Vec3f &v2)
        { return Vec3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
   
    friend Vec3f      operator *(const float d, const Vec3f &v1)
        { return Vec3f(d * v1.x, d * v1.y, d * v1.z); }

    friend Vec3f      operator *(const Vec3f &v1, const float d)
        { return d * v1; }
};

std::ostream &operator<<(std::ostream &, const Vec3f &);

#endif

