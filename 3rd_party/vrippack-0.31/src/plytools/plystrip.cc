/*
 * code to strip triangles in a binary PLY file
 * assumes PLY file contains two elements: vertex and face data
 * replaces face data with tri-strip-count and tri-strip elements
 * becomes unhappy if any of the faces have >3 vertices
 *
 * Matt Ginzton (magi@graphics), 1/21/98
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <ply.h>
#include <cassert>


enum ErrorCode {
  ERR_NONE,
  ERR_PARAMS,
  ERR_MISSINGINFILE,
  ERR_INVALIDELEMENT,
  ERR_MISSINGELEMENT,
  ERR_NOTALLTRIANGLES,
  ERR_FILE,
};




typedef void PlyVertex;    /* we'll treat these as opaque for simplicity
			      and read speed*/


#undef BYTE_ALIGN_FACES


#ifdef BYTE_ALIGN_FACES
#pragma pack (1)
#endif
//need this struct to be byte-aligned to match what's on disk
//if it's not, ReadFaces does the conversion, which is slower.
//Because STL vector class doesn't appear to like unaligned data
//(to the tune of seg-faulting).
typedef struct PlyFace
{
    unsigned char nverts;
    int verts[3];
} Face;
#ifdef BYTE_ALIGN_FACES
#pragma pack (0)
#endif

typedef struct PlyStrip
{
  int length;
  int* vertices;
} Strip;


ErrorCode ReadVerticesAndFaces (char* pszPlyFile,
				PlyVertex*& ppVertices, int& pnVertices,
				int& pcbVertice, std::vector<int>& faces,
				bool& bHaveStrips, bool bForce);
ErrorCode tris_to_strips (int nvertices, const std::vector<int>& tris,
			  std::vector<int>& strips);
ErrorCode strips_to_tris(const std::vector<int> &tstrips,
			 std::vector<int>& tris, int nTris = 0);
ErrorCode WriteStrippedPlyFile (char* pszInputPlyFile, char* pszOutputFile,
				PlyVertex* pVertices, int nVertices,
				int cbVertice, std::vector<int>& strips,
				bool bStripped);
FILE* OpenHeadedCopy (char* pszInputPlyFile, char* pszOutputFile);
bool ReadVertices (PlyFile* ply, int nVertices, PlyVertex*& pVertices,
		   int& cbVertice);
bool ReadFaces (PlyFile* ply, int nFaces, std::vector<int>& faces);
bool ReadStrips (PlyFile* ply, int nFaces, std::vector<int>& faces);
bool ReadLine (FILE* file, char* buffer);

// ugly global added by lucas
bool bQuiet = false;


int main (int argc, char **argv)
{
  ErrorCode error;
  PlyVertex* pVertices;
  int      nVertices;
  int      cbVertice;
  std::vector<int> faces;
  std::vector<int> strips;
  std::vector<int>* data;
  char* pszInput = NULL;
  char* pszOutput = NULL;
  bool     bWantStrips = true;
  bool     bForce = false;
  bool     bHaveStrips;

  if (argc > 1 && !strcmp (argv[1], "-h")) {
    std::cerr << "Usage: plystrip infile outfile" << std::endl;
    std::cerr << "plystrip will strip things that aren't, "
      "and unstrip things that are, stripped." << std::endl;
    std::cerr << "-s will force strip; -u will force unstrip" << std::endl;
    std::cerr << "-q runs in quiet mode." << std::endl;
    return ERR_PARAMS;
  }

  // Parse -args
  while (argc > 1 && argv[1][0] == '-') {
    if (!strcmp (argv[1], "-u") || !strcmp (argv[1], "-s")) {
      bWantStrips = argv[1][1] == 's';
      bForce = true;
      ++argv;
      --argc;
    } else if (!strcmp (argv[1], "-q")) {
      bQuiet = true;
      ++argv;
      --argc;
    } else {
      std::cerr << "Error:  Unhandled argument " << argv[1] << "." << std::endl;
      return ERR_PARAMS;
    }
  }	

  if (argc > 1 && argv[1])
    pszInput = argv[1];

  if (argc > 2 && argv[2])
    pszOutput = argv[2];

  if (!bQuiet) std::cerr << "Reading input file " << pszInput << "... " << std::flush;
  error = ReadVerticesAndFaces (pszInput, pVertices, nVertices, cbVertice,
				faces, bHaveStrips, bForce);
  if (!bQuiet) std::cerr << "done." << std::endl;

  if (!bForce)
    bWantStrips = !bHaveStrips;

  switch (error) {
  case ERR_NONE:
    if (bWantStrips == bHaveStrips) {
      if (!bQuiet) {
	std::cerr << "Doing nothing really fast, "
	     << "because already in correct format... "
	     << "done." << std::endl;
      }
      data = &faces;
    } else {
      if (bWantStrips)
	error = tris_to_strips (nVertices, faces, strips);
      else
	error = strips_to_tris (faces, strips);
      if (ERR_NONE != error)
	return error;
      data = &strips;
    }
    break;

  default:
    return error;
  }

  if (!bQuiet) std::cerr << "Writing output file " << std::flush;
  error = WriteStrippedPlyFile (pszInput, pszOutput, pVertices,
				nVertices, cbVertice, *data, bWantStrips);
  if (!bQuiet) std::cerr << "done." << std::endl;
  if (ERR_NONE != error)
    return error;
  
  return ERR_NONE;
}


