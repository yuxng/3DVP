#include <iostream>

#include "ActiveGrid.h"

using namespace std;

const unsigned char MAX_DIST = 255;
const unsigned char MAX_D1 = ((1 << 6) - 1);
const unsigned short MID_VALUE = 32767;
const unsigned short MAX_VALUE = USHRT_MAX;

const unsigned short MAX_RUNLENGTH = ((1<<15)-1);
const unsigned short COMMON_BIT = 0x8000;

template <class T> void my_swap(T &a, T &b) { T t = a; a = b; b = t; }

ActVoxelElem commonElement;

unsigned short *tempLength = NULL;

void GetValidRange(int *possD2List, int possD2Length, int *result, int
									 *resLen);

bool VerifyInRange(int *possD2List, int possD2Length, int *possibleD2, int
									 possibleD2Len);

void GetDifference(int *range, int rangeLen, int *fullRange, int
									 fullRangeLen, int *result, int *resultLen);

inline int min(int a, int b)
{
	if (a < b) return a;
	else return b;
}

inline int max(int a, int b)
{
	if (a < b) return b;
	else return a;
}

// ActVoxelElem
// ------------
const ActVoxelElem & ActVoxelElem::operator=(const ActVoxelElem &rhs)
{
	if (this != &rhs) {
		// an ActVoxelElem is the size of an int, just case and copy
		//*((int *)this) = *((int *)&rhs);
		memcpy(this, &rhs, sizeof(ActVoxelElem));
	}
	
	return *this;
}

bool inline ActVoxelElem::operator== (const ActVoxelElem &rhs)
{
	return (d1==rhs.d1 && d2==rhs.d2 && value==rhs.value);
}

bool inline ActVoxelElem::operator!= (const ActVoxelElem &rhs)
{
	return !(d1==rhs.d1 && d2==rhs.d2 && value==rhs.value);
}

// ActVoxelScanline
// ----------------
void ActVoxelScanline::putScanline(ActVoxelElem *scanline, int len,
																	 unsigned short d2)
{
	int elemCount, runCount, scanCount, index;
	ActVoxelElem temp;
	bool foundValidD2 = false;
	int count;

	// reset the old scanline
	reset();
	
	// store the scanline - first find the total number of things 
	// I need to store
	index = elemCount = runCount = 0;

	// save the total length
	this->length = len;
	for (int i=0; i<len; i++) {
		temp = scanline[i];
		count = 1;
		
		if (temp.d2 < d2) {
			foundValidD2 = true;
		}

		// get the size of the different runs
		if (temp == commonElement) {
			while (TRUE) {
				if (i+1 >= len) break;
				
				if (commonElement == scanline[i+1]) {
					i++;
					count++;
					
					if (count > MAX_RUNLENGTH) break;
				} else
					break;
			}
			
			tempLength[runCount++] = count | COMMON_BIT;
		} else {
			// increment element count for the first value looked at
			elemCount++;

			while (TRUE) {
				if (i+1 >= len) break;
				
				if (commonElement != scanline[i+1]) {
					if (scanline[i+1].d2 < d2) {
						foundValidD2 = true;
					}

					count++;
					elemCount++;
					i++;
					
					if (count > MAX_RUNLENGTH) break;
				} else
					break;
			}
			
			tempLength[runCount++] = count;
		}
	}
	
	// allocate memory for the two runs and save the information
	elements = new ActVoxelElem[elemCount];
	run = new unsigned short[runCount];

	elemLength = elemCount;
	runLength = runCount;

	memcpy(run, tempLength, sizeof(unsigned short) * runCount);

	// now store the runs based on the cached values in tempLength
	scanCount = elemCount = 0;

	for (int i=0; i<runLength; i++) {
		if (run[i] & COMMON_BIT) {
			scanCount+=run[i] & ~COMMON_BIT;
		} else {
			memcpy(&elements[elemCount], &scanline[scanCount],
						 sizeof(ActVoxelElem) * run[i]);
			elemCount+=run[i];
			scanCount+=run[i];
		}
	}

	// if any of the d2's are within range, then
	// this run is worth decoding and looking at, otherwise it isn't
	anyValidElements = foundValidD2;
}

void ActVoxelScanline::putScanlineInRange(ActVoxelElem *scanline, int len, unsigned
																					short d2, int *range, int rangeLen)
{
	int elemCount, runCount, scanCount, index, emptyCount;
	ActVoxelElem temp;
	bool foundValidD2 = false;
	int count;

	// reset the old scanline
	reset();

	// save the total length
	this->length = len;
	
	index = elemCount = runCount = emptyCount = 0;
	for (int i=0; i<rangeLen; i+=2) {
		int rangeStart = range[i];
		int rangeEnd = range[i+1];
		
		// get trailint empty values
		emptyCount += rangeStart - index;
		
		for (int j=rangeStart; j<rangeEnd; j++) {
			temp = scanline[j];
			count = 1;
			
			if (temp.d2 < d2) {
				foundValidD2 = true;
			}

			if (temp == commonElement) {
				while (true) {
					if (j+1 == rangeEnd) break;
					
					if (scanline[j+1] == commonElement) {
						count++;
						j++;
					} else {
						break;
					}
				}
				
				if (j+1==rangeEnd) {
					emptyCount += count;
				} else {
					tempLength[runCount++] = ((emptyCount + count) | COMMON_BIT);
					
					emptyCount = 0;
				}
			} else {
				if (emptyCount != 0) {
					tempLength[runCount++] = emptyCount | COMMON_BIT;

					emptyCount = 0;
				}

				// loop over the non common values
				elemCount++;
				while (true) {
					if (j+1 == rangeEnd) break;
					
					if (scanline[j+1] != commonElement) {
						elemCount++;
						j++;
						count++;
						
						if (scanline[j+1].d2 < d2) {
							foundValidD2 = true;
						}
						
					} else {
						break;
					}
				}
				
				tempLength[runCount++] = count;
			}
		}
		
		index = rangeEnd;
	}
	
	// deal with trailing ones
	if (len - index > 0 || emptyCount > 0) {
		tempLength[runCount++] = ((len - index + emptyCount) | COMMON_BIT);
	}
	
	// allocate memory for the two runs and save the information
	elements = new ActVoxelElem[elemCount];
	run = new unsigned short[runCount];

	elemLength = elemCount;
	runLength = runCount;

	memcpy(run, tempLength, sizeof(unsigned short) * runCount);

	// now store the runs based on the cached values in tempLength
	scanCount = elemCount = 0;

	for (int i=0; i<runLength; i++) {
		if (run[i] & COMMON_BIT) {
			scanCount+=run[i] & ~COMMON_BIT;
		} else {
			memcpy(&elements[elemCount], &scanline[scanCount],
						 sizeof(ActVoxelElem) * run[i]);
			elemCount+=run[i];
			scanCount+=run[i];
		}
	}

	// if any of the d2's are within range, then
	// this run is worth decoding and looking at, otherwise it isn't
	anyValidElements = foundValidD2;
}

