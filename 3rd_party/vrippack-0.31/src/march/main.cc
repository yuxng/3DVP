/*

Name:         main.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Isosurface reconstruction using Marching Cubes-style
              table lookup on cube topologies.


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
#include "var.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
//#include <iostream>
#include <time.h>

clock_t tm;

void printUsage();
static void start_time();
static void end_time();
static void print_time();
static float time_elapsed();


int
main(int argc, char **argv)
{
   int index;
   
   start_time();
   
   if (argc < 3 || argc > 5) {
      printUsage();
      exit(1);
   }
   
   WriteNormals = FALSE;
   UseValueWeightProduct = FALSE;
   SaveGradientAsConfidence = FALSE;

   index = 1;
   char dash[2] = "-";

   threshold = 128;

   // Need to fix this; should use a loop, and thresh should be -thresh
   while (argv[index][0] == dash[0]) {
      if (strstr(argv[index], "-n")) {
	 WriteNormals = TRUE;
	 index++;
      } 
      else if (strstr(argv[index], "-vw")) {
	 UseValueWeightProduct = TRUE;
	 index++;
      }
      else if (strstr(argv[index], "-gc")) {
	 SaveGradientAsConfidence = TRUE;
	 index++;
      }
      else if (strstr(argv[index], "-thresh")) {
	 index++;
	 threshold = atoi(argv[index])/256.0;
	 index++;
      } else {
	 fprintf(stderr, "Invalid command line argument '%s'\n", argv[index]);
	 exit(1);
      }
   }

      /*
   if (strstr(argv[1], "-n")) {
      WriteNormals = TRUE;
      index++;
   } 
   else if (strstr(argv[1], "-vw")) {
      UseValueWeightProduct = TRUE;
      index++;
   }
   
   if (strstr(argv[2], "-n")) {
      WriteNormals = TRUE;
      index++;
   } 
   else if (strstr(argv[2], "-vw")) {
      UseValueWeightProduct = TRUE;
      index++;
   }
   */
   
   strcpy(infile, argv[index++]);
   strcpy(outfile, argv[index++]);
    
   if (index < argc)
      OCC_CONF_THRESHOLD = atoi((char *)argv[index++]);
   else 
      OCC_CONF_THRESHOLD = 0;
   
   FirstSliceFileNumber = 0;
   i0 = j_0 = k0 = 1;
   
   
   Init();
   /*  DoSlices();*/
   DoSlicesOccRLE();

   end_time();
   
   /*    print_time();*/
   
   Quit();
   
   return 0;
   
} /* main */



void 
printUsage()
{
    printf("Usage: mc <in-rle-file> <out-ply-file> [weight-threshold]\n");
}


static void
start_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime;
}

static void
end_time()
{
  struct tms buffer;
  times(&buffer);
  tm = buffer.tms_utime - tm;
}

static void
print_time()
{
   printf("%f seconds\n", (double) tm / (double) CLOCKS_PER_SEC);
   //std::cout << (double) tm / (double) CLOCKS_PER_SEC << " seconds" << std::endl;
}


static float
time_elapsed()
{
    return (double) tm / (double) CLOCKS_PER_SEC;
}



