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


#ifndef _MATRIX2F_
#define _MATRIX2F_

#include "defines.h"

class Matrix2f {

 public:
    float m[2][2];

    Matrix2f() {}
    void clear() {setValue(0,0,0,0);}
    void setValue(const Matrix2f &a);
    void setValue(float a[][2]);
    void setValue(float *a0, float *a1);
    void setValue(float a00, float a01, float a10,
		  float a11);
    float elem(int i, int j) {return m[i][j];}
    void setElem(int i, int j, float val) {m[i][j] = val;}
    Matrix2f inverse() const;
    void multArray(float *in, float *out);
    void makeIdentity();
    //void toNR(Matrix2f &in, float **nrmat) const;
    void setScale(float s);
    void setScale(float sx, float sy);
};

#endif
