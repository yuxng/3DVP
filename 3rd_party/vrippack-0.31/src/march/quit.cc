/*

Name:         quit.c

Coded:        Paul Ning

Modified by:  Brian Curless
              Computer Graphics Laboratory
              Stanford University

Comment:      Exit program. 


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
#include <unistd.h>

void Quit()
{
  mcfile header;

  header.mc_magic = MC_MAGIC;
  header.mc_type = MCT_STANDARD;
  header.mc_tmaptype = MCTM_STANDARD;
  header.mc_length = 3*TotalTriangles*sizeof(TriangleVertex);
/*
  MC_WriteHeader(OutMCFile,header);
  fclose(OutMCFile);
*/
    
  exit(0);

}  /* Quit */

