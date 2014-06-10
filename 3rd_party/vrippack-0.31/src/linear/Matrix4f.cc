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


#include "Matrix4f.h"
#include "defines.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>

/* #include "NR.h" */

void
Matrix4f::setValue(float *a)
{
    m[0][0] = a[0];   m[0][1] = a[1];   m[0][2] = a[2];   m[0][3] = a[3];
    m[1][0] = a[4];   m[1][1] = a[5];   m[1][2] = a[6];   m[1][3] = a[7];
    m[2][0] = a[8];   m[2][1] = a[9];   m[2][2] = a[10];  m[2][3] = a[11];
    m[3][0] = a[12];  m[3][1] = a[13];  m[3][2] = a[14];  m[3][3] = a[15];
}

void
Matrix4f::getValue(float *a)
{
    a[0] = m[0][0];   a[1] = m[0][1];   a[2] = m[0][2];   a[3] = m[0][3];
    a[4] = m[1][0];   a[5] = m[1][1];   a[6] = m[1][2];   a[7] = m[1][3];
    a[8] = m[2][0];   a[9] = m[2][1];   a[10] = m[2][2];  a[11] = m[2][3];
    a[12] = m[3][0];  a[13] = m[3][1];  a[14] = m[3][2];  a[15] = m[3][3];
}

void
Matrix4f::setValue(float a[][4])
{
    m[0][0] = a[0][0];  m[0][1] = a[0][1];  m[0][2] = a[0][2];
    m[0][3] = a[0][3];

    m[1][0] = a[1][0];  m[1][1] = a[1][1];  m[1][2] = a[1][2];
    m[1][3] = a[1][3];

    m[2][0] = a[2][0];  m[2][1] = a[2][1];  m[2][2] = a[2][2];
    m[2][3] = a[2][3];

    m[3][0] = a[3][0];  m[3][1] = a[3][1];  m[3][2] = a[3][2];
    m[3][3] = a[3][3];
}

void
Matrix4f::setValue(const Matrix4f &a)
{
    m[0][0] = a.m[0][0];  m[0][1] = a.m[0][1];  m[0][2] = a.m[0][2];
    m[0][3] = a.m[0][3];

    m[1][0] = a.m[1][0];  m[1][1] = a.m[1][1];  m[1][2] = a.m[1][2];
    m[1][3] = a.m[1][3];

    m[2][0] = a.m[2][0];  m[2][1] = a.m[2][1];  m[2][2] = a.m[2][2];
    m[2][3] = a.m[2][3];

    m[3][0] = a.m[3][0];  m[3][1] = a.m[3][1];  m[3][2] = a.m[3][2];
    m[3][3] = a.m[3][3];
}

void
Matrix4f::setValue(float *a0, float *a1, float *a2, float *a3)
{
    m[0][0] = a0[0];  m[0][1] = a0[1];  m[0][2] = a0[2];  m[0][3] = a0[3];
    m[1][0] = a1[0];  m[1][1] = a1[1];  m[1][2] = a1[2];  m[1][3] = a1[3];
    m[2][0] = a2[0];  m[2][1] = a2[1];  m[2][2] = a2[2];  m[2][3] = a2[3];
    m[3][0] = a3[0];  m[3][1] = a3[1];  m[3][2] = a3[2];  m[3][3] = a3[3];
}

void
Matrix4f::setValue(float a00, float a01, float a02, float a03,
		   float a10, float a11, float a12, float a13,
		   float a20, float a21, float a22, float a23,
		   float a30, float a31, float a32, float a33)
{
    m[0][0] = a00;  m[0][1] = a01;  m[0][2] = a02;  m[0][3] = a03;
    m[1][0] = a10;  m[1][1] = a11;  m[1][2] = a12;  m[1][3] = a13;
    m[2][0] = a20;  m[2][1] = a21;  m[2][2] = a22;  m[2][3] = a23;
    m[3][0] = a30;  m[3][1] = a31;  m[3][2] = a32;  m[3][3] = a33;
}


void
Matrix4f::multVec(const Vec3f &src, Vec3f &dst)
{
    dst.setValue(m[0][0] * src[0] + m[0][1] * src[1] 
		 + m[0][2] * src[2] + m[0][3],
		 m[1][0] * src[0] + m[1][1] * src[1] 
		 + m[1][2] * src[2] + m[1][3],
		 m[2][0] * src[0] + m[2][1] * src[1] 
		 + m[2][2] * src[2] + m[2][3]);
}

