/*
 *  computeFeature.cc
 *  [P,L] = computeFeature(Detections,Scores)
 *
 *  Inputs:
 *
 *  Detections: N x D matrix where D(i,:) = [x1 y1 x2 y2 class]
 *  Scores:     N x 1 matrix where S(i) is the score of the ith box 
 *
 *  Assumes all detections are turned on:
 *    To use given index set "I" from maximize, 
 *    pass in Detections(find(I),:) and Scores(find(I),:)
 *
 *  Outputs:
 *
 *  P: Pairwise feature vector
 *  L: Unary feature vector
 */

#include <math.h>
#include <sys/types.h>
#include "mex.h"

#define ABS(A) ((A) < 0 ? -(A) : (A))
#define MIN(A,B) ((A) > (B) ? (B) : (A))
#define MAX(A,B) ((A) < (B) ? (B) : (A))

#define ONTOP 0
#define ABOVE 1
#define BELOW 2
#define NEXT  3
#define NEAR  4
#define FAR   5
#define OVER  6
#define NUM_PAIR  7 
#define NUM_LOCAL 2
#define NUM_CLASS 21


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   // Arguments
  double *box, *pos, *neg, *ws;
  
  // Return values; instanced detections and scores
  double *I, *S;
  
  int num, d, k;
  double cj;

  
  box = mxGetPr(prhs[0]);
  S   = mxGetPr(prhs[1]);
  
  num = mxGetM(prhs[0]);
  d   = mxGetN(prhs[0]);

  // Error check
  if (mxIsDouble(prhs[0]) == false) 
    mexErrMsgTxt("Box is not double of size NxD");
  if (mxIsDouble(prhs[1]) == false || mxGetM(prhs[1]) != num) 
    mexErrMsgTxt("S is not double of size Nx1");
  

  // Initialize output unary and pairwise features to all 0's
  plhs[0] = mxCreateDoubleMatrix(NUM_PAIR*NUM_CLASS*NUM_CLASS, 1, mxREAL);
  plhs[1] = mxCreateDoubleMatrix(NUM_LOCAL*NUM_CLASS, 1, mxREAL);
  double *P = mxGetPr(plhs[0]);
  double *L = mxGetPr(plhs[1]);

  // Pre-compute centers, widths, heights, and areas of boxes
  double *x1 = box+0*num;
  double *y1 = box+1*num;
  double *x2 = box+2*num;
  double *y2 = box+3*num;
  double *cl = box+4*num;
  int    *bc = (int *)mxCalloc(num, sizeof(int));
  double *bx = (double *)mxCalloc(num, sizeof(double));
  double *by = (double *)mxCalloc(num, sizeof(double));
  double *bw = (double *)mxCalloc(num, sizeof(double));
  double *bh = (double *)mxCalloc(num, sizeof(double));
  double *ba = (double *)mxCalloc(num, sizeof(double));
  for (int i = 0; i < num; i++) {
    // Fix matlab 0-1 array indexing
    bc[i] = (int)cl[i] - 1;
    bx[i] = 0.5*(x1[i] + x2[i]);
    by[i] = 0.5*(y1[i] + y2[i]);
    bw[i] = x2[i] - x1[i] + 1;
    bh[i] = y2[i] - y1[i] + 1;
    ba[i] = bw[i]*bh[i];
  }

  for (int i=0; i<num; i++) {
    // Store statistics of box i
    // corners, center, width, height, area, class
    double x1i = x1[i];
    double y1i = y1[i];
    double x2i = x2[i];
    double y2i = y2[i];
    double bxi = bx[i];
    double byi = by[i];
    double bwi = bw[i];
    double bhi = bh[i];
    double bai = ba[i];
    int    bci = bc[i];

    // Update local scores
    L[NUM_LOCAL*bci]   += S[i];
    L[NUM_LOCAL*bci+1] += 1;
    
    // printf("i=%d,bx=%g,by=%g,bw=%g,bh=%g,bc=%d\n",i,bxi,byi,bwi,bhi,bci);

    // Update pairwise score
    for (int j=0; j<num; j++) {
      if (i != j) {
	// Get statistics of box j
	double bxj = bx[j];
	double byj = by[j];
	int    bcj = bc[j];	
	
	// Get relative centers ignoring sign of x because of left/right symmetry
	double relx = ABS(bxj - bxi);
	double rely = byj - byi;
	double *Pij = P + NUM_PAIR*bcj + NUM_PAIR*NUM_CLASS*bci;
	double width  = bwi;
	double height = bhi;
	
	//printf("j=%d,bx=%g,by=%g,bc=%d,relx=%g,rely=%g,width=%g,height=%g,ij=%d\n",
	//     j,bxj,byj,bcj,relx,rely,width,height,NUM_PAIR*bci + NUM_PAIR*NUM_CLASS*bcj);
	
	// Are i and j far away
	if (relx > 1.5*width || rely > 1.5*height || rely < -1.5*height) {
	  Pij[FAR]++;
	} else {
	  Pij[NEAR]++;
	  
	  // Reset the width and height for near calculations
	  width  = 0.5 * width;
	  height = 0.5 * height;
	  
	  if (relx < width) {
	    if      (rely >  height) Pij[BELOW]++;
	    else if (rely > -height) Pij[ONTOP]++;
	    else                     Pij[ABOVE]++;
	  } 
	  if (relx > width && rely < height && rely > -height) {
	    Pij[NEXT]++;
	  }
	  
	  double iw = MIN(x2i,x2[j]) - MAX(x1i,x1[j]) + 1;
	  double ih = MIN(y2i,y2[j]) - MAX(y1i,y1[j]) + 1;
	  if (iw > 0 && ih > 0) {
	    double ov = iw*ih  / (bai + ba[j] - iw*ih);
	    if (ov > .5) {
	      Pij[OVER]++;
	    }
	  }
	}
      }
    }
  }
  mxFree(bc);
  mxFree(bx);
  mxFree(by);
  mxFree(bw);
  mxFree(bh);
  mxFree(ba);  
  return;
}

