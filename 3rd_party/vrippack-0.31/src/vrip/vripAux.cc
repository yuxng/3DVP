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


#include "vripAux.h"
#include "vripGlobals.h"


void
expandVaryingFromConstant(OccGridRLE *inGrid, OccGridRLE *outGrid)
{
    int i;
    RunLength length, len;
    int runType;
    OccElement *element;
    
    outGrid->copyParams(inGrid);	
    outGrid->reset();
    for (int zz = 0; zz < inGrid->zdim; zz++) {
        for (int yy = 0; yy < inGrid->ydim; yy++) {
	    inGrid->setScanline(yy,zz);
	    outGrid->allocNewRun(yy,zz);

	    while (TRUE) {
		length = inGrid->getNextLength();
		runType = inGrid->getRunType(&length);

		if (runType == OccGridRLE::END_OF_RUN) {
	            inGrid->setRunType(&length, OccGridRLE::END_OF_RUN);
		    outGrid->putNextLength(length);
		    break;
		}

		if (runType == OccGridRLE::CONSTANT_DATA) {
		    element = inGrid->getNextElement();
		    len = 1;
		    inGrid->setRunType(&len, OccGridRLE::VARYING_DATA);
		    outGrid->putNextLength(len);
		    outGrid->putNextElement(element);

		    if (length > 2) {
			len = length - 2;
			inGrid->setRunType(&len, OccGridRLE::CONSTANT_DATA);
			outGrid->putNextLength(len);
			outGrid->putNextElement(element);
		    }

		    if (length >= 2) {
			len = 1;
			inGrid->setRunType(&len, OccGridRLE::VARYING_DATA);
			outGrid->putNextLength(len);
			outGrid->putNextElement(element);
		    }
		}
		else {
		    len = length;
		    inGrid->setRunType(&len, OccGridRLE::VARYING_DATA);
		    outGrid->putNextLength(len);
		    for (i=0; i<length; i++) {
			element = inGrid->getNextElement();
			outGrid->putNextElement(element);
		    }
		}
	    }
	}
    }
}


void
swapgrids()
{
    OccGridRLE *tempRLE;
    tempRLE = backRLEGrid;
    backRLEGrid = frontRLEGrid;
    frontRLEGrid = tempRLE;

    OccGridNormRLE *tempNormRLE;
    tempNormRLE = backRLEGridNorm;
    backRLEGridNorm = frontRLEGridNorm;
    frontRLEGridNorm = tempNormRLE;
}

