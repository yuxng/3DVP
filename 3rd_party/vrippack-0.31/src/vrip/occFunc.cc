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


#include <math.h>

#include "occFunc.h"
#include "vripGlobals.h"

void
initOccFunc()
{
    float factor = pow(2, MeshResolution-1);

    MAX_DEPTH_DIFFERENCE = MaxEdgeLength;

    D1 = OccupancyRampWidth/2;
    D2 = -OccupancyRampWidth/2;

    C1 = WeightPos1;
    C2 = WeightPos2;

    C3 = WeightPos3;
    C4 = WeightPos4;
    C5 = WeightPos5;

    // Careful about choosing this number.  Should be something like
    //  MAX_STRETCH = (C5-bufferDist)/C4;
    MAX_STRETCH = 2.5;
    //MAX_C5 = MAX_STRETCH*C5;

    M_WEIGHT_1 = 1/(C2-C1);
    B_WEIGHT_1 = -C1/(C2-C1);

    M_WEIGHT_2 = 1/(C3-C4);
    B_WEIGHT_2 = -C4/(C3-C4);

    M_VALUE = 1/(D2-D1);
    B_VALUE = -D1/(D2-D1);
}

#if 1

void
updateCell(float cellDepth, float rangeDepth, float sampleSpacing,
	   float rangeConfidence, OccElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;

//    expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ > C1 || deltaZ < NEW_C5) {
	weight = 0;
	return;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {
	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + B_WEIGHT_2);
    }

    weight = MAX(weight, MinWeight/255.0);

    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }

    totalWeight = double(cell->totalWeight);

    weight *= UCHAR_MAX;
    currentValue = double(cell->value);
    value *= USHRT_MAX;

    currentValue = currentValue*totalWeight + value*weight;
    totalWeight += weight;
    if (totalWeight != 0) {
	currentValue /= totalWeight;
    } else {
	currentValue = value;
    }
    
    cell->value = ushort(currentValue);
    cell->totalWeight = ushort(totalWeight);
    
    return;
}


#else

void
updateCell(float cellDepth, float rangeDepth, float sampleSpacing,
	     float rangeConfidence, OccElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;

//    expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ > C1 || deltaZ < NEW_C5) {
	weight = 0;
	return;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {
	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + B_WEIGHT_2);
    }

    weight = MAX(weight, MinWeight/255.0);

    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }


    value = 4*(value-0.5)*(value-0.5);
    totalWeight = double(cell->totalWeight);

    weight *= UCHAR_MAX;
    currentValue = double(cell->value);
    value *= USHRT_MAX;

    currentValue = currentValue*totalWeight + value*weight;
    totalWeight += weight;
    if (totalWeight != 0) {
	currentValue /= totalWeight;
    } else {
	currentValue = value;
    }
    
    cell->value = ushort(currentValue);
    cell->totalWeight = ushort(totalWeight);
    
    return;
}

#endif


// Bias to emptiness if the input cell has low confidence

void
updateCellBiasEdgesEmpty(float cellDepth, float rangeDepth, 
			 float sampleSpacing,
			 float rangeConfidence, 
			 float edgeConfidence,
			 OccElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE, NEW_B_WEIGHT_2;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;
    int isFalseData, wasFalseData;

    //expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;


    if (edgeConfidence < 0)
	isFalseData = 1;
    else
	isFalseData = 0;

//    edgeConfidence = edgeConfidence*edgeConfidence;
    edgeConfidence = MAX(MinEdgeConfidence/255.0, edgeConfidence);

    if (isFalseData)
	edgeConfidence = 0;

/*
    if (isFalseData)
	return;
	*/

//    edgeConfidence = 10/256.0;
//    edgeConfidence = 1;

    NEW_C3 = edgeConfidence*NEW_D2 + (1-edgeConfidence)*NEW_D1;

    NEW_C4 = edgeConfidence*(NEW_D2 - (NEW_D1 - NEW_D2)/2);

/*
    NEW_C3 = (0.5*edgeConfidence+0.5)*NEW_D2 + 
	(1-(0.5*edgeConfidence+0.5))*NEW_D1;
    NEW_C4 = (0.5*edgeConfidence+0.5)*(NEW_D2 - (NEW_D1 - NEW_D2)/2);

    NEW_C3 = rangeConfidence*NEW_D2 + (1-rangeConfidence)*NEW_D1;
    NEW_C4 = rangeConfidence*(NEW_D2 - (NEW_D1 - NEW_D2)/2);
    */

    NEW_M_WEIGHT_2 = 1/(NEW_C3-NEW_C4);
    NEW_B_WEIGHT_2 = -NEW_C4/(NEW_C3-NEW_C4);
    

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ > C1 || deltaZ < NEW_C5) {
	weight = 0;
	return;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {
	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + NEW_B_WEIGHT_2);
    }

    if (!isFalseData)
	weight = MAX(weight, MinWeight/255.0);


    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }


