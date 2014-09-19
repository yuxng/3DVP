/*
 *  maximize.cc
 *  [I,S] = maximize(Detections,PosScore,NegScore,Weights)
 *
 *  Inputs:
 *
 *  Detections: N X D matrix where D(i,:) = [x1 y1 x2 y2 class]
 *  PosScore: N x 1 matrix   where P(i)   = local score for turning on box i
 *  NegScore: N x 1 matrix   where N(i)   = local score for turning off box i
 *  Weights:  Vector of length Numclass*numClass*NumSpatFeatures
 *
 *  Outputs:
 *
 *  I: N by 1 matrix where I(i) = 1 if box i is turned on
 *  S: N by 1 matrix where S(i) = accumulated score of box i
 */

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
#define NUM_CLASS 21


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
   // Arguments
  double *box, *pos, *neg, *ws;
  
  // Return values; instanced detections and scores
  double *I, *S;
  
  int num, d;
  double cj;

  
  box = mxGetPr(prhs[0]);
  pos = mxGetPr(prhs[1]);
  neg = mxGetPr(prhs[2]);
  ws  = mxGetPr(prhs[3]);
  
  num = mxGetM(prhs[0]);
  d   = mxGetN(prhs[0]);

  // Error check
  if (mxIsDouble(prhs[0]) == false) 
    mexErrMsgTxt("Box is not double of size NxD");
  if (mxIsDouble(prhs[1]) == false || mxGetM(prhs[1]) != num) 
    mexErrMsgTxt("Pos is not double of size Nx1");
  if (mxIsDouble(prhs[2]) == false || mxGetM(prhs[2]) != num) 
    mexErrMsgTxt("Neg is not double of size Nx1");
  if (mxIsDouble(prhs[3]) == false || mxGetM(prhs[3]) != NUM_PAIR*NUM_CLASS*NUM_CLASS) 
    mexErrMsgTxt("W is not double of correct size");

  //printf("num=%d\n",num);

  
  // Initialize instance set "I" to all 0s and "S" to pos
  plhs[0] = mxDuplicateArray(prhs[1]);
  plhs[1] = mxCreateDoubleMatrix(num,1, mxREAL);
  S = mxGetPr(plhs[0]);
  I = mxGetPr(plhs[1]);
    
  //printf("I[0]=%g\n",I[0]);

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
  
  int numPos = 0;
  
  while (numPos < num) 
  {
    // Find highest scoring un-instanced box "i" and 
    // make sure it's positive score is better than negative score
    double best  = -1000000;
    int i = 0;
    for (int j = 0; j < num; j++) 
    {
      if (I[j]  == 0 && S[j] > best) 
      {
	      i    = j;
	      best = S[i];
      }
    }
    
    //printf("best=%g,neg=%g,i=%d\n",best,neg[i],i);
    
    if (best < neg[i]) break;
    
    // Turn i on
    I[i] = 1;
    numPos++;
    
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
    //printf("i=%d,bx=%g,by=%g,bw=%g,bh=%g,bc=%d\n",i,bxi,byi,bwi,bhi,bci);
    
    // Loop through all uninstanced boxes j
    for (int j=0; j < num; j++) 
    {
      if (I[j] == 0) 
      {
	
	      // Get statistics of box j
	      double bxj = bx[j];
	      double byj = by[j];
	      int    bcj = bc[j];	
	      double relx = ABS(bxj - bxi);
	      double rely = byj - byi;
	      double *wij =  ws + NUM_PAIR*bcj + NUM_PAIR*NUM_CLASS*bci;
	      double width  = bwi;
	      double height = bhi;

	//printf("j=%d,bx=%g,by=%g,bc=%d,relx=%g,rely=%g,width=%g,height=%g,ij=%d,S[j]=%g\n",
	///    j,bxj,byj,bcj,relx,rely,width,height,NUM_PAIR*bci + NUM_PAIR*NUM_CLASS*bcj,S[j]);
	
	// Are i and j far away
	if (relx > 1.5*width || rely > 1.5*height || rely < -1.5*height) 
  {
	  
	  S[j] += wij[FAR];
	  
	} else
  {
	  
	  S[j] += wij[NEAR];
	  //printf("S[j]=%g\n",S[j]);
	  
	  // Reset the width and height for near calculations
	  width  = 0.5 * width;
	  height = 0.5 * height;
	  
	  if (relx < width) {
	    if      (rely >  height) S[j] += wij[BELOW];
	    else if (rely > -height) S[j] += wij[ONTOP];
	    else                     S[j] += wij[ABOVE];
	  } 
	  else if (relx > width && rely < height && rely > -height) {
	    S[j] += wij[NEXT];
	  }
	  //printf("S[j]=%g\n",S[j]);
	  // Compute overlap
	  double iw = MIN(x2i,x2[j]) - MAX(x1i,x1[j]) + 1;
	  double ih = MIN(y2i,y2[j]) - MAX(y1i,y1[j]) + 1;
	  //printf("x2i=%g,,x2j=%g,x1i=%g,,x1j=%g,iw=%g,ih=%g \n",x2i,x2[j],x1i,x1[j],iw,ih);
	  if (iw > 0 && ih > 0) {
	    double ov = iw*ih  / (bai + ba[j] - iw*ih);
	    //printf("ov=%g,bai=%g,ba=%g \n",ov,bai,ba[j]);
	    if (ov > .5) {
	      S[j] += wij[OVER];
	    }
	  }	  
	  //printf("S[j]=%g\n",S[j]);
	}

	/* Add in contribution from wji*/
	rely = -rely;
	wij  =  ws + NUM_PAIR*bci + NUM_PAIR*NUM_CLASS*bcj;
	width  = bw[j];
	height = bh[j];

	//printf("j=%d,bx=%g,by=%g,bc=%d,relx=%g,rely=%g,width=%g,height=%g,ij=%d,S[j]=%g\n",
	//j,bxj,byj,bcj,relx,rely,width,height,NUM_PAIR*bcj + NUM_PAIR*NUM_CLASS*bci,S[j]);
      
	// Are i and j far away
	if (relx > 1.5*width || rely > 1.5*height || rely < -1.5*height)
  {

	  S[j] += wij[FAR];	  

	} else
  {
	  
	  S[j] += wij[NEAR];
	  
	  // Half the width and height for near calculations
	  width  = 0.5 * width;
	  height = 0.5 * height;
	  
	  if (relx < width) {
	    if      (rely >  height) S[j] += wij[BELOW];
	    else if (rely > -height) S[j] += wij[ONTOP];
	    else                     S[j] += wij[ABOVE];
	  } 
	  else if (relx > width && rely < height && rely > -height) {
	    S[j] += wij[NEXT];
	  }

	  // Compute overlap
	  double iw = MIN(x2i,x2[j]) - MAX(x1i,x1[j]) + 1;
	  double ih = MIN(y2i,y2[j]) - MAX(y1i,y1[j]) + 1;
	  if (iw > 0 && ih > 0) {
	    double ov = iw*ih  / (bai + ba[j] - iw*ih);
	    if (ov > .5) {
	      S[j] += wij[OVER];
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