void ActVoxelScanline::getScanline(ActVoxelElem *scanline)
{
	ActVoxelElem temp;
	int count=0, elemCount=0, runCount=0, j;
	unsigned short runValue;

	int runIndex = 0;
	int scanIndex = 0;
	
	// i indexes into list of runs
	for (int i=0; runIndex<length; i++) {
		if (run[i] & COMMON_BIT) {
			runValue = run[i] & ~COMMON_BIT;
			runIndex += runValue;
			
			for (j=0; j<runValue; j++) {
				*((int *)(&scanline[scanIndex++])) = *((int *)&commonElement);
			}
		}
		else {
			runValue = run[i];
			runIndex += runValue;
			
			memcpy(&scanline[scanIndex], &elements[elemCount], runValue * sizeof(ActVoxelElem));
			scanIndex+=runValue;
			elemCount+=runValue;
		}
	}
}

void ActVoxelScanline::getScanlineInRange(ActVoxelElem *scanline, int *range, int len)
{
	int runStart, runEnd;
	int rangeStart, rangeEnd;
	int runIndex = 0, rangeIndex = 0;
	bool empty = false;
	int elemIndex = 0;
	int delta;

	// get initial ranges of both runs
	rangeStart = range[rangeIndex++];
	rangeEnd = range[rangeIndex++];
	
	runStart = 0;
	runIndex = 0;
	if (run[runIndex] & COMMON_BIT) {
		empty = true;
		runEnd = runStart + (run[runIndex++] & ~COMMON_BIT);
	} else {
		empty = false;
		runEnd = runStart + run[runIndex++];
	}

	// fill in the runline
	for (int i=0; i<len; i+=2) {
		rangeStart = range[i];
		rangeEnd = range[i+1];
		
		// now fill in the range
		while (rangeEnd - rangeStart > 0) {
			if (runEnd <= rangeStart) {
				// advance to next run
				runStart = runEnd;
				if (run[runIndex] & COMMON_BIT) {
					empty = true;
					runEnd = runStart + (run[runIndex++] & ~COMMON_BIT);
				} else {
					empty = false;
					runEnd = runStart + run[runIndex++];
				}

			} else {
				// advance runstart to rangeStart
				// delta = rangeStart - runStart; <== needed if things should get general
				// if (!empty) { elemIndex += delta; }
				runStart = rangeStart;
				
				if (runEnd < rangeEnd) {
					// copy runStart to runEnd values
					if (empty) {
						for (int j=runStart; j<runEnd; j++) {
							scanline[j] = commonElement;
						}
					} else {
						memcpy(&(scanline[runStart]), &(elements[elemIndex]),
									 sizeof(ActVoxelElem) * (runEnd - runStart));
						elemIndex += runEnd-runStart;
					}

					// advance to next run
					runStart = runEnd;
					rangeStart = runEnd;
					
					if (run[runIndex] & COMMON_BIT) {
						empty = true;
						runEnd = runStart + (run[runIndex++] & ~COMMON_BIT);
					} else {
						empty = false;
						runEnd = runStart + run[runIndex++];
					}
				} else {
					// copy runStart to rangeEnd values
					if (empty) {
						for (int j=runStart; j<rangeEnd; j++) {
							scanline[j] = commonElement;
						}
					} else {
						memcpy(&(scanline[runStart]), &(elements[elemIndex]),
									 sizeof(ActVoxelElem) * (rangeEnd - runStart));
						elemIndex += rangeEnd-runStart;
					}
					
					// update the other values
					rangeStart = rangeEnd;
					runStart = rangeEnd;
				}
			}
		}
	}
	
	if (elemIndex != elemLength)
		cerr << "Not getting everything" << endl;
}

void ActVoxelScanline::copy(ActVoxelScanline *copyLine)
{
	reset();
	
	run = new unsigned short[copyLine->runLength];
	elements = new ActVoxelElem[copyLine->elemLength];
	
	memcpy(run, copyLine->run, sizeof(unsigned short) *
				 copyLine->runLength);
	memcpy(elements, copyLine->elements, sizeof(ActVoxelElem) *
				 copyLine->elemLength);

	runLength = copyLine->runLength;
	elemLength = copyLine->elemLength;
	length = copyLine->length;
	anyValidElements = copyLine->anyValidElements;
}

void ActVoxelScanline::reset()
{
	if (elements)
		delete [] elements;
	if (run)
		delete [] run;
	
	length = 0;
}