//    weight = MIN(1.0, weight+minWeight);
/*
    if (weight == 0)
	return;
	*/

    wasFalseData = cell->totalWeight & OccGridRLE::FALSE_DATA_BIT
	|| cell->totalWeight == 0;

    totalWeight = double(cell->totalWeight & ~OccGridRLE::FALSE_DATA_BIT);

    weight *= UCHAR_MAX;
    currentValue = double(cell->value);
    value *= USHRT_MAX;


//    if (weight > 30 || !isEdge) {
    if (weight > -1 || totalWeight > -1) {
	if (totalWeight + weight != 0) {
	    currentValue = currentValue*totalWeight + value*weight;
	    totalWeight += weight;
	    currentValue /= totalWeight;
	} else {
	    currentValue = MIN(value, currentValue);	   
	}
//	isEdge = FALSE;
    }
    else {
	if (totalWeight != 0) {
	    totalWeight = MAX(totalWeight, weight);

	    // Min rule
	    //currentValue = MIN(currentValue, value);

	    // Geometric mean
	    //currentValue = sqrt(value*currentValue);

	    // u-name-it: (1+x-y)/2*y + (1+y-x)/2*x; x,y between 0,1
	    currentValue /= USHRT_MAX;
	    value /= USHRT_MAX;
	    currentValue = ((1+value-currentValue)*currentValue/2 +
		(1-value+currentValue)*value/2);
	    currentValue *= USHRT_MAX;

	} else {
	    totalWeight = weight;
	    currentValue = value;
	}
    }
    
    cell->value = ushort(currentValue);

    cell->totalWeight = ushort(totalWeight);

    if (isFalseData && wasFalseData) {
	cell->totalWeight = cell->totalWeight | OccGridRLE::FALSE_DATA_BIT;
    }
   
    return;
}


void
updateCellForCarving(float cellDepth, float rangeDepth, float sampleSpacing,
		     float rangeConfidence, OccElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;

    //expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ < NEW_C5) {
	weight = 0;
	return;
    } else if (deltaZ > C1) {
	weight = 0;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {

	// To avoid overwriting good data??
	//weight = 1/255.0;

	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + B_WEIGHT_2);
    }

//    weight = MAX(weight, MinWeight/255.0);

    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }

    totalWeight = double(cell->totalWeight);
    currentValue = double(cell->value);

    if (weight == 0 && totalWeight == 0) {
	currentValue /= USHRT_MAX;
	value *= currentValue;
	value *= USHRT_MAX;
	cell->value = ushort(value);
    } else {    
	weight *= UCHAR_MAX;
	value *= USHRT_MAX;

	currentValue = currentValue*totalWeight + value*weight;
	totalWeight += weight;
	if (totalWeight != 0) {
	    currentValue /= totalWeight;
	} else {
	    currentValue = value;
	}
    
	cell->value = ushort(currentValue);
	cell->totalWeight = ushort(totalWeight);
    }
    return;
}


void
updateCell(float cellDepth, float rangeDepth, Vec3f &norm, 
	   float sampleSpacing, float rangeConfidence, OccNormElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;
    Vec3f curNorm1, curNorm2, curNorm3, curNorm4;

    //expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ > C1 || deltaZ < NEW_C5) {
	weight = 0;
	return;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {
	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + B_WEIGHT_2);
    }

    weight = MAX(weight, MinWeight/255.0);

    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }


#if 0

    totalWeight = double(cell->totalWeight);
    currentValue = double(cell->value);

    if (weight == 0 && totalWeight == 0) {
	currentValue /= USHRT_MAX;
	value *= currentValue;
	value *= USHRT_MAX;
	cell->value = ushort(value);
    } else {    
	weight *= UCHAR_MAX;
	value *= USHRT_MAX;

	currentValue = currentValue*totalWeight + value*weight;
	totalWeight += weight;
	if (totalWeight != 0) {
	    currentValue /= totalWeight;
	} else {
	    currentValue = value;
	}
    
	cell->value = ushort(currentValue);
	cell->totalWeight = ushort(totalWeight);
    }
    return;

