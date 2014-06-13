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


#include "defines.h"
#include "Matrix3f.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>

/* #include "NR.h" */

void
Matrix3f::setValue(float a[][3])
{
    m[0][0] = a[0][0];  m[0][1] = a[0][1];  m[0][2] = a[0][2];
    m[1][0] = a[1][0];  m[1][1] = a[1][1];  m[1][2] = a[1][2];
    m[2][0] = a[2][0];  m[2][1] = a[2][1];  m[2][2] = a[2][2];
}

void
Matrix3f::setValue(const Matrix3f &a)
{
    m[0][0] = a.m[0][0];  m[0][1] = a.m[0][1];  m[0][2] = a.m[0][2];
    m[1][0] = a.m[1][0];  m[1][1] = a.m[1][1];  m[1][2] = a.m[1][2];
    m[2][0] = a.m[2][0];  m[2][1] = a.m[2][1];  m[2][2] = a.m[2][2];
}

void
Matrix3f::setValue(float *a0, float *a1, float *a2)
{
    m[0][0] = a0[0];  m[0][1] = a0[1];  m[0][2] = a0[2];
    m[1][0] = a1[0];  m[1][1] = a1[1];  m[1][2] = a1[2];
    m[2][0] = a2[0];  m[2][1] = a2[1];  m[2][2] = a2[2];
}

void
Matrix3f::setValue(Vec3f a0, Vec3f a1, Vec3f a2)
{
    m[0][0] = a0[0];  m[0][1] = a0[1];  m[0][2] = a0[2];
    m[1][0] = a1[0];  m[1][1] = a1[1];  m[1][2] = a1[2];
    m[2][0] = a2[0];  m[2][1] = a2[1];  m[2][2] = a2[2];
}

void
Matrix3f::setValue(float a00, float a01, float a02,
		  float a10, float a11, float a12,
		  float a20, float a21, float a22)
{
    m[0][0] = a00;  m[0][1] = a01;  m[0][2] = a02;
    m[1][0] = a10;  m[1][1] = a11;  m[1][2] = a12;
    m[2][0] = a20;  m[2][1] = a21;  m[2][2] = a22;
}


void
Matrix3f::makeIdentity() 
{
    setValue(1,0,0,
	     0,1,0,
	     0,0,1);
}


Vec3f
Matrix3f::mult(Vec3f& vec)
{
    return Vec3f(m[0][0] * vec[0] + m[0][1] * vec[1] + m[0][2] * vec[2],
		 m[1][0] * vec[0] + m[1][1] * vec[1] + m[1][2] * vec[2],
		 m[2][0] * vec[0] + m[2][1] * vec[1] + m[2][2] * vec[2]);
}

void 
Matrix3f::setScale(float s)
{
    setScale(s, s, s);
}


void 
Matrix3f::setScale(float sx, float sy, float sz)
{
    m[0][0] = sx; m[0][1] = 0; m[0][2] = 0;
    m[1][0] = 0; m[1][1] = sy; m[1][2] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = sz;
}


void 
Matrix3f::setScale(const Vec3f &s)
{
    setScale(s[0], s[1], s[2]);
}


Matrix3f & 
Matrix3f::multLeft(const Matrix3f &a)
{
    Matrix3f temp;

    temp.clear();
    for (int i = 0; i < 3; i++)
	for (int j = 0; j < 3; j++)
	    for (int k = 0; k < 3; k++)
		temp.m[i][j] += a.m[i][k] * m[k][j];

    setValue(temp);    

    return *this;
}

Matrix3f&
Matrix3f::operator+=(const Matrix3f &a)
{
    m[0][0] += a.m[0][0];  m[0][1] += a.m[0][1];  m[0][2] += a.m[0][2];
    m[1][0] += a.m[1][0];  m[1][1] += a.m[1][1];  m[1][2] += a.m[1][2];
    m[2][0] += a.m[2][0];  m[2][1] += a.m[2][1];  m[2][2] += a.m[2][2];
    return *this;
}


Matrix3f&
Matrix3f::operator*=(float a)
{
    m[0][0] *= a;  m[0][1] *= a;  m[0][2] *= a;
    m[1][0] *= a;  m[1][1] *= a;  m[1][2] *= a;
    m[2][0] *= a;  m[2][1] *= a;  m[2][2] *= a;
    return *this;
}


int
Matrix3f::operator!=(const Matrix3f &a)
{
    if (m[0][0] != a.m[0][0]) return TRUE;
    if (m[0][1] != a.m[0][1]) return TRUE;
    if (m[0][2] != a.m[0][2]) return TRUE;
    if (m[1][0] != a.m[1][0]) return TRUE;
    if (m[1][1] != a.m[1][1]) return TRUE;
    if (m[1][2] != a.m[1][2]) return TRUE;
    if (m[2][0] != a.m[2][0]) return TRUE;
    if (m[2][1] != a.m[2][1]) return TRUE;
    if (m[2][2] != a.m[2][2]) return TRUE;

    return FALSE;
}