ErrorCode ReadVerticesAndFaces (char* pszPlyFile,
				PlyVertex*& pVertices, int& nVertices,
				int& cbVertice, std::vector<int>& faces,
				bool& bHaveStrips, bool bForce)
{
  PlyFile* ply;
  int nElems;
  char** pElemList;
  int file_type;
  float version;
  int i;

  pVertices = NULL;

  FILE* inFile;
  if (!pszPlyFile || 0 == strcmp (pszPlyFile, "-"))
    inFile = stdin;
  else
    inFile = fopen (pszPlyFile, "r");

  if (!inFile) {
    std::cerr << "input file does not exist" << std::endl;
    return ERR_MISSINGINFILE;
  }

  ply = ply_read (inFile, &nElems, &pElemList);
  if (!ply) {
    fclose (inFile);
    std::cerr << "input can not be read as a ply file" << std::endl;
    return ERR_FILE;
  }

  ply_get_info (ply, &version, &file_type);

  /*
  std::cerr << "ply file " << argv[1] << " is type " << file_type
  << " version " << version << "; " << nElems << " elements." << std::endl;
  */

  if (nElems != 2)
    {
      std::cerr << "Only expected 2 elements (vertex and face)." << std::endl;
      return ERR_INVALIDELEMENT;
    }

  for (i = 0; i < nElems; i++)
    {
      char* elem_name = pElemList[i];
      int nInstances;
      int nProps;
      PlyProperty** plist = ply_get_element_description (ply, elem_name,
					     &nInstances, &nProps);

      if (i == 0 && !strcmp (elem_name, "vertex"))
	{
	  nVertices = nInstances;
	  if (!ReadVertices (ply, nVertices, pVertices, cbVertice))
	    {
	      std::cerr << "Invalid format." << std::endl;
	      return ERR_INVALIDELEMENT;
	    }
	}
      else if (i == 1 && !strcmp (elem_name, "face"))
	{
	  bHaveStrips = false;
	  if (!ReadFaces (ply, nInstances, faces)) {
	    std::cerr << "Not all faces are triangles, aborting" << std::endl;
	    ply_close (ply);
	    return ERR_NOTALLTRIANGLES;
	  }
	}
      else if (i == 1 && !strcmp (elem_name, "tristrips"))
	{
	  bHaveStrips = true;
	  if (!ReadStrips (ply, nInstances, faces)) {
	    std::cerr << "Error reading strip data" << std::endl;
	    ply_close (ply);
	    return ERR_INVALIDELEMENT;
	  }
	}
      else
	{
	  std::cerr << "Unknown element " << elem_name
	       << " (at pos " << i << "), aborting" << std::endl;
	  ply_close (ply);
	  return ERR_INVALIDELEMENT;
	}
    }

  ply_close (ply);

  if (!pVertices || !faces.size())
    {
      std::cerr << "Did not find both vertices and faces." << std::endl;
      return ERR_MISSINGELEMENT;
    }

  return ERR_NONE;
}


