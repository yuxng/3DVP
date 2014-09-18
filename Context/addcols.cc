#include <math.h>
#include <stdint.h>
#include "mex.h"
#include "matrix.h"


#define MAX(A,B) ((A) < (B) ? (B) : (A))

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{

  if (mxIsSingle(prhs[0]) == false) mexErrMsgTxt("Arguement 1 is not single");
  if (mxIsDouble(prhs[1]) == false) mexErrMsgTxt("Arguement 2 is not double");
  
  float  *X = (float  *)mxGetPr(prhs[0]);
  double *I = (double *)mxGetPr(prhs[1]);
  
  const uint32_t m = mxGetM(prhs[0]);
  const uint32_t l = MAX(mxGetN(prhs[1]),mxGetM(prhs[1]));

  mxArray *mxY = mxCreateDoubleMatrix(m,1,mxREAL);
  double  *Y   = (double *)mxGetPr(mxY);

  for (int i = 0; i < l; i++) {    
    float *v = X + m * (uint32_t)(*(I+i)-1);
    //printf("%d %d,%f",m,(uint32_t)(*(I+i)-1),*v);
    double* yi = Y;
    for (int j = 0; j < m; j++) {
      *(yi++) += (double)*v;
      v++;
    }
  }
  plhs[0] = mxY;
  return;
}

/*
n = 10000;
x = single(rand(n));
I = randperm(n);
I = I(1:n/5);
mask = logical(zeros(n,1));
mask(I) = 1;

tic; res = addcols(x,I); toc;

tic; res2 = double(x)*mask; toc;

tic; res3 = sum(double(x(:,I)),2); toc;
*/