void
Matrix4f::multVecHomog(const Vec3f &src, Vec3f &dst)
{
    float w;

    dst.setValue(m[0][0] * src[0] + m[0][1] * src[1] 
		 + m[0][2] * src[2] + m[0][3],
		 m[1][0] * src[0] + m[1][1] * src[1] 
		 + m[1][2] * src[2] + m[1][3],
		 m[2][0] * src[0] + m[2][1] * src[1] 
		 + m[2][2] * src[2] + m[2][3]);

    w = m[3][0]*src[0] + m[3][1]*src[1] + m[3][2]*src[2] + m[3][3]; 

    dst /= w;
}

void
Matrix4f::multDir(const Vec3f& src, Vec3f &dst)
{
    dst.setValue(m[0][0] * src[0] + m[0][1] * src[1] + m[0][2] * src[2],
		 m[1][0] * src[0] + m[1][1] * src[1] + m[1][2] * src[2],
		 m[2][0] * src[0] + m[2][1] * src[1] + m[2][2] * src[2]);
}


void
Matrix4f::makeIdentity() 
{
    setValue(1,0,0,0,
	     0,1,0,0,
	     0,0,1,0,
	     0,0,0,1);
}


Matrix4f &
Matrix4f::multRight(const Matrix4f &a)
{
    Matrix4f temp;

    temp.clear();
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    for (int k = 0; k < 4; k++)
		temp.m[i][j] += m[i][k] * a.m[k][j];

    setValue(temp);

    return *this;
}


Matrix4f & 
Matrix4f::multLeft(const Matrix4f &a)
{
    Matrix4f temp;

    temp.clear();
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    for (int k = 0; k < 4; k++)
		temp.m[i][j] += a.m[i][k] * m[k][j];

    setValue(temp);    

    return *this;
}


Matrix4f & 
Matrix4f::multLeft(const Matrix3f &a)
{
    Matrix4f temp;
    int i;

    temp.clear();
    for (i = 0; i < 3; i++)
	for (int j = 0; j < 3; j++)
	    for (int k = 0; k < 3; k++)
		temp.m[i][j] += a.m[i][k] * m[k][j];
    
    for (i = 0; i < 4; i++) {
	temp.m[3][i] = m[3][i];
	temp.m[i][3] = m[i][3];
    }

    setValue(temp);    

    return *this;
}


void 
Matrix4f::setScale(float s)
{
    setScale(s, s, s);
}


void 
Matrix4f::setScale(float sx, float sy, float sz)
{
    m[0][0] = sx; m[0][1] = 0; m[0][2] = 0;
    m[1][0] = 0; m[1][1] = sy; m[1][2] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = sz;
}


void 
Matrix4f::setScale(const Vec3f &s)
{
    setScale(s[0], s[1], s[2]);
}


void 
Matrix4f::scale(float s)
{
    setScale(s, s, s);
}


void 
Matrix4f::scale(float sx, float sy, float sz)
{
    m[0][0] *= sx; m[0][1] *= sx; m[0][2] *= sx; m[0][3] *= sx;
    m[1][0] *= sy; m[1][1] *= sy; m[1][2] *= sy; m[1][3] *= sy;
    m[2][0] *= sz; m[2][1] *= sz; m[2][2] *= sz; m[2][3] *= sz;
}


void 
Matrix4f::scale(const Vec3f &s)
{
    scale(s[0], s[1], s[2]);
}


void 
Matrix4f::rotateX(float theta) 
{
    Matrix4f temp;
    temp.makeIdentity();
    temp.setRotateX(theta);
    multLeft(temp);
}

void 
Matrix4f::rotateY(float theta) 
{
    Matrix4f temp;
    temp.makeIdentity();
    temp.setRotateY(theta);
    multLeft(temp);
}

void 
Matrix4f::rotateZ(float theta) 
{
    Matrix4f temp;
    temp.makeIdentity();
    temp.setRotateZ(theta);
    multLeft(temp);
}


void 
Matrix4f::setRotateX(float theta) 
{
    m[0][0] = 1;            m[0][1] = 0;            m[0][2] = 0;
    m[1][0] = 0;            m[1][1] = cos(theta);   m[1][2] = -sin(theta);
    m[2][0] = 0;            m[2][1] = sin(theta);   m[2][2] = cos(theta);
}

