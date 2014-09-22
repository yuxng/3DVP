/*
 *  features = compute_contextual_features(detections, centers)
 *
 *  Inputs:
 *
 *  detections: N X D double matrix where D(i,:) = [x1 y1 x2 y2 class score]
 *  centers: M X 1 double matrix of cluster centers
 *
 *  Outputs:
 *
 *  distances: N by N distance matrix
 */

#include "mex.h"
#include <math.h>

#define MAX(A,B) ((A) < (B) ? (B) : (A))

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  size_t num, nc;
  double neighbor = 0.1;
  double *distances, *detections, *centers, *features;

  num = mxGetM(prhs[0]);
  nc = mxGetM(prhs[1]);
  if(num == 0)
  {
    plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
    return;
  }

  // Error check
  if (mxIsDouble(prhs[0]) == false) 
    mexErrMsgTxt("Box is not double");
  if(mxGetNumberOfDimensions(prhs[0]) != 2)
    mexErrMsgTxt("Box is not two dimentions");

  // input
  detections = mxGetPr(prhs[0]);
  centers = mxGetPr(prhs[1]);

  // output
  plhs[0] = mxCreateDoubleMatrix(num, nc, mxREAL);
  features = mxGetPr(plhs[0]);

  distances = (double*)mxCalloc(num*num, sizeof(double));

  // bottom coordinate the detections
  double *x1 = detections + 0*num;
  double *y1 = detections + 1*num;
  double *x2 = detections + 2*num;
  double *y2 = detections + 3*num;
  double *cids = detections + 4*num;
  double *scores = detections + 5*num;
  double cxi, cyi, cxj, cyj;
  double dis;

  // compute the distances
  for(int i = 0; i < num; i++)
  {
    cxi = (x1[i] + x2[i]) / 2;
    cyi = (y1[i] + y2[i]) / 2;
    for(int j = i+1; j < num; j++)
    {
      cxj = (x1[j] + x2[j]) / 2;
      cyj = (y1[j] + y2[j]) / 2;
      dis = sqrt((cxi - cxj) * (cxi - cxj) + (cyi - cyj) * (cyi - cyj));

      distances[i*num + j] = dis;
      distances[j*num + i] = dis;
    }
  }

  // compute the features
  for(int i = 0; i < num; i++)
  {
    double w = x2[i] - x1[i];
    for(int j = 0; j < num; j++)
    {
      if(distances[j*num + i] < neighbor*w)
      {
        // find the center index
        int index = -1;
        for(int k = 0; k < nc; k++)
        {
          if(centers[k] == cids[j])
          {
            index = k;
            break;
          }
        }

        // max pooling
        features[index * num + i] = MAX(features[index * num + i], scores[j]);
      }
    }
  }

  mxFree(distances);
}
