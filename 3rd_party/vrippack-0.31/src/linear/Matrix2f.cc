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
#include "Matrix2f.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

/*#include "NR.h"*/

void
Matrix2f::setValue(float a[][2])
{
    m[0][0] = a[0][0];  m[0][1] = a[0][1];
    m[1][0] = a[1][0];  m[1][1] = a[1][1];
}

void
Matrix2f::setValue(const Matrix2f &a)
{
    m[0][0] = a.m[0][0];  m[0][1] = a.m[0][1];
    m[1][0] = a.m[1][0];  m[1][1] = a.m[1][1];
}

void
Matrix2f::setValue(float *a0, float *a1)
{
    m[0][0] = a0[0];  m[0][1] = a0[1];
    m[1][0] = a1[0];  m[1][1] = a1[1];
}

void
Matrix2f::setValue(float a00, float a01, float a10, float a11)
{
    m[0][0] = a00;  m[0][1] = a01;
    m[1][0] = a10;  m[1][1] = a11;
}


void
Matrix2f::multArray(float *in, float *out)
{
    out[0] = m[0][0] * in[0] + m[0][1] * in[1];
    out[1] = m[1][0] * in[0] + m[1][1] * in[1];
}

void 
Matrix2f::setScale(float s)
{
    setScale(s, s);
}


void 
Matrix2f::setScale(float sx, float sy)
{
    m[0][0] = sx; m[0][1] = 0;
    m[1][0] = 0; m[1][1] = sy;
}

void
Matrix2f::makeIdentity() 
{
    setValue(1,0,
	     0,1);
}


Matrix2f 
Matrix2f::inverse() const 
{
   Matrix2f result;
   int i, j, k;
   double temp;
   double bigm[4][2];
   /*   Declare identity matrix   */

   result.makeIdentity();
   for (i = 0; i < 2; i++) {
      for (j = 0;  j < 2;  j++) {
	 bigm[i][j] = m[i][j];
	 bigm[i+2][j] = result.m[i][j];
      }
   }

   /*   Work across by columns   */
   for (i = 0;  i < 2;  i++) {
      for (j = i;  (bigm[i][j] == 0.0) && (j < 2);  j++)
	 ;
      if (j == 2) {
	 fprintf (stderr, "error:  cannot do inverse matrix\n");
	 exit (2);
      } 
      else if (i != j) {
	 for (k = 0;  k < 4;  k++) {
	    temp = bigm[k][i];   
	    bigm[k][i] = bigm[k][j];   
	    bigm[k][j] = temp;
	 }
      }
      
      /*   Divide original row   */      
      for (j = 3;  j >= i;  j--)
	 bigm[j][i] /= bigm[i][i];
      
      /*   Subtract other rows   */      
      for (j = 0;  j < 2;  j++)
	 if (i != j)
	    for (k = 3;  k >= i;  k--)
	       bigm[k][j] -= bigm[k][i] * bigm[i][j];
   }
   
   for (i = 0;  i < 2;  i++)
      for (j = 0;  j < 2;  j++)
	 result.m[i][j] = bigm[i+2][j];

   return result;
}



/*
Matrix2f
Matrix2f::inverse() const 
{
    Matrix2f temp, result;
    int i;

    temp.setValue(*this);
    float *nrmat[2];

    toNR(temp, nrmat);

    // See NRC book
    float d, col[2];
    int indx[2];
    ludcmp(nrmat-1, 2, indx-1, &d);
    for (int j = 0; j < 2; j++) {
	for (i = 0; i < 2; i++) col[i] = 0;
	col[j] = 1.0;
	lubksb(nrmat-1, 2, indx-1, col-1);
	for(i = 0; i < 2; i++) result.m[i][j] = col[i];
    }

    return Matrix2f(result);
}


void
Matrix2f::toNR(Matrix2f &in, float **nrmat) const
{
    nrmat[0] = in.m[0] - 1;
    nrmat[1] = in.m[1] - 1;
}
*/