void 
Matrix4f::setRotateY(float theta) 
{
    m[0][0] = cos(theta);   m[0][1] = 0;            m[0][2] = sin(theta);
    m[1][0] = 0;            m[1][1] = 1;            m[1][2] = 0;
    m[2][0] = -sin(theta);  m[2][1] = 0;            m[2][2] = cos(theta);
}

void 
Matrix4f::setRotateZ(float theta) 
{
    m[0][0] = cos(theta);   m[0][1] = -sin(theta);  m[0][2] = 0;
    m[1][0] = sin(theta);   m[1][1] = cos(theta);   m[1][2] = 0;
    m[2][0] = 0;            m[2][1] = 0;            m[2][2] = 1;
}

void 
Matrix4f::setTranslate(Vec3f t) 
{
    setTranslate(t[0], t[1], t[2]);
}


void 
Matrix4f::setTranslate(float tx, float ty, float tz) 
{
    m[0][3] = tx;
    m[1][3] = ty;
    m[2][3] = tz;
}


void 
Matrix4f::translate(Vec3f t) 
{
    translate(t[0], t[1], t[2]);
}


void 
Matrix4f::translate(float tx, float ty, float tz) 
{
    m[0][3] += tx;
    m[1][3] += ty;
    m[2][3] += tz;
}



Matrix4f 
Matrix4f::inverse() const 
{
   Matrix4f result;
   int i, j, k;
   double temp;
   double bigm[8][4];
   /*   Declare identity matrix   */

   result.makeIdentity();
   for (i = 0; i < 4; i++) {
      for (j = 0;  j < 4;  j++) {
	 bigm[i][j] = m[i][j];
	 bigm[i+4][j] = result.m[i][j];
      }
   }

   /*   Work across by columns   */
   for (i = 0;  i < 4;  i++) {
      for (j = i;  (bigm[i][j] == 0.0) && (j < 4);  j++)
	 ;
      if (j == 4) {
	 fprintf (stderr, "error:  cannot do inverse matrix\n");
	 exit (2);
      } 
      else if (i != j) {
	 for (k = 0;  k < 8;  k++) {
	    temp = bigm[k][i];   
	    bigm[k][i] = bigm[k][j];   
	    bigm[k][j] = temp;
	 }
      }
      
      /*   Divide original row   */      
      for (j = 7;  j >= i;  j--)
	 bigm[j][i] /= bigm[i][i];
      
      /*   Subtract other rows   */      
      for (j = 0;  j < 4;  j++)
	 if (i != j)
	    for (k = 7;  k >= i;  k--)
	       bigm[k][j] -= bigm[k][i] * bigm[i][j];
   }
   
   for (i = 0;  i < 4;  i++)
      for (j = 0;  j < 4;  j++)
	 result.m[i][j] = bigm[i+4][j];

   return result;
}


void
Matrix4f::transpose()
{
    float temp;

    SWAP(m[0][1], m[1][0], temp);
    SWAP(m[0][2], m[2][0], temp);
    SWAP(m[0][3], m[3][0], temp);
    SWAP(m[1][2], m[2][1], temp);
    SWAP(m[1][3], m[3][1], temp);
    SWAP(m[2][3], m[3][2], temp);
}


int
Matrix4f::operator!=(const Matrix4f &a)
{
    if (m[0][0] != a.m[0][0]) return TRUE;
    if (m[0][1] != a.m[0][1]) return TRUE;
    if (m[0][2] != a.m[0][2]) return TRUE;
    if (m[0][3] != a.m[0][3]) return TRUE;
    if (m[1][0] != a.m[1][0]) return TRUE;
    if (m[1][1] != a.m[1][1]) return TRUE;
    if (m[1][2] != a.m[1][2]) return TRUE;
    if (m[1][3] != a.m[1][3]) return TRUE;
    if (m[2][0] != a.m[2][0]) return TRUE;
    if (m[2][1] != a.m[2][1]) return TRUE;
    if (m[2][2] != a.m[2][2]) return TRUE;
    if (m[2][3] != a.m[2][3]) return TRUE;
    if (m[3][0] != a.m[3][0]) return TRUE;
    if (m[3][1] != a.m[3][1]) return TRUE;
    if (m[3][2] != a.m[3][2]) return TRUE;
    if (m[3][3] != a.m[3][3]) return TRUE;

    return FALSE;
}

