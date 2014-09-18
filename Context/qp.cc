/* -----------------------------------------------------------------------
 * qp.c:
 *                                                                     
 * Synopsis:                                                                     
 * [x,QP,QD,exitflag,nIter] = qp(H,f,b,uint32(I),uint8(S),x0,MaxIter,TolAbs,TolRel,QP_TH,verb)
 *
 * Compile:
 *  mex qp.c
 *
 * DESCRIPTION
 *  The library provides function which solves the following instance of
 *  a convex Quadratic Programmin task:
 *  
 *   min QP(x):= 0.5*x'*H*x + f'*x  
 *    x
 *
 * subject to:   
 *   sum_{i in I_k} x[i] == b[k]  for all k such that S[k] == 0 
 *   sum_{i in I_k} x[i] <= b[k]  for all k such that S[k] == 1
 *                             x(i) >= 0 for all i=1:n
 *   
 *  where I_k = { i | I[i] == k}, k={1,...,m}.
 *
 * A precision of the found solution is controled by the input argumens
 * MaxIter, TolAbs, QP_TH and MaxIter which define the stopping conditions:
 * 
 *  nIter >= MaxIter     ->  exitflag = 0   Number of iterations
 *  QP-QD <= TolAbs      ->  exitflag = 1   Abs. tolerance (duality gap)
 *  QP-QD <= QP*TolRel   ->  exitflag = 2   Relative tolerance
 *  QP <= QP_TH          ->  exitflag = 3   Threshold on objective value
 *
 * where QP and QD are primal respectively dual objective values.
 *
 * INPUT ARGUMENTS
 *  get_col   function which returns pointer to the i-th column of H.
 *  diag_H [double n x 1] vector containing values on the diagonal of H.
 *  f [double n x 1] vector.
 *  b [double n x 1] vector of positive numbers.
 *  I [uint16_T n x 1] vector containing numbers 1...m. 
 *  S [uint8_T n x 1] vector containing numbers 0 and 1.
 *  x [double n x 1] solution vector; must be feasible.
 *  n [uint32_t 1 x 1] dimension of H.
 *  MaxIter [uint32_t 1 x 1] max number of iterations.
 *  TolAbs [double 1 x 1] Absolute tolerance.
 *  TolRel [double 1 x 1] Relative tolerance.
 *  QP_TH  [double 1 x 1] Threshold on the primal value.
 *  print_state  print function; if == NULL it is not called.
 *
 * RETURN VALUE
 *  structure [libqp_state_T] 
 *  .QP [1 x 1] Primal objective value.
 *  .QD [1 x 1] Dual objective value.
 *  .nIter [1 x 1] Number of iterations.
 *  .exitflag [1 x 1] Indicates which stopping condition was used:
 *    -1  ... Not enough memory.
 *     0  ... Maximal number of iteations reached: nIter >= MaxIter.
 *     1  ... Relarive tolerance reached: QP-QD <= abs(QP)*TolRel
 *     2  ... Absolute tolerance reached: QP-QD <= TolAbs
 *     3  ... Objective value reached threshold: QP <= QP_TH.
 *
 * REFERENCE
 *  The algorithm is described in:
 *  V. Franc, V. Hlavac. A Novel Algorithm for Learning Support Vector Machines
 *   with Structured Output Spaces. Research Report K333 22/06, CTU-CMP-2006-04. 
 *   May, 2006. ftp://cmp.felk.cvut.cz/pub/cmp/articles/franc/Franc-TR-2006-04.ps
 *
 * Copyright (C) 2006-2008 Vojtech Franc, xfrancv@cmp.felk.cvut.cz
 * Center for Machine Perception, CTU FEL Prague
 *-------------------------------------------------------------------- */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "mex.h"
#include "matrix.h"