int ActVoxelScanline::getScanlineRuns(int *validRun)
{
	int index=0;
	int runIndex = 0;

	for (int i=0; i<runLength; i++) {
		if (run[i] & COMMON_BIT)
			index += run[i] & ~COMMON_BIT;
		else {
			// start value
			if (index-2 >= 0)
				validRun[runIndex++] = (index - 2) << 1;
			else
				validRun[runIndex++] = 0 << 1;

			// end value
			if (index + run[i] + 2 < length)
				validRun[runIndex++] = ((index + run[i] + 2) << 1) + 1;
			else
				validRun[runIndex++] = ((length) << 1) + 1;

			index+= run[i];
		}
	}
	
	if (index != length ) cerr << "Error" << endl;
	
	return runIndex;
}

// ActVoxelGrid
// ------------

ActVoxelGrid::ActVoxelGrid()
{
	scanlines = NULL;
	
	firstPass = true;
}

ActVoxelGrid::ActVoxelGrid(int dx, int dy, int dz)
{
	// load the dimensions of the grid
	xdim = dx;
	ydim = dy;
	zdim = dz;

	// iterate over the voxels and store the results
	// for now I'm just doing 2D - in the future this will be in 3D
	int ix, iy; //, iz;
	
	// reset the current memory and allocate new
	reset();
	scanlines = new ActVoxelScanline *[zdim];
	for (int i=0; i<zdim; i++) 
		scanlines[i] = new ActVoxelScanline [ydim];
}

ActVoxelGrid::~ActVoxelGrid()
{
	reset();
	
	if (tempLength) delete [] tempLength;
}

void ActVoxelGrid::draw(FrameBuffer *fb, int sweep, int type, int d1, int d2)
{
	unsigned char r, g, b;
	
	ActVoxelElem *scanline = new ActVoxelElem[xdim];

	// display the data to the framebuffer
	for (int iy=0; iy<ydim; iy++) {
		for (int ix=0; ix<xdim; ix++) {
			scanlines[sweep][iy].getScanline(scanline);

			unsigned short val = scanline[ix].value;
			unsigned char d1temp = scanline[ix].getD1(); // replace with getD1() later
			unsigned char d2temp = scanline[ix].d2;
			
			//if (scanline[ix] == commonElement) {
			//	r = 0;
			//	g = 255;
			//	b = 255;
			//} else {

			switch (type) {
			case 0:
				r = (char)(((float)val/65535) * 255);
				g=b=0;
				//g = (char)(((float)val/65535) * 255);
				//b = (char)(((float)val/65535) * 255);
				if (scanline[ix].getType() == EMPTY) {
					r = g = b = 0;
				}
				break;
			case 1:
				r = 0;
				g = MAX_D1 - d1temp;
				b = 0;
				
				if (d1temp == 0){
					g = 0;
					b = 255;
				}
				break;
			case 2:
				//if (d2temp < d2)
				//	b = 255 - d2temp;
				//else 
				//	b = 0;
				b = 255 - d2temp;
				r = 0;
				g = 0;
				
				if (d2temp == 0) {
					r = 255;
					b = 0;
				}
				break;
			case 3:
				r = (char)(((float)val/65535) * 255);	
				if (d1temp < d1)
					g = 255 - d1temp;
				else 
					g = 0;
				
				if (d2temp < d2) {
					b = (char)(((float)(d2 - d2temp) / d2) * 255);
					//b = 255 - d2temp;
				} else {
					b = 0;
					r = 0;
				}
				
				if (scanline[ix].getType() == EMPTY) {
					r = 0;
				}
				
				break;
			}
			//}
			
			fb->DrawPixel(r, g, b, ix, iy);
		}
	}
	
	delete [] scanline;
}

// loads an OccGridRLE into
// the Active Voxel Grid data structure
void ActVoxelGrid::loadFromOccGridRLE(OccGridRLE *ogSrc, unsigned char maxD2Dist, bool conserveMemory)
{
	// load the dimensions of the grid
	xdim = ogSrc->xdim;
	ydim = ogSrc->ydim;
	zdim = ogSrc->zdim;

	// set up temporary memory for scanline copies
	tempLength = new unsigned short[xdim];

	// iterate over the voxels and store the results
	// for now I'm just doing 2D - in the future this will be in 3D
	int ix, iy, iz;
	
	// reset the current memory and allocate new
	reset();
	scanlines = new ActVoxelScanline *[zdim];
	for (int i=0; i<zdim; i++) 
		scanlines[i] = new ActVoxelScanline [ydim];

	ActVoxelElem *scanline = new ActVoxelElem[xdim];
	
	// set the common element
	commonElement.setD1(MAX_D1);
	commonElement.setType(EMPTY);

	commonElement.d2 = MAX_DIST;
	commonElement.value = 0;

	for (iz = 0; iz < ogSrc->zdim; iz++) { // sweep in only one direction
		for (iy = 0; iy < ogSrc->ydim; iy++) {
			
			OccElement *line = ogSrc->getScanline(iy, iz);

			// in future will want to make this supportive of different data structures
			for (ix = 0; ix < ogSrc->xdim; ix++) {
				unsigned short value = line[ix].value;
				unsigned short weight = line[ix].totalWeight;

				if (weight!=0) {
					scanline[ix].setType(FILLED);
				} else {
					scanline[ix].setType(EMPTY);
					value = 0;
				}
				
				scanline[ix].value = value;

				// use the MSB of d1 to store if the voxel is valid or not
				scanline[ix].setD1(MAX_D1);
				scanline[ix].d2 = MAX_DIST;
			}
			
			scanlines[iz][iy].putScanline(scanline, xdim, maxD2Dist);
		}
	}
}

