// 
// undo.cc:
// Save every variable that changes, so that we can exactly "undo"
// an operation by putting everything back the way it was.  This
// allows us to try mesh simplification changes, and abort the
// changes if we detect something bad happening (like a topology
// change...)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ply.h>
#include <Linear.h>
#include <string.h>
#include <strings.h>

#include "Mesh.h"
#include "plyio.h"
#include "undo.h"



// Buffer stuff to remember, undo.

int google = 3;
int google22 = 3;

// Variables for storing the undo operation list
bool saveUndo = FALSE;
char  undoBuf[sizeof(void *)*MAXUNDOBUFSIZE];
char *undoPtr;
void *undoLoc[MAXUNDOBUFSIZE];
int  undoSize[MAXUNDOBUFSIZE];
int undoN = 0;

// Variables for saving calls to delete().  We don't actually
// delete until we see the commit...
// (not implemented yet)
void *undoFree[MAXUNDOBUFSIZE];
int   undoFreeN = 0;

void undo(void)
{
  // Undo all the saves, in backward order
  while (--undoN >= 0) {
    undoPtr -= undoSize[undoN];	
    bcopy(undoPtr, (char *) undoLoc[undoN], undoSize[undoN]);
  }

  // Turn saving off, until we checkpoint again
  SaveOff();
  
}

void SaveCheckpoint(void)
{
  // Commit previous operations -- delete anything sitting around...
  // (not implemented yet)
  undoFreeN = 0;

  // Initialize variables to start saving again...
  saveUndo = TRUE;
  undoN = 0;
  undoPtr = undoBuf;

}


void SaveOff(void)
{
  // Turn off saving 
  saveUndo = FALSE;
  
  undoFreeN = 0;
  undoN = 0;
  undoPtr = undoBuf;
}
