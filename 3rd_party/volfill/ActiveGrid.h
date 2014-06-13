#ifndef ACTIVEGRID_H
#define ACTIVEGRID_H

#include "OccGridRLE.h"
#include "fb.h"

typedef enum {EMPTY=0, MODIFYABLE=1, FILLED=2} voxelType;

struct ActVoxelElem
{
	public:
unsigned short value;
	
	// d1 is inner distance, d2 is farther distance
	// because d1 should be closer to the active voxels, it's 2
	// most significant bits are used to store whether the voxel
	// is valid or not - meaning it has a value that should
	// be used during bluring
	unsigned char d2;

	inline unsigned char getD1() { return d1 & 0x3F; }
	inline unsigned char getType() { return d1 >> 6; }
	inline void setD1(unsigned char d1new) { d1 = (d1new & 0x3F) + (d1 & 0xC0); }  
	inline void setType(unsigned char newType) { d1 = (d1 &0x3F) + (newType << 6); } 
		
	// copy constructor
	const ActVoxelElem &operator= (const ActVoxelElem &rhs);
	bool inline operator== (const ActVoxelElem &rhs);
	bool inline operator!= (const ActVoxelElem &rhs);
	
 private:
	unsigned char d1; // just to prevent any mistaken indexing
};

class ActVoxelScanline
{
 public:
	unsigned short *run;
	ActVoxelElem *elements;

	int runLength, elemLength;
	int length;
	
	// if any of the values are within d2 range, this
	// will be true, else false
	bool anyValidElements; 

	void copy(ActVoxelScanline *copyLine);
	
	ActVoxelScanline() : elements(NULL), run(NULL), length(0), anyValidElements(false) {}
	~ActVoxelScanline() { delete [] elements; }
	
	// gets and puts a scanline
	void putScanline(ActVoxelElem *scanline, int length, unsigned short d2);
	void putScanlineInRange(ActVoxelElem *scanline, int length, unsigned
													short d2, int *range, int rangeLen);
	void getScanline(ActVoxelElem *scanline);
	void getScanlineInRange(ActVoxelElem *scanline, int *range, int length);
	
	int getScanlineRuns(int *validRun);
	
	void reset();
};

class ActVoxelGrid
{
 public:
	ActVoxelGrid();
	ActVoxelGrid(int dx, int dy, int dz);

	~ActVoxelGrid();
	
	// Displays the voxel grid to the current frame buffer
	void draw(FrameBuffer *fb, int sweep, int type, int d1, int d2);

	// loads and saves the voxel grid from an OccGrid
	void loadFromOccGridRLE(OccGridRLE *ogSrc, unsigned char maxD2Dist, bool conserveMemory = false);
	void saveToOccGridRLE(OccGridRLE *ogSrc, OccGridRLE *ogDst);
	
	// Initialize distances
	void initDistances(unsigned char maxD2Dist, bool propD2);

	// Reset the memory
	void reset();

	// blur, update distances, etc...
	void blur(unsigned char d1limit, unsigned char d2limit);
	
	void getScanline(ActVoxelElem *scanline, int iy, int iz) {
		scanlines[iz][iy].getScanline(scanline); 
	}
	
	void getScanlineInRange(ActVoxelElem *scanline, int iy, int iz, int
													*validRange, int validLen) {
		scanlines[iz][iy].getScanlineInRange(scanline, validRange, validLen);
	}
	
	int getScanlineRuns(int *values, int iy, int iz) {
		return scanlines[iz][iy].getScanlineRuns(values);
	}
	
	bool validD2Distance(int iy, int iz) {
		return scanlines[iz][iy].anyValidElements;
	}
	
	inline int getXDim() { return xdim; }
	inline int getYDim() { return ydim; }
	inline int getZDim() { return zdim; }
 private:
	// Propogate distance function
	void PropogateD2Distances(unsigned char maxD2Dist);
	void PropogateD2DistancesTest(unsigned char maxD2Dist);
	void saveSpace(unsigned char maxD2Dist);
	
	ActVoxelScanline **scanlines;
	
	bool firstPass;
	
	int xdim, ydim, zdim;
	bool conserveMemory;
	
	friend class SweepLine;
};

// utility class to help with blurring/distances/etc...
class SweepLine
{
 public:
	static const int SWEEP_WIDTH = 3;

	// creates a widthxwidth 2d array of scanlines
	SweepLine(int xdim) {
		possRangeLength = 0;

		length = xdim;
		
		for (int i=0; i<SWEEP_WIDTH; i++) {
			for (int j=0; j<SWEEP_WIDTH; j++) {
				scanlines[i][j] = new ActVoxelElem[xdim];
				valid[i][j] = false;
			}
		}
		
		possibleD2 = new int[18 * xdim];
		possRange = new int[18 * xdim]; // 9 lines * worst case 2 as many
		tempRange = new int[18 * xdim]; // 3 new lines at most
		lastY = lastZ = -3;
	}

	// free the memory associated with a sweepline
	~SweepLine() {
		for (int i=0; i<SWEEP_WIDTH; i++) {
			for (int j=0; j<SWEEP_WIDTH; j++) {
				delete [] scanlines[i][j];
			}
		}
		
		if (possibleD2)
			delete [] possibleD2;
		if (possRange)
			delete [] possRange;
		if (tempRange)
			delete [] tempRange;
	}
	
	void getScanlines(ActVoxelGrid *src, int iy, int iz, bool validD2 = false) {
		for (int i=-1; i<=1; i++) {
			for (int j=-1; j<=1; j++) {
				if ((j+iy) >= 0 && (j+iy) < src->getYDim() &&
						(i+iz) >= 0 && (i+iz) < src->getZDim() &&
						(!validD2 || src->validD2Distance(iy+j, iz+i))) {
					valid[j+1][i+1] = true;
					src->getScanline(scanlines[j+1][i+1], iy+j, iz+i);
				} else {
					valid[j+1][i+1] = false;
				}
			}
		}
	}
	
	bool getScanlinesD2Info(ActVoxelGrid *src, int iy, int iz, unsigned
													char d2limit);
	
	ActVoxelElem *scanlines[3][3];
	bool valid[3][3];
	
	int *possRange;
	int possRangeLength;

	int *possibleD2;
	
	int *tempRange;
	int tempRangeLen;
	
	int length;
	
	int lastY, lastZ;
};

#endif 