ErrorCode WriteStrippedPlyFile (char* pszInputPlyFile, char* pszOutputFile,
				PlyVertex* pVertices, int nVertices,
				int cbVertice, std::vector<int>& strips,
				bool bStripped)
{
  FILE* outfile;

  outfile = OpenHeadedCopy (pszInputPlyFile, pszOutputFile);
  if (outfile == NULL) {
    std::cerr << "The ply header could not be written." << std::endl;
    return ERR_PARAMS;
  }

  if (bStripped)
    fprintf (outfile, "element tristrips 1\n"
	     "property list int int vertex_indices\n");
  else
    fprintf (outfile, "element face %d\n"
	     "property list uchar int vertex_indices\n", strips.size() / 3);
  fprintf (outfile, "end_header\n");

  if (nVertices != fwrite (pVertices, cbVertice, nVertices, outfile)) {
    std::cerr << "Error writing vertices." << std::endl;
    fclose (outfile);
    return ERR_FILE;
  }

  if (bStripped) {
    int num = strips.size();
    fwrite (&num, sizeof(int), 1, outfile);
    fwrite (&strips[0], sizeof(int), num, outfile);
  } else {
    char c = 3;
    for (int i = 0; i < strips.size(); i += 3) {
      fwrite (&c, sizeof(char), 1, outfile);
      fwrite (&strips[i], sizeof(int), 3, outfile);
    }
  }

  fclose (outfile);

  return ERR_NONE;
}


FILE* OpenHeadedCopy (char* pszInputPlyFile, char* pszOutputFile)
{
  FILE* infile;
  FILE* outfile;

  if (pszOutputFile) {
    if (!bQuiet) std::cerr << pszOutputFile << "... " << std::flush;
    outfile = fopen (pszOutputFile, "w");
  } else {
    if (!bQuiet) std::cerr << "(stdout)... " << std::flush;
    outfile = stdout;
  }
   
  if (!pszInputPlyFile || 0 == strcmp (pszInputPlyFile, "-")) {
    infile = stdin;
    // BUGBUG this doesn't work: have to find some other way to read the
    // header twice from stdin
    rewind (infile);
  } else {
    infile = fopen (pszInputPlyFile, "r");

  }
  if (infile == NULL) {
    std::cerr << "Unable to open input file!" << std::endl;
    return NULL;
  }

  if (outfile != NULL) {
    char szLine[400];
    while (true) {
      
      if (!ReadLine (infile, szLine)) {
	std::cerr << "Read error on input!" << std::endl;
	fclose (outfile);
	outfile = NULL;
	break;
      }
      
      if (strncmp (szLine, "element face", 12)
	  && strncmp (szLine, "element tristrips", 17)) {
	// copy verbatim
	fprintf (outfile, szLine);
      } else {
	// we'll print the rest ourselves
	break;
      }
    }
  }
  
  fclose (infile);
  return outfile;
}


int CalcPropertySize (PlyElement* elem)
{
  int i;
  int size = 0;
  int type;

  int SizeOf[] = { 0, 1, 2, 4, 1, 2, 4, 4, 8 };
  for (i = 0; i < elem->nprops; i++)
    {
      type = elem->props[i]->external_type;
      if (type <= PLY_START_TYPE || type >= PLY_END_TYPE)
	{
	  std::cerr << "Unknown type " << type << " found for property "
	       << elem->props[i]->name << " in elem " << elem->name << std::endl;
	  return 0;
	}
      size += SizeOf[type];
    }

  return size;
}


bool ReadVertices (PlyFile* ply, int nVertices,
		   PlyVertex*& pVertices, int& cbVertice)
{
  PlyVertex* vchunk;
  int nr;

  /* for now, assume that vertices are first element in plyfile. */
  PlyElement* elem = ply->elems[0];
  if (strcmp (elem->name, "vertex"))
    {
      std::cerr << "Bad element name '" << elem->name << "'" << std::endl;
      return false;
    }
  if (elem->size > 0)
    cbVertice = elem->size;
  else
    cbVertice = CalcPropertySize (elem);

  vchunk = (PlyVertex*) malloc (cbVertice * nVertices);

  if (nVertices != (nr = fread (vchunk, cbVertice, nVertices, ply->fp)))
    {
      std::cerr << "Expected " << nVertices << " vertices, read " << nr << std::endl;
      return false;
    }

  pVertices = vchunk;
  return true;
}


