/*
 *  Scores = compute_matching_scores(Detections, Patterns)
 *
 *  Inputs:
 *
 *  Detections: N X D double matrix where D(i,:) = [x1 y1 x2 y2 class]
 *  Patterns: height x width x N unit8 matrix for occlusion patterns
 *
 *  Outputs:
 *
 *  Scores: N by N matching score matrix
 */

#include "mex.h"

#define MIN(A,B) ((A) > (B) ? (B) : (A))
#define MAX(A,B) ((A) < (B) ? (B) : (A))

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  size_t num, d, height, width;
  const mwSize *dims;
  double *scores, *detections;
  unsigned char *patterns;

  num = mxGetM(prhs[0]);
  if(num == 0)
  {
    plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
    return;
  }
  else if(num == 1)
  {
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
    return;
  }

  // Error check
  if (mxIsDouble(prhs[0]) == false) 
    mexErrMsgTxt("Box is not double");
  if (mxIsClass(prhs[1], "uint8") == false) 
    mexErrMsgTxt("Pattern is not uint8");
  if(mxGetNumberOfDimensions(prhs[0]) != 2)
    mexErrMsgTxt("Box is not two dimentions");
  if(mxGetNumberOfDimensions(prhs[1]) != 3)
    mexErrMsgTxt("Pattern is not three dimentions");

  d = mxGetN(prhs[0]);
  dims = mxGetDimensions(prhs[1]);
  height = dims[0];
  width = dims[1];
  if(dims[2] != num)
    mexErrMsgTxt("Pattern and detection not matched");

  // input
  detections = mxGetPr(prhs[0]);
  patterns = (unsigned char *)mxGetData(prhs[1]);

  // output
  plhs[0] = mxCreateDoubleMatrix(num, num, mxREAL);
  scores = mxGetPr(plhs[0]);

  // bottom coordinate the detections
  double *x1 = detections + 0*num;
  double *y1 = detections + 1*num;
  double *x2 = detections + 2*num;
  double *y2 = detections + 3*num;

  // compute the area for each pattern
  double *area = (double*)mxCalloc(num, sizeof(double));
  for(int i = 0; i < num; i++)
  {
    unsigned char *p = patterns + i*height*width;
    int count = 0;
    for(int x = x1[i]-1; x < x2[i]; x++)
    {
      for(int y = y1[i]-1; y < y2[i]; y++)
      {
        if(p[x*height + y] > 0)
          count++;
      }
    }
    area[i] = count;
  }

  // compute the matching score
  for(int i = 0; i < num; i++)
  {
    for(int j = i+1; j < num; j++)
    {
      double score = 0;

      // compute bounding box overlap
  	  double w = MIN(x2[i], x2[j]) - MAX(x1[i], x1[j]) + 1;
	    double h = MIN(y2[i], y2[j]) - MAX(y1[i], y1[j]) + 1;
      if (w > 0 && h > 0)
      {
        unsigned char *pi = patterns + i*height*width;
        unsigned char *pj = patterns + j*height*width;

        // compute the overlap and occluded areas
        double overlap = 0;
        double oi = 0;
        double oj = 0;
        int xmin = MIN(x1[i], x1[j]);
        int xmax = MAX(x2[i], x2[j]);
        int ymin = MIN(y1[i], y1[j]);
        int ymax = MAX(y2[i], y2[j]);
        for(int x = xmin-1; x < xmax; x++)
        {
          for(int y = ymin-1; y < ymax; y++)
          {
            if(pi[x*height+y] > 0 && pj[x*height+y] > 0)
            {
              overlap++;
              if(pi[x*height+y] == 2)
                oi++;
              if(pj[x*height+y] == 2)
                oj++;
            }
          }
        }

        double ratio;
        if(y2[i] < y2[j])  // object i is occluded
        {
          ratio = overlap / area[i];
          if(overlap)
            score = oi / overlap;
        }
        else
        {
          ratio = overlap / area[j];
          if(overlap)
            score = oj / overlap;
        }
      }

      // assign the value
      scores[i*num + j] = score;
      scores[j*num + i] = score;
    }
  }

  mxFree(area); 
}
