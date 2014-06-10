/*

Convert from Marc Levoy's .den file format to Paul Ning's slice format.

Brian Curless - November 1993

*/

#include <stdio.h>
#include <math.h>
#include "defines.h"
#include "levden.h"

unsigned char *dens; 
unsigned char *slice;


/******************************************************************************
Main routine.
******************************************************************************/

main(argc,argv)
  int argc;
  char *argv[];
{
  int i,j,k;
  int fd;
  char infile[80],outfile[80];
  unsigned char swap[12];
  int shd_version;
  int xlen,ylen,zlen;
  int total_len;
  int density_size;
  int index;
  int result;
  int val;

  strcpy (infile, argv[1]);

/*
  if (strlen (infile) < 4 ||
      strcmp (infile + strlen (infile) - 4, ".den") != 0)
      strcat (infile, ".den");
  
  printf ("reading densities from '%s'\n", infile);

*/

  Load_DEN_File(infile, TRUE);  

  dens = map_address;
  xlen = map_len[X];
  ylen = map_len[Y];
  zlen = map_len[Z];

  /* write to slice files */

  slice = (unsigned char *) malloc (xlen * ylen);

  for (i = 0; i < zlen; i++) {
    sprintf (outfile, "slice.%0d", i);
    index = i * xlen * ylen;
    for (j = 0; j < xlen; j++)
      for (k = 0; k < ylen; k++) {
	slice[j * ylen + k] = dens[index];
	index++;
      }
    printf ("writing file '%s'\n", outfile);
    if ((fd = creat(outfile, 0666)) < 0) {
      fprintf (stderr, "bad open\n");
      exit (-1);
    }
    write (fd, slice, xlen * ylen);
    close (fd);
  }
}

