/*

Convert from Marc Levoy's .shd file format to Paul Ning's slice format.

Greg Turk - November 1992

*/

#include <stdio.h>
#include <math.h>

unsigned short *density;
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

  if (strlen (infile) < 4 ||
      strcmp (infile + strlen (infile) - 4, ".shd") != 0)
      strcat (infile, ".shd");

  printf ("reading densities from '%s'\n", infile);

  /* open the .shd file */

  if ((fd = open(infile, 0)) < 0) {
    fprintf (stderr, "bad open\n");
    exit (-1);
  }

  /* read header info from the .shd file */
  if (read(fd, swap, 12) < 12) {
    fprintf (stderr, "Couldn't read from header.\n");
    exit (-1);
  }

  /* extract the header info */

  shd_version = swap[0] | (swap[1] << 8);
  printf ("shd_version: %d\n", shd_version);

  xlen = swap[2] | (swap[3] << 8);
  ylen = swap[4] | (swap[5] << 8);
  zlen = swap[6] | (swap[7] << 8);
  total_len = swap[9] | (swap[10] << 8) | (swap[11] << 16) | (swap[12] << 24);

  printf ("lengths (x y z): %d %d %d\n", xlen, ylen, zlen);
  density_size = xlen * ylen * zlen;
  printf ("density_size: %d\n", density_size);

  /* allocate space for density and read it from file */

  density = (unsigned short *) malloc (density_size * 2);
  if (density == NULL) {
    fprintf (stderr, "could not malloc density block\n");
    exit (-1);
  }

  result = read(fd, density, density_size * 2);
  if (result < density_size * 2) {
    fprintf (stderr, "Couldn't read density.\n");
    fprintf (stderr, "result = %d\n", result);
    exit (-1);
  }

  close (fd);

  /* write to slice files */

  slice = (unsigned char *) malloc (sizeof (xlen * ylen));

  for (i = 0; i < zlen; i++) {
    sprintf (outfile, "slice.%0d", i);
    index = i * xlen * ylen;
    for (j = 0; j < xlen; j++)
      for (k = 0; k < ylen; k++) {
	val = density[index] & 0x00ff;
	slice[j * ylen + k] = (unsigned char) val;
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

