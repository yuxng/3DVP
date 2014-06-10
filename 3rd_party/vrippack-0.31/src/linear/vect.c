/*
 * vect:
 *	Functions to support operations on vectors and matrices.
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Much mucking with by:
 * Gavin Bell
 */
#include <stdio.h>
#include "vect.h"

float *
vnew()
{
	register float *v;

	v = (float *) malloc(sizeof(float)*3);
	return v;
}

float *
vclone(const float *v)
{
	register float *c;

	c = vnew();
	vcopy(v, c);
	return c;
}

void
vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0 ; i < 3 ; i++)
		v2[i] = v1[i];
}

void
vprint(const float *v)
{
	printf("x: %f y: %f z: %f\n",v[0],v[1],v[2]);
}

void
vset(float *v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void
vzero(float *v)
{
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 0.0;
}

void
vnormal(float *v)
{
	vscale(v,1.0/vlength(v));
}

float
vlength(const float *v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void
vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void
vmult(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] * src2[0];
	dst[1] = src1[1] * src2[1];
	dst[2] = src1[2] * src2[2];
}

void
vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void
vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void
vhalf(const float *v1, const float *v2, float *half)
{
	float len;

	vadd(v2,v1,half);
	len = vlength(half);
	if(len>0.0001)
		vscale(half,1.0/len);
	else
		vcopy(v1, half);
}

float
vdot(const float *v1, const float *v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void
vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

void
vdirection(const float *v1, float *dir)
{
	vcopy(v1, dir);
	vnormal(dir);
}

void
vreflect(const float *in, const float *mirror, float *out)
{
	float temp[3];

	vcopy(mirror, temp);
	vscale(temp,vdot(mirror,in));
	vsub(temp,in,out);
	vadd(temp,out,out);
}

void
vmultmatrix(const Matrix m1, const Matrix m2, Matrix prod)
{
	register int row, col;
	Matrix temp;

	for(row=0 ; row<4 ; row++) 
		for(col=0 ; col<4 ; col++)
			temp[row][col] = m1[row][0] * m2[0][col]
						   + m1[row][1] * m2[1][col]
						   + m1[row][2] * m2[2][col]
						   + m1[row][3] * m2[3][col];
	for(row=0 ; row<4 ; row++) 
		for(col=0 ; col<4 ; col++)
		prod[row][col] = temp[row][col];
}

void
vtransform(const float *v, const Matrix mat, float *vt)
{
	float t[3];

	t[0] = v[0]*mat[0][0] + v[1]*mat[1][0] + v[2]*mat[2][0] + mat[3][0];
	t[1] = v[0]*mat[0][1] + v[1]*mat[1][1] + v[2]*mat[2][1] + mat[3][1];
	t[2] = v[0]*mat[0][2] + v[1]*mat[1][2] + v[2]*mat[2][2] + mat[3][2];
	vcopy(t, vt);
}
void
vtransform4(const float *v, const Matrix mat, float *vt)
{
	float t[3];

	t[0] = v[0]*mat[0][0] + v[1]*mat[1][0] + v[2]*mat[2][0] + mat[3][0];
	t[1] = v[0]*mat[0][1] + v[1]*mat[1][1] + v[2]*mat[2][1] + mat[3][1];
	t[2] = v[0]*mat[0][2] + v[1]*mat[1][2] + v[2]*mat[2][2] + mat[3][2];
	vcopy(t, vt);
	t[3] = v[0]*mat[0][3] + v[1]*mat[1][3] + v[2]*mat[2][3] + mat[3][3];
	vt[3] = t[3];
}

Matrix idmatrix =
{
	{ 1.0, 0.0, 0.0, 0.0,},
	{ 0.0, 1.0, 0.0, 0.0,},
	{ 0.0, 0.0, 1.0, 0.0,},
	{ 0.0, 0.0, 0.0, 1.0,},
};

void
mcopy(const Matrix m1, Matrix m2)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			m2[i][j] = m1[i][j];
}

void
minvert(const Matrix mat, Matrix result)
{
	int i, j, k;
	double temp;
	double m[8][4];
	/*   Declare identity matrix   */

	mcopy(idmatrix, result);
	for (i = 0;  i < 4;  i++) {
		for (j = 0;  j < 4;  j++) {
			m[i][j] = mat[i][j];
			m[i+4][j] = result[i][j];
		}
	}

	/*   Work across by columns   */

	for (i = 0;  i < 4;  i++) {
		for (j = i;  (m[i][j] == 0.0) && (j < 4);  j++)
				;
		if (j == 4) {
			fprintf (stderr, "error:  cannot do inverse matrix\n");
			exit (2);
		} 
		else if (i != j) {
			for (k = 0;  k < 8;  k++) {
				temp = m[k][i];   
				m[k][i] = m[k][j];   
				m[k][j] = temp;
			}
		}

		/*   Divide original row   */

		for (j = 7;  j >= i;  j--)
			m[j][i] /= m[i][i];

		/*   Subtract other rows   */

		for (j = 0;  j < 4;  j++)
			if (i != j)
				for (k = 7;  k >= i;  k--)
					m[k][j] -= m[k][i] * m[i][j];
	}

	for (i = 0;  i < 4;  i++)
		for (j = 0;  j < 4;  j++)
				result[i][j] = m[i+4][j];
}

/*
 * Get combined Model/View/Projection matrix, in any mmode
 */
void
vgetmatrix(Matrix m)
{
	long mm;

	mm = getmmode();

	if (mm == MSINGLE)
	{
		getmatrix(m);
	}
	else
	{
		Matrix mp, mv;

		mmode(MPROJECTION);
		getmatrix(mp);
		mmode(MVIEWING);
		getmatrix(mv);

		pushmatrix();	/* Multiply them together */
		loadmatrix(mp);
		multmatrix(mv);
		getmatrix(m);
		popmatrix();

		mmode(mm);	/* Back into the mode we started in */
	}
}

/*
 * Gaussian Elimination with Scaled Column Pivoting
 *
 * copied out of the book by	    Wade Olsen
 *				    Silicon Graphics
 *				    Feb. 12, 1990
 */

void
linsolve(	
	const float *eqs[], 	/* System of eq's to solve */
	int		n, 		/* of size inmat[n][n+1] */
	float	*x		/* Result float *or of size x[n] */
)
{
	int		i, j, p;

	float **a;

	/* Allocate space to work in */
	/* (avoid modifying the equations passed) */
	a = (float **)malloc(sizeof(float *)*n);
	for (i = 0; i < n; i++)
	{
		a[i] = (float *)malloc(sizeof(float)*(n+1));
		bcopy(eqs[i], a[i], sizeof(float)*(n+1));
	}


	if (n == 1)
	{		/* The simple single variable case */
		x[0] = a[0][1] / a[0][0];
		return;
	}
				/* Gausian elimination process */	
	for (i = 0; i < n -1; i++)
	{

				/* find non-zero pivot row */
		p = i;
		while (a[p][i] == 0.0)
		{
			p++;
			if (p == n)
			{
				printf("linsolv:  No unique solution exists.\n");
				exit(1);
			}
		}
				/* row swap */
		if (p != i)
		{
			float *swap;

			swap = a[i];
			a[i] = a[p];
			a[p] = swap;
		}
				/* row subtractions */
		for (j = i + 1; j < n; j++)
		{
			float mult	= a[j][i] / a[i][i];

			int	k;
			for (k = i + 1; k < n + 1; k++)
				a[j][k] -= mult * a[i][k];
		}
	}

	if (a[n-1][n-1] == 0.0)
	{
		printf("linsolv:  No unique solution exists.\n");
		exit(1);
	}

					/* backwards substitution */
	x[n-1] = a[n-1][n] / a[n-1][n-1];
	for (i = n -2; i > -1; i--)
	{
		float sum = a[i][n];

		for (j = i + 1; j < n; j++)
			sum -= a[i][j] * x[j];

		x[i] = sum / a[i][i];
	}

	/* Free working space */
	for (i = 0; i < n; i++)
	{
		free(a[i]);
	}
	free(a);
}
