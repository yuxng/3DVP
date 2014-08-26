/*=================================================================
 *
 * stdout_withFlush.C	take a string and print it to stdout and flush 
 *
 * The calling syntax is:
 *
 *		stdout_withFlush(s)
 *
 * Yuanqing Lin, 09/20/2011
 *
 *=================================================================*/
#include <stdio.h>
#include "mex.h"


void mexFunction( int nlhs, mxArray *plhs[], 
		  int nrhs, const mxArray*prhs[] )
     
{ 
    char *str;
    (void) plhs;      /* unused parameter */

    /* Check for proper number of input and output arguments */    
    if (nrhs != 1) {
	mexErrMsgTxt("One input argument required.");
    } 
    if(nlhs > 1){
	mexErrMsgTxt("Too many output arguments.");
    }
    if (!(mxIsChar(prhs[0]))){
	mexErrMsgTxt("Input must be of type string.\n.");
    }
    
    /* The user passes a string in prhs[0]; write the string
       to the data file. NOTE: you must free str after it is used */ 
    str=mxArrayToString(prhs[0]);
    /*
	if ((size_t)fprintf(fp,"%s\n", str) != strlen(str) +1){
	mxFree(str);
   	mexErrMsgTxt("Could not write data to file.\n");
    }
    mexPrintf("Writing data to file.\n");
	*/
	fprintf(stderr,"%s\n",str);
	fflush(stderr);
    mxFree(str);
    
}