void ActVoxelGrid::saveSpace(unsigned char maxD2Dist)
{
	// run through the grid and get rid of anything not within d2 range
	int iz, iy, ix;
	int count = 0;
	ActVoxelElem *scanline = new ActVoxelElem[xdim];

	cerr << "Emptying out space" << endl;
	
	for (iz=0; iz<zdim; iz++) {
		for (iy=0; iy<ydim; iy++) {
			scanlines[iz][iy].getScanline(scanline);

			for (ix=0; ix<xdim; ix++) {
				if (scanline[ix].d2 > maxD2Dist) {
					scanline[ix] = commonElement;
					count++;
				}
			}
			
			scanlines[iz][iy].putScanline(scanline, xdim, maxD2Dist);
		}
	}
	
	cerr << count << endl;
	delete [] scanline;
}

// blur, update distances, etc...
void ActVoxelGrid::blur(unsigned char d1limit, unsigned char d2limit)
{
	ActVoxelScanline *sweeps[2];
	bool *usedScanline[2];
	ActVoxelElem *middle;
	ActVoxelElem *temp = new ActVoxelElem[xdim];

	int ix, iy, iz, i, j, k;
	unsigned short value;
	bool s, edge, voidNeighbor, emptyStart;
	int min, numInvalid = 0, minD1;
	SweepLine sweepLine(xdim);

	// allocate space for the sweeps
	sweeps[0] = new ActVoxelScanline[ydim];
	sweeps[1] = new ActVoxelScanline[ydim];
	usedScanline[0] = new bool[ydim];
	usedScanline[1] = new bool[ydim];
	
	// need to store results in temp, then save this
	cerr << "Blurring...";
	
	for (iz=0; iz<zdim; iz++) {
		for (iy=0; iy<ydim; iy++) {
			// get the 9 surrounding scanlines
			if (sweepLine.getScanlinesD2Info(this, iy, iz, d2limit)) {
				// alias the middle scanline
				middle = sweepLine.scanlines[1][1];
				memcpy(temp, middle, sizeof(ActVoxelElem) * xdim);
				int scanIndex=0;

				while (sweepLine.possibleD2[scanIndex] != -1) {
					for (ix=sweepLine.possibleD2[scanIndex]; ix<=sweepLine.possibleD2[scanIndex+1]; ix++) {
						value = middle[ix].value;
						s = value > MID_VALUE;
						edge = false;
						voidNeighbor = false;
						emptyStart = middle[ix].value == EMPTY;
					
						// perform the distance calculations
						min = MAX_DIST;
						minD1 = MAX_D1;
						
						int sum = 0;
						int c = 0;
					
						for (i=-1; i<=1; i++) { // x direction
							for (j=-1; j<=1; j++) { // y direction
								for (k=-1; k<=1; k++) { // z direction
									if (sweepLine.valid[k+1][j+1] && // sweepLine.valid will do the y, z check
											ix + i >= 0 && ix + i < xdim) {
										if (sweepLine.scanlines[k+1][j+1][i+ix].d2 < min)
											min = sweepLine.scanlines[k+1][j+1][i+ix].d2;
									
										if (sweepLine.scanlines[k+1][j+1][i+ix].getType() > EMPTY &&
												!emptyStart &&
												((sweepLine.scanlines[k+1][j+1][i+ix].value >
													MID_VALUE) != s))
											edge = true;
										
										if (sweepLine.scanlines[k+1][j+1][i+ix].getType() ==
												EMPTY) {
											if (i !=0 || j != 0 || k != 0)
												voidNeighbor = true;
										} else {
											c++;
											sum += sweepLine.scanlines[k+1][j+1][ix+i].value;
										}
										
										// do d1 min checks
										if (!firstPass && sweepLine.scanlines[k+1][j+1][i+ix].getD1() < minD1)
											minD1 = sweepLine.scanlines[k+1][j+1][i+ix].getD1();
									}
								}
							}
						}
					
						// d2 calculations
						if (voidNeighbor && edge && middle[ix].getType() > EMPTY) {
							//cerr << "Found one" << endl;
							temp[ix].d2 = 0;
						} else {
							// add one to min unless this causes an overflow
							if (min > d2limit) min = MAX_DIST;
							else if (min != MAX_DIST) min++;
						
							temp[ix].d2 = min;
						}
					
						// d1 calculations
						if (!firstPass) {
							if (middle[ix].getType() == MODIFYABLE && edge) {
								//cerr << "Found one" << endl;
								temp[ix].setD1(0);
							} else {
								// add one to min unless this causes an overflow
								if (minD1 > d1limit) minD1 = MAX_D1;
								else if (minD1 != MAX_D1) minD1++;
							
								temp[ix].setD1(minD1);
							}
						}
					
						// perform the blur
						if (temp[ix].getType() < FILLED && 
								temp[ix].d2 < d2limit  && 
								(firstPass || temp[ix].getD1() < d1limit)) {
							if (c!=0) {
								temp[ix].value = sum / c;
								temp[ix].setType(MODIFYABLE);
							}
						}
					}
					
					scanIndex+=2;
				}
				
				sweeps[0][iy].putScanlineInRange(temp, xdim, d2limit,
																			 sweepLine.possRange,
																			 sweepLine.possRangeLength);
				usedScanline[0][iy] = true;
			} else {
				usedScanline[0][iy] = false;
				numInvalid++;
			}
		}
		
		// copy over the scanline
		if (iz != 0) {
			for (i=0; i<ydim; i++) {
				if (usedScanline[1][i]) {
					scanlines[iz-1][i].copy(&sweeps[1][i]);
				}
			}
		}
		
		my_swap(sweeps[0], sweeps[1]);
		my_swap(usedScanline[0],  usedScanline[1]);
	}

	// copy the last row over
	for (i=0; i<ydim; i++) {
		if (usedScanline[1][i]) {
			scanlines[iz-1][i].copy(&sweeps[1][i]);
		}
	}
	
	// delete the old sweeps
	delete [] sweeps[0];
	delete [] sweeps[1];
	
	delete [] usedScanline[0];
	delete [] usedScanline[1];
	
	firstPass = false;

	cerr << "Done " << endl;
}

