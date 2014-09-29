#include <mex.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  const mwSize *dims;
  int i, j, k;
  int num_data, num_dim;
  double s;
  double *data, *similarity, *pi, *pj;

  /* parse input matrix */
  dims = mxGetDimensions(prhs[0]);
  num_dim = dims[0];
  num_data = dims[1];
  data = mxGetPr(prhs[0]);

  /* prepare for output */
  plhs[0] = mxCreateDoubleMatrix(num_data, num_data, mxREAL);
  similarity = mxGetPr(plhs[0]);

  /* compute similarity */
  for(i = 0; i < num_data; i++)
  {
    pi = data + i*num_dim;
    for(j = i+1; j < num_data; j++)
    {
      pj = data + j*num_dim;
      s = 0.0;
      for(k = 0; k < num_dim; k++)
      {
        s += (*(pi+k) - *(pj+k)) * (*(pi+k) - *(pj+k));
      }
      s *= -1;
      similarity[j*num_data+i] = s;
      similarity[i*num_data+j] = s;
    }
  }
}
