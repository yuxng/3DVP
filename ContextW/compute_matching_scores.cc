/*
 *  [Scores, Overlaps] = compute_matching_scores(Detections, Patterns)
 *
 *  Inputs:
 *
 *  Detections: N X D double matrix where D(i,:) = [x1 y1 x2 y2 class]
 *  Patterns: height x width x N unit8 matrix for occlusion patterns
 *
 *  Outputs:
 *
 *  Scores: N by N matching score matrix
 *  Overlaps: N by N matching overlap matrix
 */

#include "mex.h"

#define MIN(A,B) ((A) > (B) ? (B) : (A))
#define MAX(A,B) ((A) < (B) ? (B) : (A))

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  size_t num, height, width;
  const mwSize *dims;
  double *scores1, *scores2, *detections;
  unsigned char *patterns;

  num = mxGetM(prhs[0]);
  if(num == 0)
  {
    plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(0, 0, mxREAL);
    return;
  }
  else if(num == 1)
  {
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
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
  scores1 = mxGetPr(plhs[0]);
  plhs[1] = mxCreateDoubleMatrix(num, num, mxREAL);
  scores2 = mxGetPr(plhs[1]);

  // bottom coordinate the detections
  double *x1 = detections + 0*num;
  double *y1 = detections + 1*num;
  double *x2 = detections + 2*num;
  double *y2 = detections + 3*num;

  // compute the area for each pattern
  double *area1 = (double*)mxCalloc(num, sizeof(double));
  double *area2 = (double*)mxCalloc(num, sizeof(double));
  for(int i = 0; i < num; i++)
  {
    unsigned char *p = patterns + i*height*width;
    int count1 = 0;
    int count2 = 0;
    for(int x = x1[i]-1; x < x2[i]; x++)
    {
      for(int y = y1[i]-1; y < y2[i]; y++)
      {
        if(p[x*height + y] == 1 || p[x*height + y] == 2)
          count1++;
        if(p[x*height + y] > 0)
          count2++;
      }
    }
    area1[i] = count1;
    area2[i] = count2;
  }

  // compute the matching scores
  for(int i = 0; i < num; i++)
  {
    for(int j = i+1; j < num; j++)
    {
      double s1 = 0;
      double s2 = 0;

      // compute bounding box overlap
  	  double w = MIN(x2[i], x2[j]) - MAX(x1[i], x1[j]) + 1;
	    double h = MIN(y2[i], y2[j]) - MAX(y1[i], y1[j]) + 1;
      if (w > 0 && h > 0)
      {
        unsigned char *pi = patterns + i*height*width;  // near object
        unsigned char *pj = patterns + j*height*width;  // far object

        if(y2[i] < y2[j])  // object i is occluded
        {
          pi = patterns + j*height*width;
          pj = patterns + i*height*width;
        }

        // compute the overlap and occluded areas
        double overlap_visibility = 0;
        double overlap_occlusion = 0;
        double oj = 0;
        int xmin = MIN(x1[i], x1[j]);
        int xmax = MAX(x2[i], x2[j]);
        int ymin = MIN(y1[i], y1[j]);
        int ymax = MAX(y2[i], y2[j]);
        for(int x = xmin-1; x < xmax; x++)
        {
          for(int y = ymin-1; y < ymax; y++)
          {
            if((pi[x*height+y] == 1 && pj[x*height+y] == 1) || (pi[x*height+y] == 2 && pj[x*height+y] == 2))
              overlap_visibility++;

            if(pi[x*height+y] == 1 && pj[x*height+y] == 2)
              overlap_occlusion++;
          }
        }

        s1 = overlap_visibility / (area1[i] + area1[j] - overlap_visibility);

        if(y2[i] < y2[j])  // object i is occluded
          s2 = overlap_occlusion / area2[i];
        else
          s2 = overlap_occlusion / area2[j];
      }

      // assign the value
      scores1[i*num + j] = s1;
      scores1[j*num + i] = s1;
      scores2[i*num + j] = s2;
      scores2[j*num + i] = s2;
    }
  }

  mxFree(area1); 
  mxFree(area2); 
}