void ActVoxelGrid::saveToOccGridRLE(OccGridRLE *ogSrc, OccGridRLE *ogDst)
{
	ogDst->reset();
	ActVoxelElem *scanline = new ActVoxelElem[xdim];
	OccElement *line = new OccElement[xdim];
	OccElement *line2 = new OccElement[xdim];
	
	for (int iz=0; iz<zdim; iz++) {
		for (int iy=0; iy<ydim; iy++) {
			scanlines[iz][iy].getScanline(scanline);
			ogSrc->getScanline(iy, iz, line2);

			for (int ix=0; ix<xdim; ix++) {
				line[ix].value = scanline[ix].value;
				if (line2[ix].totalWeight==0)
					line[ix].totalWeight = (scanline[ix].getType() == MODIFYABLE) ? 2 : 0;
				else
					line[ix].totalWeight = line2[ix].totalWeight;
			}
			
			ogDst->putScanline(line, iy, iz);
		}
	}
	
	delete [] scanline;
	delete [] line;
	delete [] line2;
}

void ActVoxelGrid::initDistances(unsigned char maxD2Dist, bool propD2)
{
	ActVoxelElem *middle;
	SweepLine sweepLine(xdim);
	
	int ix, iy, iz, i, j, k;
	unsigned short value;
	bool s, edge, voidNeighbor;

	cerr << "Initializing Distances" << endl;
	
	for (iz=0; iz<zdim; iz++) {
		for (iy=0; iy<ydim; iy++) {			
			// get the 9 surrounding scanlines
			sweepLine.getScanlines(this, iy, iz);
			// alias the middle scanline
			middle = sweepLine.scanlines[1][1];
			
			for (ix=0; ix<xdim; ix++) {
				value = middle[ix].value;
				s = value > MID_VALUE;
				edge = false;
				voidNeighbor = false;
				
				// perform the distance calculation
				if (middle[ix].getType() == FILLED) {

					for (i=-1; i<=1; i++) { // x direction
						for (j=-1; j<=1; j++) { // y direction
							for (k=-1; k<=1; k++) { // z direction
								if ((i!=0 || j!=0 || k!=0) &&
										sweepLine.valid[k+1][j+1] && // sweepLine.valid will do the y, z check
										ix + i >= 0 && ix + i < xdim) {
									
									if (sweepLine.scanlines[k+1][j+1][i+ix].getType() == FILLED &&
											((sweepLine.scanlines[k+1][j+1][i+ix].value >
												MID_VALUE) != s))
										edge = true;
									
									if (sweepLine.scanlines[k+1][j+1][i+ix].getType() == EMPTY)
										voidNeighbor = true;
								}
							}
						}
					}
					
					if (voidNeighbor && edge) {
						//cerr << "Found one" << endl;
						middle[ix].d2 = 0;
					} else {
						middle[ix].d2 = MAX_DIST;
					}
				}
			}
			
			// put the scanline
			scanlines[iz][iy].putScanline(sweepLine.scanlines[1][1], xdim, maxD2Dist);
		}
	}

	// now propogate this distance
	if (propD2) 
		PropogateD2Distances(maxD2Dist);
}