#else

    Vec3f curNorm;
    if (weight != 0) {
	weight *= UCHAR_MAX;
	currentValue = double(cell->value);
	value *= USHRT_MAX;
	totalWeight = double(cell->totalWeight);
	if (totalWeight == 0) {
	    cell->totalWeight = ushort(weight);
	    cell->value = ushort(value);
	    cell->nx = (signed char)(126*norm.x);
	    cell->ny = (signed char)(126*norm.y);
	    cell->nz = (signed char)(126*norm.z);

	    return;
	} 

	curNorm.setValue(cell->nx, cell->ny, cell->nz);
	if (curNorm.dot(norm) > 0) {
	    curNorm.normalize();
	    curNorm *= totalWeight;
	    norm *= weight;
	    curNorm += curNorm;
	    curNorm.normalize();
	    currentValue = currentValue*totalWeight + value*weight;
	    totalWeight += weight;
	    currentValue /= totalWeight;

	    cell->value = ushort(currentValue);
	    cell->totalWeight = ushort(totalWeight);
	    cell->nx = (signed char)(126*curNorm.x);
	    cell->ny = (signed char)(126*curNorm.y);
	    cell->nz = (signed char)(126*curNorm.z);
	} else {
	    if (currentValue > value) {
		cell->value = ushort(value);
		cell->totalWeight = ushort(weight);
		cell->nx = (signed char)(126*norm.x);
		cell->ny = (signed char)(126*norm.y);
		cell->nz = (signed char)(126*norm.z);
	    }
	}
    }

#endif

}

void
updateCellForCarving(float cellDepth, float rangeDepth, float sampleSpacing,
		     float rangeConfidence, OccNormElement *cell)
{
    float deltaZ, NEW_M_WEIGHT_2, NEW_M_VALUE;
    float NEW_C3, NEW_C4, NEW_C5, NEW_D1, NEW_D2;
    double weight, value, totalWeight, currentValue;
    float expansion;

    //expansion = (1-MAX_STRETCH)*rangeConfidence + MAX_STRETCH;
    expansion = 1;

    NEW_M_WEIGHT_2 = M_WEIGHT_2/expansion;
    NEW_M_VALUE = M_VALUE/expansion;
    NEW_C3 = C3*expansion;
    NEW_C4 = C4*expansion;
    NEW_C5 = C5;
    NEW_D1 = D1*expansion;
    NEW_D2 = D2*expansion;

    deltaZ = cellDepth - rangeDepth;
    deltaZ *= sampleSpacing;

    if (deltaZ < NEW_C5) {
	weight = 0;
	return;
    } else if (deltaZ > C1) {
	weight = 0;
    }
    else if (deltaZ > NEW_C5 && deltaZ < NEW_C4) {

	// To avoid overwriting good data??
	//weight = 1/255.0;

	weight = 0;
    }
    else if (deltaZ > NEW_C3 && deltaZ < C2) {
	weight = rangeConfidence;
    }
    else if (deltaZ > C2) {
	weight = rangeConfidence*(M_WEIGHT_1*deltaZ + B_WEIGHT_1);
    }
    else {
	weight = rangeConfidence*(NEW_M_WEIGHT_2*deltaZ + B_WEIGHT_2);
    }

    if (deltaZ > NEW_D1) {
	value = 0;
    }
    else if (deltaZ < NEW_D2) {
	value = 1;
    }
    else {
	value = NEW_M_VALUE*deltaZ + B_VALUE;
    }

    totalWeight = double(cell->totalWeight);
    currentValue = double(cell->value);

    if (weight == 0 && totalWeight == 0) {
	currentValue /= USHRT_MAX;
	value *= currentValue;
	value *= USHRT_MAX;
	cell->value = ushort(value);
    } else {    
	weight *= UCHAR_MAX;
	value *= USHRT_MAX;

	currentValue = currentValue*totalWeight + value*weight;
	totalWeight += weight;
	if (totalWeight != 0) {
	    currentValue /= totalWeight;
	} else {
	    currentValue = value;
	}
    
	cell->value = ushort(currentValue);
	cell->totalWeight = ushort(totalWeight);
    }
    return;
}


