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


#ifndef _MATRIX4F_
#define _MATRIX4F_

#include "Vec3f.h"
#include "Matrix3f.h"
#include "defines.h"

class Matrix4f {
 public:
    float m[4][4];

    Matrix4f() {}
    Matrix4f(const Matrix4f &a) {setValue(a);}
    Matrix4f(float a[][4]) {setValue(a);}
    Matrix4f(float *a0, float *a1, float *a2, float *a3) 
        {setValue(a0, a1, a2, a3);}
    Matrix4f(float a00, float a01, float a02, float a03,
	     float a10, float a11, float a12, float a13,
	     float a20, float a21, float a22, float a23,
	     float a30, float a31, float a32, float a33)
        { setValue(a00, a01, a02, a03, 
		   a10, a11, a12, a13, 
		   a20, a21, a22, a23,
		   a30, a31, a32, a33); }

    void clear() {setValue(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);}
    void setValue(const Matrix4f &a);
    void setValue(float a[][4]);
    void setValue(float *a0, float *a1, float *a2, float *a3);
    void setValue(float a00, float a01, float a02, float a03,
		  float a10, float a11, float a12, float a13,
		  float a20, float a21, float a22, float a23,
		  float a30, float a31, float a32, float a33);
    void setValue(float *a);
    void getValue(float *a);
    void multVec(const Vec3f &src, Vec3f &dst);
    void multVecHomog(const Vec3f &src, Vec3f &dst);
    void multDir(const Vec3f &src, Vec3f &dst);
    float elem(int i, int j) {return m[i][j];}
    void setElem(int i, int j, float val) {m[i][j] = val;}
    void makeIdentity();
    Matrix4f & multRight(const Matrix4f &m);
    Matrix4f & multLeft(const Matrix4f &m);
    Matrix4f & multLeft(const Matrix3f &a);
    void setScale(float s);
    void setScale(float sx, float sy, float sz);
    void setScale(const Vec3f &s);
    void setRotateX(float theta);
    void setRotateY(float theta);
    void setRotateZ(float theta);
    void setTranslate(Vec3f t);
    void setTranslate(float tx, float ty, float tz);
    void scale(float s);
    void scale(float sx, float sy, float sz);
    void scale(const Vec3f &s);
    void rotateX(float theta);
    void rotateY(float theta);
    void rotateZ(float theta);
    void translate(Vec3f t);
    void translate(float tx, float ty, float tz);
    Matrix4f inverse() const;
    void transpose();
    //void freeNR(float **in) const; 
    //void toNR(Matrix4f &in, float **nrmat) const;
    int operator==(const Matrix4f &);
    int operator!=(const Matrix4f &);
    void print();
    void write(FILE *fp);
    void read(FILE *fp);
/*
    Matrix4f & operator *=(const Matrix4f &m);
    friend Matrix4f operator *(const Matrix4f &m1, const Matrix4f &m2);
*/
};

#endif
