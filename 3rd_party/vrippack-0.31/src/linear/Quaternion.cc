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


#include "Quaternion.h"
#include "Matrix4f.h"

#define X 0
#define Y 1
#define Z 2
#define W 3


Quaternion::Quaternion()
{
    q[0] = 0;
    q[1] = 0;
    q[2] = 0;
    q[3] = 1;
}

Quaternion::~Quaternion()
{
}


void
Quaternion::setValue(float q0, float q1, float q2, float q3)
{
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
}

void
Quaternion::setValue(float *quat)
{
    q[0] = quat[0];
    q[1] = quat[1];
    q[2] = quat[2];
    q[3] = quat[3];
}


void
Quaternion::setValue(Quaternion &quat)
{
    q[0] = quat.q[0];
    q[1] = quat.q[1];
    q[2] = quat.q[2];
    q[3] = quat.q[3];
}


/******************************************************************************
Convert a quaternion into a rotation matrix.  Does *not* assume a unit
quaternion.  From Ken Shoemake.

******************************************************************************/
void
Quaternion::toMatrix(Matrix4f &mat)
{
    float s;
    float xs,ys,zs;
    float wx,wy,wz;
    float xx,xy,xz;
    float yy,yz,zz;
    
    /* for unit q, just set s = 2 or set xs = this->q[X] + this->q[X], etc. */
    
    s = 2 / (this->q[X]*this->q[X] + this->q[Y]*this->q[Y] 
	     + this->q[Z]*this->q[Z] + this->q[W]*this->q[W]);
    
    xs = this->q[X] * s;
    ys = this->q[Y] * s;
    zs = this->q[Z] * s;
    
    wx = this->q[W] * xs;
    wy = this->q[W] * ys;
    wz = this->q[W] * zs;
    
    xx = this->q[X] * xs;
    xy = this->q[X] * ys;
    xz = this->q[X] * zs;
    
    yy = this->q[Y] * ys;
    yz = this->q[Y] * zs;
    zz = this->q[Z] * zs;
    
    mat.m[X][X] = 1 - (yy + zz);
    mat.m[X][Y] = xy - wz;
    mat.m[X][Z] = xz + wy;
    mat.m[X][W] = 0;
    
    mat.m[Y][X] = xy + wz;
    mat.m[Y][Y] = 1 - (xx + zz);
    mat.m[Y][Z] = yz - wx;
    mat.m[Y][W] = 0;
    
    mat.m[Z][X] = xz - wy;
    mat.m[Z][Y] = yz + wx;
    mat.m[Z][Z] = 1 - (xx + yy);
    mat.m[Z][W] = 0;
    
    mat.m[W][X] = 0;
    mat.m[W][Y] = 0;
    mat.m[W][Z] = 0;
    mat.m[W][W] = 1;
}


/******************************************************************************
Convert a rotation mat.mrix into a unit quaternion.  From Ken Shoemake.

******************************************************************************/
void
Quaternion::fromMatrix(Matrix4f &mat)
{
  int i,j,k;
  float tr,s;
  static int nxt[3] = {Y, Z, X};

  tr = mat.m[X][X] + mat.m[Y][Y] + mat.m[Z][Z];

  if (tr > 0) {
    s = sqrt (tr + 1);
    this->q[W] = s * 0.5;
    s = 0.5 / s;
    this->q[X] = (mat.m[Z][Y] - mat.m[Y][Z]) * s;
    this->q[Y] = (mat.m[X][Z] - mat.m[Z][X]) * s;
    this->q[Z] = (mat.m[Y][X] - mat.m[X][Y]) * s;
  }
  else {
    i = X;
    if (mat.m[Y][Y] > mat.m[X][X])
      i = Y;
    if (mat.m[Z][Z] > mat.m[i][i])
      i = Z;
    j = nxt[i];
    k = nxt[j];
    s = sqrt (1 + (mat.m[i][i] - (mat.m[j][j] + mat.m[k][k])));
    this->q[i] = s * 0.5;
    s = 0.5 / s;
    this->q[W] = (mat.m[k][j] - mat.m[j][k]) * s;
    this->q[j] = (mat.m[j][i] + mat.m[i][j]) * s;
    this->q[k] = (mat.m[k][i] + mat.m[i][k]) * s;
  }
}


void
Quaternion::normalize()
{
   float norm;
   norm = sqrt(this->q[X]*this->q[X] + this->q[Y]*this->q[Y]
	       +this->q[Z]*this->q[Z] + this->q[W]*this->q[W]);
   
   if (norm != 0) {
      norm = 1/norm;
      this->q[X] = norm*this->q[X];
      this->q[Y] = norm*this->q[Y];
      this->q[Z] = norm*this->q[Z];
      this->q[W] = norm*this->q[W];
   } else {
      /* Uh oh.  Do nothing? */
   }
}

void
Quaternion::toAxisAngle(float &angle, Vec3f &axis)
{
   float norm;

   angle = acos(this->q[W])*360/M_PI;
   norm = sqrt(this->q[X]*this->q[X]
		  + this->q[Y]*this->q[Y] + this->q[Z]*this->q[Z]);
   if (norm != 0) {
      norm = 1/norm;
      axis.setValue(this->q[X]*norm, 
		       this->q[Y]*norm, this->q[Z]*norm);
   } else {
      axis.setValue(0,0,1);
   }   
}