#define LIBQP_PLUS_INF mxGetInf()
#define LIBQP_CALLOC(x,y) mxCalloc(x,y)
#define LIBQP_FREE(x) mxFree(x)
#define LIBQP_INDEX(ROW,COL,NUM_ROWS) ((COL)*(NUM_ROWS)+(ROW))
#define LIBQP_MIN(A,B) ((A) > (B) ? (B) : (A))
#define LIBQP_MAX(A,B) ((A) < (B) ? (B) : (A))
#define LIBQP_ABS(A) ((A) < 0 ? -(A) : (A))

/* QP solver return value */
typedef struct {
  uint32_t nIter;       /* number of iterations */ 
  double QP;            /* primal objective value */ 
  double QD;            /* dual objective value */  
  int8_t exitflag;      /* -1 ... not enough memory 
                            0 ... nIter >= MaxIter 
                            1 ... QP - QD <= TolRel*ABS(QP)
                            2 ... QP - QD <= TolAbs
                            3 ... QP <= QP_TH
                            4 ... eps-KKT conditions satisfied */
} libqp_state_T; 

/* -- Global variables --------------------------------------*/

double   *mat_H;     /* pointer to the Hessian matrix [n x n] */
uint32_t nVar;       /* number of ariables */


/* ------------------------------------------------------------
  Returns pointer at the i-th column of the Hessian matrix H.
------------------------------------------------------------ */
const double *get_col( uint32_t i )
{
  return( &mat_H[ nVar*i ] );
}


