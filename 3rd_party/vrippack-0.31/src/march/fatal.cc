/*
Name:         fatal.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Fatal exit from program

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
#include <unistd.h>
#include "mc.h"

void 
Fatal(char *message)
{
  printf("%s\n", message);
  fflush(stdout);
  exit(-1);
}  /* Fatal */