bool ReadFaces (PlyFile* ply, int nFaces, std::vector<int>& faces)
{
  struct {
    unsigned char padding[3];
    unsigned char nv;
    int vert[3];
  } diskvert;

  assert (sizeof (diskvert) == 16);
  faces.reserve (nFaces * 3);

  for (int iF = 0; iF < nFaces; iF++) {
    if (1 != fread (((char*)&diskvert) + 3, sizeof(diskvert) - 3, 1, ply->fp))
      return false;
    
    assert (diskvert.nv == 3);
    if (diskvert.nv != 3)
      return false;
    
    for (int iV = 0; iV < 3; iV++)
      faces.push_back (diskvert.vert[iV]);
  }

  return true;
}


bool ReadStrips (PlyFile* ply, int nFaces, std::vector<int>& faces)
{
  int count;
  fread (&count, sizeof(count), 1, ply->fp);
  faces = std::vector<int> (count, 0);

  // Used to say:
  //  return (count == fread (&faces.begin(), sizeof(int), count, ply->fp));
  // but begin is an iterator.  On the other hand, is it legal to read a
  //  vector in this manner, using front or other?  B. Curless -- 8/15/05

  //  return (count == fread (&faces.begin(), sizeof(int), count, ply->fp));
  //  return (count == fread (&faces.front(), sizeof(int), count, ply->fp));
  return (count == fread (&(*(faces.begin())), sizeof(int), count, ply->fp));
}


bool ReadLine (FILE* file, char* buffer)
{
  int c;

  while ((c = getc(file)) != EOF)
    {
      *buffer++ = (char)c;
      if (c == '\n')
      {
        *buffer = 0; /*terminate*/
        return true;
      }
    }

  return false;
}




//////////////////////////////////////////////////////////////////////
//
// smr's tstrip code
//
//////////////////////////////////////////////////////////////////////

typedef int* adjacentfacelist;

adjacentfacelist*
TriMesh_FindAdjacentFaces (int numvertices,
			   const std::vector<int>& faces,
			   int*& numadjacentfaces)
{
  if (!bQuiet) std::cerr << "  Computing vtx->face mappings..." << std::flush;

  // Step I - compute numadjacentfaces
  numadjacentfaces = new int[numvertices];
  memset(numadjacentfaces, 0, numvertices*sizeof(int));

  int numfaces = faces.size();
  int i;
  for (i = 0; i < numfaces; i++) {
    numadjacentfaces[faces[i]]++;
  }

  // allocate one chunk of memory for all adjacent face lists
  // total number of adjacent faces is numfaces
  int* adjacentfacedata = new int [numfaces];
  // this pointer will be incremented as needed but adjacentfaces[0]
  // will always point to the beginning so it can later be freed
  
  // Step II - compute the actual vertex->tri lists...
  adjacentfacelist* adjacentfaces = new adjacentfacelist[numvertices];
  for (i = 0; i < numvertices; i++) {
    adjacentfaces[i] = adjacentfacedata;
    adjacentfacedata += numadjacentfaces[i];

    //for (int j=0; j<numadjacentfaces[i]; j++)
    //  adjacentfaces[i][j] = numfaces;
  }

  assert (adjacentfacedata == adjacentfaces[0] + numfaces);
  for (int* afdp = adjacentfaces[0]; afdp < adjacentfacedata; afdp++)
    *afdp = numfaces;
  
  for (i = 0; i < numfaces; i++) {
    int *p = adjacentfaces[faces[i]];
    while (*p != numfaces)
      p++;
    // snap to nearest multiple of 3, the start of tri data for that face
    *p = (i/3) * 3;
  }

  return adjacentfaces;
}