int
Matrix4f::operator==(const Matrix4f &a)
{
    if (m[0][0] != a.m[0][0]) return FALSE;
    if (m[0][1] != a.m[0][1]) return FALSE;
    if (m[0][2] != a.m[0][2]) return FALSE;
    if (m[0][3] != a.m[0][3]) return FALSE;
    if (m[1][0] != a.m[1][0]) return FALSE;
    if (m[1][1] != a.m[1][1]) return FALSE;
    if (m[1][2] != a.m[1][2]) return FALSE;
    if (m[1][3] != a.m[1][3]) return FALSE;
    if (m[2][0] != a.m[2][0]) return FALSE;
    if (m[2][1] != a.m[2][1]) return FALSE;
    if (m[2][2] != a.m[2][2]) return FALSE;
    if (m[2][3] != a.m[2][3]) return FALSE;
    if (m[3][0] != a.m[3][0]) return FALSE;
    if (m[3][1] != a.m[3][1]) return FALSE;
    if (m[3][2] != a.m[3][2]) return FALSE;
    if (m[3][3] != a.m[3][3]) return FALSE;

    return TRUE;
}

void
Matrix4f::print()
{
    for (int i=0; i<4; i++) {
	for (int j=0; j<4; j++) {
	    printf("%f\t", m[i][j]);
	}
	printf("\n");
    }
}

void
Matrix4f::read(FILE *fp)
{
   float a0, a1, a2, a3;

   fscanf(fp, "%f %f %f %f", &a0, &a1, &a2, &a3);
   this->setElem(0,0,a0);
   this->setElem(0,1,a1);
   this->setElem(0,2,a2);
   this->setElem(0,3,a3);

   fscanf(fp, "%f %f %f %f", &a0, &a1, &a2, &a3);
   this->setElem(1,0,a0);
   this->setElem(1,1,a1);
   this->setElem(1,2,a2);
   this->setElem(1,3,a3);

   fscanf(fp, "%f %f %f %f", &a0, &a1, &a2, &a3);
   this->setElem(2,0,a0);
   this->setElem(2,1,a1);
   this->setElem(2,2,a2);
   this->setElem(2,3,a3);

   fscanf(fp, "%f %f %f %f", &a0, &a1, &a2, &a3);
   this->setElem(3,0,a0);
   this->setElem(3,1,a1);
   this->setElem(3,2,a2);
   this->setElem(3,3,a3);
}

void
Matrix4f::write(FILE *fp)
{
   float a0, a1, a2, a3;

   a0 = this->elem(0,0);
   a1 = this->elem(0,1);
   a2 = this->elem(0,2);
   a3 = this->elem(0,3);
   fprintf(fp, "%f %f %f %f\n", a0, a1, a2, a3);

   a0 = this->elem(1,0);
   a1 = this->elem(1,1);
   a2 = this->elem(1,2);
   a3 = this->elem(1,3);
   fprintf(fp, "%f %f %f %f\n", a0, a1, a2, a3);

   a0 = this->elem(2,0);
   a1 = this->elem(2,1);
   a2 = this->elem(2,2);
   a3 = this->elem(2,3);
   fprintf(fp, "%f %f %f %f\n", a0, a1, a2, a3);

   a0 = this->elem(3,0);
   a1 = this->elem(3,1);
   a2 = this->elem(3,2);
   a3 = this->elem(3,3);
   fprintf(fp, "%f %f %f %f\n", a0, a1, a2, a3);

}


/*
Matrix4f 
Matrix4f::inverse() const 
{
    Matrix4f temp, result;
    int i;

    temp.setValue(*this);
    float *nrmat[4];

    toNR(temp, nrmat);

    // See NRC book
    float d, col[4];
    int indx[4];
    ludcmp(nrmat-1, 4, indx-1, &d);
    for (int j = 0; j < 4; j++) {
	for (i = 0; i < 4; i++) col[i] = 0;
	col[j] = 1.0;
	lubksb(nrmat-1, 4, indx-1, col-1);
	for(i = 0; i < 4; i++) result.m[i][j] = col[i];
    }

    return Matrix4f(result);
}


void
Matrix4f::toNR(Matrix4f &in, float **nrmat) const
{
    nrmat[0] = in.m[0] - 1;
    nrmat[1] = in.m[1] - 1;
    nrmat[2] = in.m[2] - 1;
    nrmat[3] = in.m[3] - 1;
}


void
Matrix4f::freeNR(float **in) const
{
    free(in);
}


Matrix4f & 
Matrix4f::operator *=(const Matrix4f &m) 
{
}


friend Matrix4f 
Matrix4f::operator *(const Matrix4f &m1, const Matrix4f &m2)
{
}
*/