libqp_state_T libqp_splx_solver(const double* (*get_col)(uint32_t),
                  double *diag_H,
                  double *f,
                  double *b,
                  uint32_t *I,
                  uint8_t *S,
                  double *x,
                  uint32_t n,
                  uint32_t MaxIter,
                  double TolAbs,
                  double TolRel,
                  double QP_TH,
				  void (*print_state)(libqp_state_T state))
{
  double *d;
  double *col_u, *col_v;
  double *x_neq;
  double tmp;
  double improv;
  double tmp_num;
  double tmp_den=0;
  double tau=0;
  double delta;
  double yu;
  uint32_t *inx;
  uint32_t *nk;
  uint32_t m;
  uint32_t u=0;
  uint32_t v=0;
  uint32_t k;
  uint32_t i, j;
  libqp_state_T state;

  
  /* ------------------------------------------------------------ 
    Initialization                                               
  ------------------------------------------------------------ */
  state.nIter = 0;

  inx=NULL;
  nk=NULL;
  d=NULL;
  x_neq = NULL;

  /* count number of constraints */
  for( i=0, m=0; i < n; i++ ) 
    m = LIBQP_MAX(m,I[i]);

  /* auxciliary variables for tranforming equalities to inequalities */
  x_neq = (double*) LIBQP_CALLOC(m, sizeof(double));
  if( x_neq == NULL )
  {
	  state.exitflag=-1;
	  goto cleanup;
  }

  /* inx is translation table between variable index i and its contraint */
  inx = (uint32_t*) LIBQP_CALLOC(m*n, sizeof(uint32_t));
  if( inx == NULL )
  {
	  state.exitflag=-1;
	  goto cleanup;
  }

  /* nk is the number of variables coupled by i-th linear constraint */
  nk = (uint32_t*) LIBQP_CALLOC(m, sizeof(uint32_t));
  if( nk == NULL )
  {
	  state.exitflag=-1;
	  goto cleanup;
  }

  /* setup auxciliary variables */
  for( i=0; i < m; i++ ) 
    x_neq[i] = b[i];


  /* create inx and nk */
  for( i=0; i < n; i++ ) {
     k = I[i]-1;
     inx[LIBQP_INDEX(nk[k],k,n)] = i;
     nk[k]++;     

     if(S[k] != 0) 
       x_neq[k] -= x[i];
  }
    
  /* d = H*x + f is gradient*/
  d = (double*) LIBQP_CALLOC(n, sizeof(double));
  if( d == NULL )
  {
	  state.exitflag=-1;
	  goto cleanup;
  }
 
  /* compute gradient */
  for( i=0; i < n; i++ ) 
  {
    d[i] += f[i];
    if( x[i] > 0 ) {
      col_u = (double*)get_col(i);      
      for( j=0; j < n; j++ ) {
          d[j] += col_u[j]*x[i];
      }
    }
  }
  
  /* compute state.QP = 0.5*x'*(f+d);
             state.QD = 0.5*x'*(f-d); */
  for( i=0, state.QP = 0, state.QD=0; i < n; i++) 
  {
    state.QP += x[i]*(f[i]+d[i]);
    state.QD += x[i]*(f[i]-d[i]);
  }
  state.QP = 0.5*state.QP;
  state.QD = 0.5*state.QD;
  
  for( i=0; i < m; i++ ) 
  {
    for( j=0, tmp = LIBQP_PLUS_INF; j < nk[i]; j++ ) 
      tmp = LIBQP_MIN(tmp, d[inx[LIBQP_INDEX(j,i,n)]]);

    if(S[i] == 0) 
      state.QD += b[i]*tmp;
    else
      state.QD += b[i]*LIBQP_MIN(tmp,0);
  }
  
  /* print initial state */
  if( print_state != NULL) 
    print_state( state );

  /* ------------------------------------------------------------ 
    Main optimization loop 
  ------------------------------------------------------------ */
  state.exitflag = 100;
  while( state.exitflag == 100 ) 
  {
    state.nIter ++;

    /* go over blocks of variables coupled by lin. constraint */
    for( k=0; k < m; k++ ) 
    {       
        
      /* compute u = argmin_{i in I_k} d[i] 
             delta =  sum_{i in I_k} x[i]*d[i] - b*min_{i in I_k} */
      for( j=0, tmp = LIBQP_PLUS_INF, delta = 0; j < nk[k]; j++ ) 
      {
        i = inx[LIBQP_INDEX(j,k,n)];
        delta += x[i]*d[i];
        if( tmp > d[i] ) {
          tmp = d[i];
          u = i;
        }
      }

      if(S[k] != 0 && d[u] > 0) 
        u = -1;
      else
        delta -= b[k]*d[u];
            
      /* if satisfied then k-th block of variables needs update */
      if( delta > TolAbs/m && delta > TolRel*LIBQP_ABS(state.QP)/m) 
      {         
        /* for fixed u select v = argmax_{i in I_k} Improvement(i) */
        if( u != -1 ) 
        {
          col_u = (double*)get_col(u);
          improv = -LIBQP_PLUS_INF;
          for( j=0; j < nk[k]; j++ ) 
          {
            i = inx[LIBQP_INDEX(j,k,n)];
           
            if(x[i] > 0 && i != u) 
            {
              tmp_num = x[i]*(d[i] - d[u]); 
              tmp_den = x[i]*x[i]*(diag_H[u] - 2*col_u[i] + diag_H[i]);
              if( tmp_den > 0 ) 
              {
                if( tmp_num < tmp_den ) 
                  tmp = tmp_num*tmp_num / tmp_den;
                else 
                  tmp = tmp_num - 0.5 * tmp_den;
                 
                if( tmp > improv ) 
                { 
                  improv = tmp;
                  tau = LIBQP_MIN(1,tmp_num/tmp_den);
                  v = i;
                } 
              }
            }
          }

          /* check if virtual variable can be for updated */
          if(x_neq[k] > 0 && S[k] != 0) 
          {
            tmp_num = -x_neq[k]*d[u]; 
            tmp_den = x_neq[k]*x_neq[k]*diag_H[u];
            if( tmp_den > 0 ) 
            {
              if( tmp_num < tmp_den ) 
                tmp = tmp_num*tmp_num / tmp_den;
              else 
                tmp = tmp_num - 0.5 * tmp_den;
                 
              if( tmp > improv ) 
              { 
                improv = tmp;
                tau = LIBQP_MIN(1,tmp_num/tmp_den);
                v = -1;
              } 
            }
          }

          /* minimize objective w.r.t variable u and v */
          if(v != -1)
          {
            tmp = x[v]*tau;
            x[u] += tmp;
            x[v] -= tmp;

            /* update d = H*x + f */
            col_v = (double*)get_col(v);
            for(i = 0; i < n; i++ )              
              d[i] += tmp*(col_u[i]-col_v[i]);
          }
          else
          {
            tmp = x_neq[k]*tau;
            x[u] += tmp;
            x_neq[k] -= tmp;

            /* update d = H*x + f */
            for(i = 0; i < n; i++ )              
              d[i] += tmp*col_u[i];
          }
        }
        else
        {
          improv = -LIBQP_PLUS_INF;
          for( j=0; j < nk[k]; j++ ) 
          {
            i = inx[LIBQP_INDEX(j,k,n)];
           
            if(x[i] > 0) 
            {
              tmp_num = x[i]*d[i]; 
              tmp_den = x[i]*x[i]*diag_H[i];
              if( tmp_den > 0 ) 
              {
                if( tmp_num < tmp_den ) 
                  tmp = tmp_num*tmp_num / tmp_den;
                else 
                  tmp = tmp_num - 0.5 * tmp_den;
                 
                if( tmp > improv ) 
                { 
                  improv = tmp;
                  tau = LIBQP_MIN(1,tmp_num/tmp_den);
                  v = i;
                } 
              }
            }
          }

          tmp = x[v]*tau;
          x_neq[k] += tmp;
          x[v] -= tmp;

          /* update d = H*x + f */
          col_v = (double*)get_col(v);
          for(i = 0; i < n; i++ )              
            d[i] -= tmp*col_v[i];
        }

        /* update objective value */
        state.QP = state.QP - improv;
      }
    }
    
    /* Compute primal and dual objectives */
    for( i=0, state.QP = 0, state.QD=0; i < n; i++) 
    {
       state.QP += x[i]*(f[i]+d[i]);
       state.QD += x[i]*(f[i]-d[i]);
    }
    state.QP = 0.5*state.QP;
    state.QD = 0.5*state.QD;

    for( k=0; k < m; k++ ) 
    { 
      for( j=0,tmp = LIBQP_PLUS_INF; j < nk[k]; j++ ) {
        i = inx[LIBQP_INDEX(j,k,n)];
        tmp = LIBQP_MIN(tmp, d[i]);
      }
      
      if(S[k] == 0) 
        state.QD += b[k]*tmp;
      else
        state.QD += b[k]*LIBQP_MIN(tmp,0);
    }

    /* print state */
    if( print_state != NULL) 
      print_state( state );

    /* check stopping conditions */
    if(state.QP-state.QD <= LIBQP_ABS(state.QP)*TolRel ) state.exitflag = 1;
    else if( state.QP-state.QD <= TolAbs ) state.exitflag = 2;
    else if( state.QP <= QP_TH ) state.exitflag = 3;
    else if( state.nIter >= MaxIter) state.exitflag = 0;
  }

  /*----------------------------------------------------------
    Clean up
  ---------------------------------------------------------- */
cleanup:
  LIBQP_FREE( d );
  LIBQP_FREE( inx );
  LIBQP_FREE( nk );
  
  return( state ); 
}


