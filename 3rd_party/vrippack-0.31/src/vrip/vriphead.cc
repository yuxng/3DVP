/*

Brian Curless

Computer Graphics Laboratory
Stanford University

---------------------------------------------------------------------

Copyright (1997) The Board of Trustees of the Leland Stanford Junior
University. Except for commercial resale, lease, license or other
commercial transactions, permission is hereby given to use, copy,
modify this software for academic purposes only.  No part of this
software or any derivatives thereof may be used in the production of
computer models for resale or for use in a commercial
product. STANFORD MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND
CONCERNING THIS SOFTWARE.  No support is implied or provided.

*/


#include <unistd.h>
#include <stdio.h>
#ifdef LINUX
#include <stdlib.h>
#endif

#include "OccGridRLE.h"

int main(int argc, char **argv)
{
   int xd, yd, zd, axis, flip;
   vec3f origin;
   float resolution;
   
   char *filename = argv[1];

   FILE *fp = fopen(filename, "rb");
   
   if (fp == NULL) {
      fprintf(stderr, "Could not open file.\n");
      exit(1);
   }
   
   fread(&xd, sizeof(int), 1, fp);
   fread(&yd, sizeof(int), 1, fp);
   fread(&zd, sizeof(int), 1, fp);
   fread(&axis, sizeof(int), 1, fp);
   fread(&flip, sizeof(int), 1, fp);
   fread(&origin, sizeof(float), 3, fp);
   fread(&resolution, sizeof(float), 1, fp);
   fclose(fp);

   printf("Dimensions: %d x %d x %d\n", xd, yd, zd);
   printf("Axis: %d  Flip: %d\n", axis, flip);
   printf("Origin: (%f, %f, %f)\n", origin[0], origin[1], origin[2]);
   printf("Resolution: %f\n", resolution);

   exit(0);
}