#define FOR_EACH_VERTEX_OF_FACE(i,j) \
  for (int jtmp = 0, j = faces[i]; \
       jtmp < 3; \
       jtmp++, j = faces[i + jtmp])

#define FOR_EACH_ADJACENT_FACE(i,j) \
  for (int jtmp=0, j = adjacentfaces[i][0]; \
       jtmp < numadjacentfaces[i]; \
       jtmp++, j = adjacentfaces[i][jtmp])

static bool* done = NULL;
static int* stripsp = NULL;
static int* numadjacentfaces = NULL;
static adjacentfacelist* adjacentfaces = NULL;
static const int* faces = NULL;
static int nstrips = 0;
static int nEvilTriangles;



// Figure out the next triangle we're headed for...
static inline int
Tstrip_Next_Tri(int tri, int v1, int v2)
{
  FOR_EACH_ADJACENT_FACE(v1, f1) {
    if ((f1 == tri) || done[f1/3])
      continue;
    FOR_EACH_ADJACENT_FACE(v2, f2) {
      if ((f2 == tri) || done[f2/3])
	continue;
      if (f1 == f2)
	return f1;
    }
  }
  
  return -1;
}

// Build a whole strip of triangles, as long as possible...
static void Tstrip_Crawl(int v1, int v2, int v3,
			 int next)
{
  // Insert the first tri...
  *stripsp++ = v1;
  *stripsp++ = v2;
  *stripsp++ = v3;
  
  int vlast1 = v3;
  int vlast2 = v2;

  bool shouldbeflipped = true;

  // Main loop...
  do {
    // Find the next vertex
    int vnext;
    FOR_EACH_VERTEX_OF_FACE(next,vnext_tmp) {
      if ((vnext_tmp == vlast1) || (vnext_tmp == vlast2))
	continue;
      vnext = vnext_tmp;
      break;
    }

    bool thisflipped = true;
    if ((faces[next+0] == vlast2) &&
	(faces[next+1] == vlast1) &&
	(faces[next+2] == vnext))
      thisflipped = false;
    if ((faces[next+2] == vlast2) &&
	(faces[next+0] == vlast1) &&
	(faces[next+1] == vnext))
      thisflipped = false;
    if ((faces[next+1] == vlast2) &&
	(faces[next+2] == vlast1) &&
	(faces[next+0] == vnext))
      thisflipped = false;
    
    if (thisflipped != shouldbeflipped) {
      if (nEvilTriangles-- > 0 && !bQuiet) {
	std::cerr << "Ugh!  As Alice would say, this triangle \""
	     << "goes the wrong way\"!" << std::endl
	     << " (Ran into inconsistent triangle orientation "
	     << "during tstrip generation)" << std::endl
	     << " (This triangle #" << next << ")" << std::endl << std::endl;
      }
      goto bail;
    }
    
    
    // Record it

    *stripsp++ = vnext;
    vlast2 = vlast1;
    vlast1 = vnext;
    done[next/3] = true;
    shouldbeflipped = !shouldbeflipped;
    
    // Try to find the next tri
  } while ((next = Tstrip_Next_Tri(next, vlast1, vlast2)) != -1);
  
 bail:
  // OK, done.  Mark end of strip
  *stripsp++ = -1;
  ++nstrips;
}

// Begin a tstrip, starting with triangle tri
// tri is ordinal, not index (counts by 1)
static void Tstrip_Bootstrap(int tri)
{
  done[tri] = true;
  
  // Find two vertices with which to start.
  // We do only a bit of lookahead, starting with vertices that will
  // let us form a strip of length at least 2...
  
  tri *= 3;
  int vert1 = faces[tri];
  int vert2 = faces[tri+1];
  int vert3 = faces[tri+2];
  
  // Try vertices 1 and 2...
  int nextface = Tstrip_Next_Tri(tri, vert1, vert2);
  if (nextface != -1) {
    Tstrip_Crawl(vert3, vert1, vert2, nextface);
    return;
  }
  
  // Try vertices 2 and 3...
  nextface = Tstrip_Next_Tri(tri, vert2, vert3);
  if (nextface != -1) {
    Tstrip_Crawl(vert1, vert2, vert3, nextface);
    return;
  }
  
  // Try vertices 3 and 1...
  nextface = Tstrip_Next_Tri(tri, vert3, vert1);
  if (nextface != -1) {
    Tstrip_Crawl(vert2, vert3, vert1, nextface);
    return;
  }
  
  // OK, nothing we can do. Do a single-triangle-long tstrip.
  *stripsp++ = vert1;
  *stripsp++ = vert2;
  *stripsp++ = vert3;
  *stripsp++ = -1;
  ++nstrips;
}


