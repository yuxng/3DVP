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


#include "ChunkLink.h"

#include <stdio.h>
#include <unistd.h>

ChunkLink::ChunkLink()
{
    this->size = 0;
    this->chunk = NULL;
}


ChunkLink::ChunkLink(int size)
{
    this->size = size;
    this->chunk = new uchar[size];
    if (this->chunk == NULL) {
	fprintf(stderr, "ChunkLink::ChunkLink() : not enough memory.\n");
	exit(1);
    }
    this->next = NULL;
}


ChunkLink::~ChunkLink()
{
    delete [] chunk;
}