int
Matrix3f::operator==(const Matrix3f &a)
{
    if (m[0][0] != a.m[0][0]) return FALSE;
    if (m[0][1] != a.m[0][1]) return FALSE;
    if (m[0][2] != a.m[0][2]) return FALSE;
    if (m[1][0] != a.m[1][0]) return FALSE;
    if (m[1][1] != a.m[1][1]) return FALSE;
    if (m[1][2] != a.m[1][2]) return FALSE;
    if (m[2][0] != a.m[2][0]) return FALSE;
    if (m[2][1] != a.m[2][1]) return FALSE;
    if (m[2][2] != a.m[2][2]) return FALSE;

    return TRUE;
}


float
Matrix3f::determinant()
{
    return 
	m[0][0] * m[1][1] * m[2][2] +
	m[0][1] * m[1][2] * m[2][0] +
	m[0][2] * m[1][0] * m[2][1] -
	m[0][2] * m[1][1] * m[2][0] - 
	m[0][1] * m[1][0] * m[2][2] - 
	m[0][0] * m[1][2] * m[2][1];
}


Matrix3f 
Matrix3f::inverse() const 
{
   Matrix3f result;
   int i, j, k;
   double temp;
   double bigm[6][3];
   /*   Declare identity matrix   */

   result.makeIdentity();
   for (i = 0; i < 3; i++) {
      for (j = 0;  j < 3;  j++) {
	 bigm[i][j] = m[i][j];
	 bigm[i+3][j] = result.m[i][j];
      }
   }

   /*   Work across by columns   */
   for (i = 0;  i < 3;  i++) {
      for (j = i;  (bigm[i][j] == 0.0) && (j < 3);  j++)
	 ;
      if (j == 3) {
	 fprintf (stderr, "error:  cannot do inverse matrix\n");
	 exit (2);
      } 
      else if (i != j) {
	 for (k = 0;  k < 6;  k++) {
	    temp = bigm[k][i];   
	    bigm[k][i] = bigm[k][j];   
	    bigm[k][j] = temp;
	 }
      }
      
      /*   Divide original row   */
      
      for (j = 5;  j >= i;  j--)
	 bigm[j][i] /= bigm[i][i];
      
      /*   Subtract other rows   */
      
      for (j = 0;  j < 3;  j++)
	 if (i != j)
	    for (k = 5;  k >= i;  k--)
	       bigm[k][j] -= bigm[k][i] * bigm[i][j];
   }
   
   for (i = 0;  i < 3;  i++)
      for (j = 0;  j < 3;  j++)
	 result.m[i][j] = bigm[i+3][j];

   return result;
}


void
Matrix3f::map(const Vec3f vin[3], const Vec3f vout[3])
{
/*
    Vec3f x0, x1, x2;
    Matrix3f A;
    float *nrmat[3], d, x0[3], x1[3], x2[3];
    int indx[3];

    A.setValue(vin[0], vin[1], vin[2]);
    toNR(A, nrmat);
    ludcmp(nrmat-1, 3, indx-1, &d);

    x0.setValue(vout[0][0], vout[1][0], vout[2][0]);
    lubksb(nrmat-1, 3, indx-1, &x0[0] - 1);

    x1.setValue(vout[0][1], vout[1][1], vout[2][1]);    
    lubksb(nrmat-1, 3, indx-1, &x1[0] - 1);

    x2.setValue(vout[0][2], vout[1][2], vout[2][2]);    
    lubksb(nrmat-1, 3, indx-1, &x2[0] - 1);

    setValue(x0, x1, x2);
*/
}


/*

Matrix3f 
Matrix3f::inverse() const 
{
    Matrix3f temp, result;
    int i;

    temp.setValue(*this);
    float *nrmat[3];

    toNR(temp, nrmat);

    // See NRC book
    float d, col[3];
    int indx[3];
    ludcmp(nrmat-1, 3, indx-1, &d);
    for (int j = 0; j < 3; j++) {
	for (i = 0; i < 3; i++) col[i] = 0;
	col[j] = 1.0;
	lubksb(nrmat-1, 3, indx-1, col-1);
	for(i = 0; i < 3; i++) result.m[i][j] = col[i];
    }

    return Matrix3f(result);
}


void
Matrix3f::toNR(Matrix3f &in, float **nrmat) const
{
    nrmat[0] = in.m[0] - 1;
    nrmat[1] = in.m[1] - 1;
    nrmat[2] = in.m[2] - 1;
}
*/