/* -------------------------------------------------------------
  MEX main function.
------------------------------------------------------------- */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
  int verb;          
  uint32_t MaxIter;
  uint32_t *vec_I; 
  uint32_t nConstr; 
  uint8_t *vec_S;
  double *vec_x;         
  double TolRel;     
  double TolAbs;     
  double QP_TH;
  double *vec_x0;        
  double *vec_f;         
  double *vec_b;
  double *diag_H;    
  long i ;           
  libqp_state_T state;
 
  /*------------------------------------------------------------------- 
     Get input arguments
   ------------------------------------------------------------------- */
  if( nrhs < 11) mexErrMsgTxt("Incorrect number of input arguments.");
  if (mxIsUint32(prhs[3]) == false) mexErrMsgTxt("Arguement I is not uint32");
  if (mxIsUint8 (prhs[4]) == false) mexErrMsgTxt("Argument  S is not uint8");

  mat_H = mxGetPr(prhs[0]);
  nVar  = mxGetM(prhs[0]);
  vec_f = mxGetPr(prhs[1]);
  vec_b = (double*)mxGetPr(prhs[2]);
  vec_I = (uint32_t*)mxGetPr(prhs[3]);
  vec_S = (uint8_t*)mxGetPr(prhs[4]);
  
  nConstr = LIBQP_MAX(mxGetN(prhs[2]),mxGetM(prhs[2]));
  vec_x0  = mxGetPr(prhs[5]);
  MaxIter = mxIsInf( mxGetScalar(prhs[6])) ? 0xFFFFFFFF : (uint32_t)mxGetScalar(prhs[6]);
  TolAbs  = mxGetScalar(prhs[7]);   
  TolRel  = mxGetScalar(prhs[8]);   
  QP_TH   = mxGetScalar(prhs[9]);   
  verb    = (int)mxGetScalar(prhs[10]);  
  
  /* print input setting if required */  
  if( verb > 0 ) {
    mexPrintf("Settings of LIBQP_SSVM solver:\n");
    mexPrintf("MaxIter  : %u\n", MaxIter );
    mexPrintf("TolAbs   : %f\n", TolAbs );
    mexPrintf("TolRel   : %f\n", TolRel );
    mexPrintf("QP_TH    : %f\n", QP_TH );
    mexPrintf("nVar     : %u\n", nVar );
    mexPrintf("nConstr  : %u\n", nConstr );
  }     
  
  /*------------------------------------------------------------------- 
     Inicialization                                                     
   ------------------------------------------------------------------- */

  /* create solution vector x [nVar x 1] */
  plhs[0] = mxCreateDoubleMatrix(nVar,1,mxREAL);
  vec_x = mxGetPr(plhs[0]);
  memcpy( vec_x, vec_x0, sizeof(double)*nVar );

  /* make diagonal of the Hessian matrix */
  diag_H = (double *) mxCalloc(nVar, sizeof(double));
  if( diag_H == NULL ) mexErrMsgTxt("Not enough memory.");
  for(i = 0; i < nVar; i++ ) {
    diag_H[i] = mat_H[nVar*i+i];
  }
  
  /*------------------------------------------------------------------- 
   Call the QP solver.
   -------------------------------------------------------------------*/

  state = libqp_splx_solver(&get_col, diag_H, vec_f, vec_b, vec_I, vec_S, vec_x, nVar, 
                        MaxIter, TolAbs, TolRel, QP_TH, NULL);
  
  /*------------------------------------------------------------------- 
    Set output arguments                                                   
    [x,QP,QD,exitflag,nIter] = libqp_splx_mex(...)
  ------------------------------------------------------------------- */

  plhs[1] = mxCreateDoubleMatrix(1,1,mxREAL);
  *(mxGetPr(plhs[1])) = (double)state.QP;

  plhs[2] = mxCreateDoubleMatrix(1,1,mxREAL);
  *(mxGetPr(plhs[2])) = (double)state.QD;

  plhs[3] = mxCreateDoubleMatrix(1,1,mxREAL);
  *(mxGetPr(plhs[3])) = (double)state.exitflag;

  plhs[4] = mxCreateDoubleMatrix(1,1,mxREAL);
  *(mxGetPr(plhs[4])) = (double)state.nIter;


  /* ------------------------------------------------------------------- 
`    Clean up 
  ------------------------------------------------------------------- */
  mxFree( diag_H );  
  return;
}
 
