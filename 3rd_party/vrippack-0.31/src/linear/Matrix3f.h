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


#ifndef _MATRIX3F_
#define _MATRIX3F_

#include "Vec3f.h"
#include "defines.h"

class Matrix3f {
    friend class Vec3f;

 public:
    float m[3][3];

    Matrix3f() {}
    Matrix3f(const Matrix3f &a) {setValue(a);}
    Matrix3f(float a[][3]) {setValue(a);}
    Matrix3f(float *a0, float *a1, float *a2) {setValue(a0, a1, a2);}
    Matrix3f(Vec3f a0, Vec3f a1, Vec3f a2) {setValue(a0, a1, a2);}
    Matrix3f(float a00, float a01, float a02,
		  float a10, float a11, float a12,
		  float a20, float a21, float a22)
        { setValue(a00, a01, a02, a10, a11, a12, a20, a21, a22); }
    void clear() {setValue(0,0,0,0,0,0,0,0,0);}
    void makeIdentity();
    void setValue(const Matrix3f &a);
    void setValue(float a[][3]);
    void setValue(float *a0, float *a1, float *a2);
    void setValue(Vec3f a0, Vec3f a1, Vec3f a2);
    void setValue(float a00, float a01, float a02,
		  float a10, float a11, float a12,
		  float a20, float a21, float a22);
    Vec3f mult(Vec3f &vec);
    float elem(int i, int j) {return m[i][j];}
    void setElem(int i, int j, float val) {m[i][j] = val;}
    Matrix3f&     operator +=(const Matrix3f &a);
    float determinant();
    int operator==(const Matrix3f &);
    int operator!=(const Matrix3f &);
    Matrix3f & operator*=(float);
    Matrix3f inverse() const;
    void map(const Vec3f vin[3], const Vec3f vout[3]);
    void toNR(Matrix3f &in, float **nrmat) const;
    void setScale(float s);
    void setScale(float sx, float sy, float sz);
    void setScale(const Vec3f &s);
    Matrix3f & multLeft(const Matrix3f &m);
};

#endif
