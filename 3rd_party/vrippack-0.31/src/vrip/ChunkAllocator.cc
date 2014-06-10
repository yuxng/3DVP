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


#include "ChunkAllocator.h"

ChunkAllocator::ChunkAllocator()
{
    this->chunkSize = 0;
    this->chunkList = NULL;
    this->currentChunk = NULL;
    this->currentAddr = NULL;
    this->countInChunk = 0;
}


ChunkAllocator::ChunkAllocator(int chunkSize)
{
    this->chunkSize = chunkSize;

    this->chunkList = new ChunkLink(chunkSize);
    //printf("\nAllocating new chunk.\n");

    this->currentChunk = this->chunkList;
    this->currentAddr = this->chunkList->chunk;
    this->countInChunk = 0;
}


uchar *
ChunkAllocator::alloc(int num)
{
    this->newChunkIfNeeded(num);
    uchar *addr = this->currentAddr;
    this->currentAddr += num;
    this->countInChunk += num;
    return addr;
}


void
ChunkAllocator::newChunkIfNeeded(int num)
{
    if (num+this->countInChunk > this->chunkSize) {
	if (this->currentChunk->next == NULL) {
	    //printf("\nAllocating new chunk.\n");
	    this->currentChunk->next = new ChunkLink(chunkSize);
	}
	this->currentChunk = this->currentChunk->next;
	this->countInChunk = 0;
	this->currentAddr = this->currentChunk->chunk;
    }
}


void
ChunkAllocator::reset()
{
    this->currentChunk = this->chunkList;
    this->currentAddr = this->chunkList->chunk;
    this->countInChunk = 0;
}


ChunkAllocator::~ChunkAllocator()
{
    ChunkLink *chunk, *nextChunk;
    
    chunk = this->chunkList;
    while (chunk != NULL) {
	nextChunk = chunk->next;
	delete chunk;
	chunk = nextChunk;
    }
}