void ActVoxelGrid::PropogateD2DistancesTest(unsigned char maxD2Dist)
{
	cerr << "Propogating Distances" << endl;
	int i, ix, iy, iz, j;
	unsigned short d2;
	bool valid;
	
	ActVoxelElem *scanline = new ActVoxelElem[xdim];
	ActVoxelElem *temp[2];
	
	ActVoxelElem **values;
	
	cerr << "Sweeping in x..." << endl;
	for (iz=0; iz<zdim; iz++) {
		for (iy=0; iy<ydim; iy++) {
			
			scanlines[iz][iy].getScanline(scanline);

			for (ix=0; ix<xdim; ix++) {
				if (scanline[ix].d2 == 0) {
					valid = false;
					
					for (i=ix+1; i<min(ix+maxD2Dist,xdim); i++) {
						if (scanline[i].d2 == 0 || scanline[i].getType() != EMPTY) {
							valid = true;
							break;
						}
					} 
					
					if (valid) {
						d2 = MAX_DIST;
						for (j=ix+1; j<i; j++) {
							d2 = scanline[j-1].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < scanline[j].d2)
								scanline[j].d2 = d2;
						}
					}
				}
			}

			for (ix=xdim-1; ix>=0; ix--) {
				if (scanline[ix].d2 == 0) {
					valid = false;

					for (i=ix-1; i>=max(ix-maxD2Dist,0); i--) {
						if (scanline[i].d2 == 0 || scanline[i].getType() != EMPTY) {
							valid = true;
							break;
						}
					} 
					
					if (valid) {
						d2 = MAX_DIST;
						for (j=ix-1; j>i; j--) {
							d2 = scanline[j+1].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < scanline[j].d2)
								scanline[j].d2 = d2;
						}
					}
				}
			}
			
			scanlines[iz][iy].putScanline(scanline, xdim, maxD2Dist);
		}
	}
	
	cerr << "Sweeping in y..." << endl;
	values = new ActVoxelElem *[ydim];
	for (i=0; i<ydim; i++) {
		values[i] = new ActVoxelElem [xdim];
	}

	// get temporary values
	for (iz=0; iz<zdim; iz++) {
		// get a sweep of values
		for (iy=0; iy<ydim; iy++) {
			scanlines[iz][iy].getScanline(values[iy]);
		}

		// do the distance propogation
		for (ix=0; ix<xdim; ix++) {
			for (iy=0; iy<ydim; iy++) {			
				if (values[iy][ix].d2 != MAX_DIST) {
					valid = false;
					
					// sweep in one direction
					for (i=iy+1; i<min(iy+maxD2Dist, ydim); i++) {
						if (values[i][ix].d2 != MAX_DIST || values[i][ix].getType() !=
								EMPTY) {
							valid = true;
							break;
						}
					}
					
					if (valid) {
						d2 = MAX_DIST;
						for (j=iy+1; j<i; j++) {
							d2 = values[j-1][ix].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < values[j][ix].d2)
								values[j][ix].d2 = d2;
						}
					}
					
					
					// go the other way
					for (i=iy-1; i>=max(iy-maxD2Dist, 0); i--) {
						if (values[i][ix].d2 != MAX_DIST || values[i][ix].getType() !=
								EMPTY) {
							valid = true;
							break;
						}
					}

					if (valid) {
						d2 = MAX_DIST;
						for (j=iy-1; j>i; j--) {
							d2 = values[j+1][ix].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < values[j][ix].d2)
								values[j][ix].d2 = d2;
						}
					}
				}
			}
		}
		
		// copy the values back in
		for (iy=0; iy<ydim; iy++) {
			scanlines[iz][iy].putScanline(values[iy], xdim, maxD2Dist);
		}
	}
	
	for (i=0; i<ydim; i++) {
		delete [] values[i];
	}
	delete [] values;
	
	cerr << "Sweeping in z..." << endl;
	values = new ActVoxelElem *[zdim];
	for (i=0; i<zdim; i++) {
		values[i] = new ActVoxelElem [xdim];
	}

	// get temporary values
	for (iy=0; iy<ydim; iy++) {
		// get a sweep of values
		for (iz=0; iz<zdim; iz++) {
			scanlines[iz][iy].getScanline(values[iz]);
		}

		// do the distance propogation
		for (ix=0; ix<xdim; ix++) {
			for (iz=0; iz<zdim; iz++) {			
				if (values[iz][ix].d2 != MAX_DIST) {
					valid = false;
					
					// sweep in one direction
					for (i=iz+1; i<min(iz+maxD2Dist, zdim); i++) {
						if (values[i][ix].d2 != MAX_DIST || values[i][ix].getType() !=
								EMPTY) {
							valid = true;
							break;
						}
					}
					
					if (valid) {
						d2 = MAX_DIST;
						for (j=iz+1; j<i; j++) {
							d2 = values[j-1][ix].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < values[j][ix].d2)
								values[j][ix].d2 = d2;
						}
					}
					
					
					// go the other way
					for (i=iz-1; i>=max(iz-maxD2Dist, 0); i--) {
						if (values[i][ix].d2 != MAX_DIST || values[i][ix].getType() !=
								EMPTY) {
							valid = true;
							break;
						}
					}

					if (valid) {
						d2 = MAX_DIST;
						for (j=iz-1; j>i; j--) {
							d2 = values[j+1][ix].d2 + 1;
							if (d2 > maxD2Dist) d2 = MAX_DIST;
							if (d2 < values[j][ix].d2)
								values[j][ix].d2 = d2;
						}
					}
				}
			}
		}
		
		// copy the values back in
		for (iz=0; iz<zdim; iz++) {
			scanlines[iz][iy].putScanline(values[iz], xdim, maxD2Dist);
		}
	}
	
	for (i=0; i<zdim; i++) {
		delete [] values[i];
	}
	delete [] values;	
	
	delete [] scanline;
}

void ActVoxelGrid::PropogateD2Distances(unsigned char maxD2Dist)
{
	cerr << "Propogating Distances" << endl;
	int i, ix, iy, iz;
	unsigned short d2;
	ActVoxelElem *scanline = new ActVoxelElem[xdim];
	ActVoxelElem *temp[2];
	
	temp[0] = new ActVoxelElem[xdim];
	temp[1] = new ActVoxelElem[xdim];

	cerr << "Sweeping in X..." << endl;
	for (iz=0; iz<zdim; iz++) {
		for (iy=0; iy<ydim; iy++) {
			scanlines[iz][iy].getScanline(scanline);
			
			for (int ix=1; ix<xdim; ix++) {
				d2 = scanline[ix-1].d2 +1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < scanline[ix].d2)
					scanline[ix].d2 = d2;
			}
			
			for (int ix=xdim-2; ix>=0; ix--) {
				d2 = scanline[ix+1].d2 + 1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < scanline[ix].d2)
					scanline[ix].d2 = d2;
			}
			
			scanlines[iz][iy].putScanline(scanline, xdim, maxD2Dist);
		}
	}
	
	cerr << "Sweeping in Y..." << endl;
	for (iz=0; iz<zdim; iz++) {
		scanlines[iz][0].getScanline(temp[0]);

		for (iy=1; iy<ydim; iy++) {
			scanlines[iz][iy].getScanline(temp[1]);

			for (ix=0; ix<xdim; ix++) {
				d2 = temp[0][ix].d2 + 1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < temp[1][ix].d2)
					temp[1][ix].d2 = d2;
			}
			
			scanlines[iz][iy].putScanline(temp[1], xdim, maxD2Dist);
			my_swap(temp[0], temp[1]);
		}
	}
	
	for (iz=0; iz<zdim; iz++) {
		scanlines[iz][ydim-1].getScanline(temp[0]);

		for (iy=ydim-2; iy>=0; iy--) {
			scanlines[iz][iy].getScanline(temp[1]);

			for (ix=0; ix<xdim; ix++) {
				d2 = temp[0][ix].d2 + 1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < temp[1][ix].d2)
					temp[1][ix].d2 = d2;
			}
			
			scanlines[iz][iy].putScanline(temp[1], xdim, maxD2Dist);
			my_swap(temp[0], temp[1]);
		}
	}
	
	cerr << "Sweeping in Z..." << endl;
	for (iy=0; iy<ydim; iy++) {
		scanlines[0][iy].getScanline(temp[0]);
		
		for (iz=1; iz<zdim; iz++) {
			scanlines[iz][iy].getScanline(temp[1]);
			
			for (ix=0; ix<xdim; ix++) {
				d2 = temp[0][ix].d2 + 1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < temp[1][ix].d2)
					temp[1][ix].d2 = d2;
			}
			
			scanlines[iz][iy].putScanline(temp[1], xdim, maxD2Dist);
			my_swap(temp[0], temp[1]);
		}
	}
	
	for (iy=0; iy<ydim; iy++) {
		scanlines[zdim-1][iy].getScanline(temp[0]);

		for (iz=zdim-2; iz>=0; iz--) {
			scanlines[iz][iy].getScanline(temp[1]);

			for (ix=0; ix<xdim; ix++) {
				d2 = temp[0][ix].d2 + 1;
				if (d2 > maxD2Dist) d2 = MAX_DIST;
				if (d2 < temp[1][ix].d2)
					temp[1][ix].d2 = d2;
			}
			
			scanlines[iz][iy].putScanline(temp[1], xdim, maxD2Dist);
			my_swap(temp[0], temp[1]);
		}
	}
	
	delete [] scanline;
	delete [] temp[0];
	delete [] temp[1];
}

