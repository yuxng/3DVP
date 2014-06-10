/*

Name:         getinfo.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Get essential information.

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/

 
#include <stdio.h>
#include "mc.h"

void GetInfo()
{
  FILE *paramfile;
  Boolean WriteParams;
  char answer;

  printf("\n");
  printf("           *****  Welcome to MC Version 2.0  *****\n");
  printf("\n");
  printf("This program performs Marching Cubes reconstruction of contour\n");
  printf("surfaces.  The input data set consists of a sequence of 2-D\n");
  printf("slices taken from some 3-D object.\n\n");
  
  if (!(paramfile = fopen("mc.par","w"))) {
    WriteParams = FALSE;
    printf("     Error in opening \"mc.par\".  Parameters will not be\n");
    printf("     written to file\n\n");
  } else {
    WriteParams = TRUE;
    printf("     Input parameters will be written to file \"mc.par\".\n\n");
  }

  WriteParams = FALSE;

  printf("1) <input_root> file name?\n>");
  answer = (char) getchar();
  while (answer != '\n') {
    infile[strlen(infile)] = answer;
    answer = (char) getchar();
  }
  printf("%s\n",infile);
  if (WriteParams)
    fprintf(paramfile, "%s\n",infile);
  
  printf("2) <number> of first file?\n>");
  scanf("%d",&FirstSliceFileNumber);
  printf("%d\n",FirstSliceFileNumber);
  if (WriteParams)
    fprintf(paramfile, "%d\n",FirstSliceFileNumber);

  printf("3) Number of image planes?\n>");
  scanf("%d",&Nslice);
  printf("%d\n",Nslice);
  if (WriteParams)
    fprintf(paramfile, "%d\n",Nslice);

  printf("4) Row and column dimensions of cross sections? <row> <column>\n>");
  scanf("%d %d",&Nrow,&Ncol);
  printf("%d %d\n",Nrow,Ncol);
  if (WriteParams)
    fprintf(paramfile, "%d %d\n",Nrow,Ncol);
  
  printf("5) Separation between adjacent image planes?\n>");
  scanf("%f",&dslice);
  printf("%f\n",dslice);
  if (WriteParams)  
    fprintf(paramfile, "%f\n",dslice);

  printf("6) Separation between samples in adjacent rows and columns?\n");
  printf("   <row separation> <column separation>\n>");
  scanf("%f %f",&drow,&dcol);
  printf("%f %f\n",drow,dcol);
  if (WriteParams)  
    fprintf(paramfile, "%f %f\n",drow,dcol);

  printf("7) <output_root> file name?\n>");
  answer = (char) getchar();
  answer = (char) getchar();
  while (answer != '\n') {
    outfile[strlen(outfile)] = answer;
    answer = (char) getchar();
  }
  printf("%s\n",outfile);
  if (WriteParams)  
    fprintf(paramfile, "%s\n",outfile);

  printf("8) Reconstruct part of surface? {y/n}\n>");
  scanf("%1s",&answer);
  printf("%c\n",answer);
  if (WriteParams)  
    fprintf(paramfile, "%c\n",answer);
  if (answer == 'n') {
    i0 = j_0 = k0 = 1;
    i1 = Nslice - 2;
    j_1 = Nrow - 2;
    k1 = Ncol - 2;
  }
  else {
    printf("   8a) Starting indices? <i0> <j_0> <k0>\n>");
    scanf("%d %d %d",&i0,&j_0,&k0);
    printf("%d %d %d\n",i0,j_0,k0);
    if (WriteParams)  
      fprintf(paramfile, "%d %d %d\n",i0,j_0,k0);
    printf("   8b) Final indices? <i1> <j_1> <k1>\n>");
    scanf("%d %d %d",&i1,&j_1,&k1);
    printf("%d %d %d\n",i1,j_1,k1);
    if (WriteParams)  
      fprintf(paramfile, "%d %d %d\n",i1,j_1,k1);
  }
  printf("9) Threshold value for contour surface?\n>");
  scanf("%f",&threshold);
  printf("%f\n",threshold);
  if (WriteParams) {
    fprintf(paramfile, "%f\n",threshold);
    fclose(paramfile);
  }

} /* GetInfo */