static int *Build_Tstrips(int numvertices,
			       const std::vector<int>& tris,
			       int*& endstrips,
			       int& outNstrips)
{
  adjacentfaces = TriMesh_FindAdjacentFaces
    (numvertices, tris, numadjacentfaces);

  if (!bQuiet) std::cerr << " stripping... " << std::flush;
  int numfaces = tris.size() / 3;
  
  // Allocate more than enough memory
  int* strips = new int [4*numfaces+1];
  stripsp = strips;
  nEvilTriangles = 10;
  
  // Allocate array to record what triangles we've already done
  done = new bool[numfaces];
  memset(done, 0, numfaces*sizeof(bool));
  faces = &tris[0];
  nstrips = 0;
  
  // Build the tstrips
  for (int i = 0; i < numfaces; i++) {
    if (!done[i])
      Tstrip_Bootstrap (i);
  }
  endstrips = stripsp;
  outNstrips = nstrips;

  if (nEvilTriangles < 0 && !bQuiet) {
    std::cerr << "And there were " << -nEvilTriangles
	 << " more evil triangles for which no warnings were printed."
	 << std::endl << std::endl;
  }
  
  // cleanup global arrays
  delete [] done; done = NULL;
  delete [] numadjacentfaces; numadjacentfaces = NULL;
  delete [] adjacentfaces[0]; // ptr to one chunk of data for all
  delete [] adjacentfaces; adjacentfaces = NULL;
  stripsp = NULL;
  faces = NULL;

  if (!bQuiet) std::cerr << " done." << std::endl;
  return strips;
}


ErrorCode
tris_to_strips(int numvertices,
	       const std::vector<int>& tris,
	       std::vector<int>& tstripinds)
{
  if (!bQuiet)
    std::cerr << "Tstripping " << tris.size()/3 << " triangles ("
	 << tris.size() << " vertices)..." << std::endl;

  int *strips, *end;
  int nStrips;
  strips = Build_Tstrips (numvertices, tris, end, nStrips);

  tstripinds.clear();

  // this is a hair faster...
  //tstripinds.reserve (end-strips);
  //memcpy (&tstripinds[0], strips, 4*(end-strips));

  // but this is legal :)
  tstripinds.insert (tstripinds.end(), strips, end);
  
  if (!bQuiet) 
    std::cerr << "Tstrip results: " << nstrips << " strips ("
	 << tstripinds.size() - nStrips << " vertices, avg. length "
	 << ((float)tstripinds.size()/nStrips) - 3 << ")." << std::endl;

  return ERR_NONE;
}





ErrorCode
strips_to_tris(const std::vector<int> &tstrips, 
	       std::vector<int>& tris, int nTris)
{
  assert(tstrips.back() == -1);
  tris.clear();
  if (!nTris)
    nTris = tstrips.size() * 3;  // estimate
  tris.reserve (nTris);

  if (!bQuiet) std::cerr << "Reyhdrating tris... " << std::flush;

  std::vector<int>::const_iterator vert;
  for (vert = tstrips.begin(); vert != tstrips.end(); vert++) {
    while (*vert == -1) { // handle 0-length strips
      ++vert;
      if (vert == tstrips.end()) break;
    }
    if (vert == tstrips.end()) break;

    vert += 2;   //we'll look backwards at these

    int dir = 0;
    while (*vert != -1)  {
      tris.push_back(vert[-2 + dir]);
      tris.push_back(vert[-1 - dir]);
      tris.push_back(vert[0]);
      vert++;
      dir ^= 1;
    }
  }

  if (!bQuiet) std::cerr << tris.size() << " tris.  done." << std::endl;

  return ERR_NONE;
}