void ActVoxelGrid::reset()
{
	// free up all memory
	if (scanlines) {
		for (int i=0; i<zdim; i++)
			delete [] scanlines[i];
		delete [] scanlines;
	}

	scanlines = NULL;
}

bool VerifyInRange(int *possD2List, int possD2Length, int *possibleD2, int
									 possibleD2Len)
{
	for (int i=0; i<possibleD2Len-1; i++) {
		if (possibleD2[i] > possibleD2[i+1]) 
			cerr << "Could be bad" << endl;
	}

	for (int i=0; i<possD2Length; i+=2) {
		bool found = false;

		for (int j=0; j<possibleD2Len; j+=2) {
			if (possD2List[i] >= possibleD2[j] &&
					possD2List[i+1] <= possibleD2[j+1]) {
				found = true;
				break;
			}
		}
		
		if (!found) return false;
	}
	
	return true;
}

bool SweepLine::getScanlinesD2Info(ActVoxelGrid *src, int iy, int iz,
																	 unsigned char d2limit)
{
	ActVoxelElem *tempScanlines[9];
	int inRangeLines = 0;
	bool validD2 = false;
	int possibleD2Len = 0;
	
	// get everything anew if the lastY was not the one previous to it
	if (lastY != iy - 1) {
		possRangeLength = 0;
		
		// check for any valid lines
		for (int i=-1; i<=1; i++) {
			for (int j=-1; j<=1; j++) {
				if ((j+iy) >= 0 && (j+iy) < src->getYDim() &&
						(i+iz) >= 0 && (i+iz) < src->getZDim()) {
					
					valid[j+1][i+1] = true;
					if (src->validD2Distance(j+iy, i+iz)) {
						validD2 = true;
						tempScanlines[inRangeLines++] = scanlines[j+1][i+1];
					}
					
					// add the run to the ranges -
					// MUST use all runs regardless of D2 validity because
					// any non common voxels count here
					possRangeLength += src->getScanlineRuns(&(possRange[possRangeLength]),
																									iy+j, iz+i);
				} else {
					valid[j+1][i+1] = false;
				}
			}
		} 
		
		if (!validD2) {
			lastZ = lastY = -3;
			possRangeLength = 0;
			return false;
		}

		GetValidRange(possRange, possRangeLength, possibleD2, &possibleD2Len);
		
		// get the scanlines for this range
		for (int i=-1; i<=1; i++) {
			for (int j=-1; j<=1; j++) {
				if ((j+iy) >= 0 && (j+iy) < src->getYDim() &&
						(i+iz) >= 0 && (i+iz) < src->getZDim()) {
					src->getScanlineInRange(scanlines[j+1][i+1], iy+j, iz+i,
																	possibleD2, possibleD2Len);
				}
			}
		}
	} else {
		// 6 of the 9 lines can be reused - get difference in 
		// runs of the two lines and use this to update previous 6
		ActVoxelElem *temp1, *temp2, *temp3;
		
		temp1 = scanlines[0][0];
		temp2 = scanlines[0][1];
		temp3 = scanlines[0][2];

		// move good values over 1
		scanlines[0][0] = scanlines[1][0];
		valid[0][0] = valid[1][0];
		scanlines[0][1] = scanlines[1][1];
		valid[0][1] = valid[1][1];
		scanlines[0][2] = scanlines[1][2];
		valid[0][2] = valid[1][2];
		
		scanlines[1][0] = scanlines[2][0];
		valid[1][0] = valid[2][0];
		scanlines[1][1] = scanlines[2][1];
		valid[1][1] = valid[2][1];
		scanlines[1][2] = scanlines[2][2];
		valid[1][2] = valid[2][2];
		
		// save temp to save pointers
		scanlines[2][0] = temp1;
		scanlines[2][1] = temp2;
		scanlines[2][2] = temp3;

		// load in new y values
		for (int i=-1; i<=1; i++) {
			if ((1+iy) >= 0 && (1+iy) < src->getYDim() &&
					(i+iz) >= 0 && (i+iz) < src->getZDim()) {
				
				valid[2][i+1] = true;

				// add the run to the ranges -
				// MUST use all runs regardless of D2 validity because
				// any non common voxels count here
				possibleD2Len += src->getScanlineRuns(&(possibleD2[possibleD2Len]),
																							iy+1, iz+i);
			} else {
				valid[2][i+1] = false;
			}
		}
		
		// now look for valid distances
		for (int i=-1; i<=1; i++) {
			for (int j=-1; j<=1; j++) {
				if (valid[j+1][i+1]) {		
					if (src->validD2Distance(j+iy, i+iz)) {
						validD2 = true;
						tempScanlines[inRangeLines++] = scanlines[j+1][i+1];
					}
				}
			}
		}
		
		if (!validD2) {
			lastZ = lastY = -3;
			possRangeLength = 0;
			return false;
		}

		// get the new range
		for (int i=0; i<possRangeLength; i+=2) {
			possibleD2[possibleD2Len++] = possRange[i] << 1;
			possibleD2[possibleD2Len++] = (possRange[i+1] << 1) + 1;
		} 

		GetValidRange(possibleD2, possibleD2Len, tempRange, &tempRangeLen);

		// get the scanlines for this range
		for (int i=-1; i<=1; i++) {
			if ((1+iy) >= 0 && (1+iy) < src->getYDim() &&
					(i+iz) >= 0 && (i+iz) < src->getZDim()) {
				src->getScanlineInRange(scanlines[2][i+1], iy+1, iz+i,
																tempRange, tempRangeLen);
			}
		}

		// use the difference of the two to fill in the empty spots
		GetDifference(possRange, possRangeLength, tempRange, tempRangeLen,
									possibleD2, &possibleD2Len);

		ActVoxelElem *temp = new ActVoxelElem[length];
		
		for (int j=0; j<2; j++) {	
			for (int k=0; k<3; k++) {
				if (valid[j][k]) {

					for (int i=0; i<possibleD2Len; i+=2) {
						// copy over empty values
						for (int l=possibleD2[i]; l<possibleD2[i+1]; l++) {
							scanlines[j][k][l] = commonElement;
						}
					}
				}
			}
		} 
		
		delete [] temp;
		
		my_swap(tempRange, possibleD2);
		my_swap(tempRangeLen, possibleD2Len);
	}
	
	lastY = iy;
	lastZ = iz;
	
	// save the range of the length - using possibleD2 right now
	// as storage for what will become possRange
	possRangeLength = possibleD2Len;

	bool found;
	bool start = true;
	int possibleIndex = 0;

	// go over range and get final d2 range
	for (int i=0; i<possibleD2Len; i+=2) {
		int rangeStart = possibleD2[i];
		int rangeEnd = possibleD2[i+1];
		
		start = true;

		for (int j=rangeStart+1; j<rangeEnd-1; j++) {
			found = false;
			
			for (int k=0; k<inRangeLines; k++) {
				if (tempScanlines[k][j].d2 < d2limit && start)
					found = true;
			}
			
			if (start && found) {
				// starting point is one less than the current voxel
				possRange[possibleIndex++] = (j-1 == rangeStart) ? j : j-1;
				start = false;
			}
			if (!start && !found) {
				if (j+1 < rangeEnd - 2) {
					possRange[possibleIndex++] = j+1;
					j++; // to avoid any overlap
				} else {
					possRange[possibleIndex++] = rangeEnd-2;
				}

				start = true;
			}
		}
		
		// just in case we didn't find an end		
		if (!start)
			possRange[possibleIndex++] = rangeEnd - 2;
	}

	possRange[possibleIndex] = -1;
	my_swap(possRange, possibleD2);
	
	return (possibleD2[0] != -1);
}

