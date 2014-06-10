// 
// undo.h:
// Save every variable that changes, so that we can exactly "undo"
// an operation by putting everything back the way it was.  This
// allows us to try mesh simplification changes, and abort the
// changes if we detect something bad happening (like a topology
// change...)

#ifndef UNDO_H
#define UNDO_H


#define MAXUNDOBUFSIZE 40000

// Variables for storing the undo operation list
extern bool saveUndo;
extern char  undoBuf[sizeof(void *)*MAXUNDOBUFSIZE];
extern char *undoPtr;
extern void *undoLoc[MAXUNDOBUFSIZE];
extern int  undoSize[MAXUNDOBUFSIZE];
extern int undoN;

// Functions
template<class T> void save(T &data)
{
  if (!saveUndo) return;
  void *ptr = &data;
  if (sizeof(T) + undoPtr > MAXUNDOBUFSIZE + undoBuf) {
    // Error checking:
    // Shit.  We don't have enough buffer to push this on undo stack
    // Really should make it dynamically allocated. @!#$!#!@#
    fprintf(stderr, "Error.  Out of memory, can't push object of size");
    fprintf(stderr, "        %d on the undo stack.\n", sizeof(T));
    fprintf(stderr, "        Recommend increasing MAXUNDOBUFSIZE. :-( \n");
    exit(-1);
  }
  bcopy((char *) ptr, undoPtr, sizeof(T));
  undoPtr += sizeof(T);
  undoLoc[undoN] = &data;
  undoSize[undoN] = sizeof(T);
  undoN++;
}

void SaveCheckpoint(void);
void undo(void);
void SaveOff(void);

#endif  // UNDO_H