int D2Compare(const void *elem1, const void *elem2)
{
	return ((*(int *)elem1) - (*(int *)elem2));
}

void GetValidRange(int *possD2List, int possD2Length, int *result, int *resLen)
{
	qsort(possD2List, possD2Length, sizeof(int), D2Compare);
	
	int count = 0;
	int start = -1, end;
	int resultIndex = 0;
	
	// now find outputted value
	for (int i=0; i<possD2Length; i++) {
		if (!(possD2List[i] & 0x1)) {
			if  (start == -1) {
				start = possD2List[i] >> 1;
			}
			
			count++;
		} else {
			count--;
			
			if (count==0) {
				end = possD2List[i] >> 1;
				result[resultIndex++] = start;
				result[resultIndex++] = end;
				start = -1;
			}
		}
	}
	
	result[resultIndex] = -1;
	*resLen = resultIndex;
}

void GetDifference(int *range, int rangeLen, int *fullRange, 
									 int fullRangeLen, int *result, int *resultLen)
{
	int i;
	int rangeIndex = 0;
	int resIndex = 0;

	for (i=0; i<fullRangeLen; i+=2) {
		int rangeStart = fullRange[i];
		int rangeEnd = fullRange[i+1];
		
		if (rangeIndex >= rangeLen) break;
		
		if (range[rangeIndex] <= rangeEnd) {
			while (!(range[rangeIndex] >= rangeStart && 
							 range[rangeIndex] <= rangeEnd))
				rangeIndex++;
			
			// check whether first point is in difference from start
			if (range[rangeIndex] != rangeStart)
				result[resIndex++] = rangeStart;
			
			// check middle points
			while (rangeIndex < rangeLen && range[rangeIndex] < rangeEnd) {
				if (range[rangeIndex] != rangeStart &&
						range[rangeIndex] != rangeEnd)
					result[resIndex++] = range[rangeIndex];
				
				rangeIndex++;
			}
			
			// check end points
			if (range[rangeIndex] != rangeEnd)
				result[resIndex++] = rangeEnd;
			else
				rangeIndex++;
		} else {
			// add these points
			result[resIndex++] = rangeStart;
			result[resIndex++] = rangeEnd;
		}
	}
	
	// add the end run points if applicable
	if (i!=fullRangeLen) {
		for (;i<fullRangeLen; i+=2) {
			result[resIndex++] = fullRange[i];
			result[resIndex++] = fullRange[i+1];
		}
	}
	
	if (resIndex % 2 == 1) cerr << "odd value - not good" << endl;
	
	*resultLen = resIndex;
}















