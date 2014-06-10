/*

plysubdiv:
Subdivide triangles in the mesh to create a new file with even more
triangles.  As a general operator, it will create a new vertex in
the middle of a triangle, and re-tesselate it with 3 triangles instead
of 1.

Lucas Pereira, Jan, 2000.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <iostream>
#include <assert.h>

/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  int index;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


/*** the PLY object ***/

#define MAXTRISPERVERT 20

class Midpoints {

public:
  Midpoints() {
    npoints = 0;
  }

  int GetMidpoint(int ptB) {
    for (int i=0; i < npoints; i++) {
      if (otherpoint[i] == ptB) {
	return(midpoint[i]);
      }

    }
    // if we get to here, didn't find it
    return(-1);
  }

  int SetMidpoint(int ptB, int mid) {
    if (npoints+1 >= MAXTRISPERVERT) {
      fprintf(stderr, "Error adding edge to vertex %d -> too many tris!\n", 
	      ptB);
      exit(-1);
    }
    otherpoint[npoints] = ptB;
    midpoint[npoints] = mid;
    npoints++;
    return mid;
  }

  int npoints;
  int otherpoint[MAXTRISPERVERT];
  int midpoint[MAXTRISPERVERT];

};

// The midpointtable class looks up the numbers of the midpoints,
// and assigns them as needed...
class MidpointTable {
public:
  MidpointTable(int numverts) {
    nverts = numverts;
    mps = new Midpoints[nverts];
  }
  int GetMidpoint(int ptA, int ptB) {
    // cerr << "GMP: " << ptA << " " << ptB << endl;
    // make sure ptA is the lower index.
    if (ptA > ptB) {
      int t = ptA;
      ptA = ptB;
      ptB = t;
    }
    return(mps[ptA].GetMidpoint(ptB));
  }

  int SetMidpoint(int ptA, int ptB, int mid) {
    // make sure ptA is the lower index.
    if (ptA > ptB) {
      int t = ptA;
      ptA = ptB;
      ptB = t;
    }
    return(mps[ptA].SetMidpoint(ptB, mid));
  }
    
  int nverts;
  Midpoints *mps;
};
  


class PlyObject {

public:
  void read_file(FILE *inFile);
  void write_file();
  void subdivide(PlyObject *inply);

  int nverts,nfaces;
  int nvertsallocated, nfacesallocated;
  Vertex **vlist;
  Face **flist;
  PlyOtherElems *other_elements;
  PlyOtherProp *vert_other, *face_other;
  int nelems;
  char **elist;
  int num_comments;
  char **comments;
  int num_obj_info;
  char **obj_info;
  int file_type;
  MidpointTable *mpt;
};

void usage(char *progname);


/******************************************************************************
Main program.
******************************************************************************/

int
main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  FILE *inFile = stdin;
  PlyObject *inply, *outply;

  inply = new PlyObject();
  outply = new PlyObject();

  progname = argv[0];

  /* Parse -flags */
  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 'h':
	  usage (progname);
	  exit(0);
	  break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  /* optional input file (if not, read stdin ) */
  if (argc > 0 && *argv[0] != '-') {
    inFile = fopen(argv[0], "r");
    if (inFile == NULL) {
      fprintf(stderr, "Error: Couldn't open input file %s\n", argv[0]);
      usage(progname);
      exit(-1);
    }
    argc --;
    argv ++;
  } 

   /* Check no extra args */
   if (argc > 0) {
     fprintf(stderr, "Error: Unhandled arg: %s\n", argv[0]);
     usage(progname);
     exit(-1);
   }

   // Read in the file
   inply->read_file(inFile);

   // Generate outply from inply
   outply->subdivide(inply);
   
  // Write back out
  outply->write_file();
}


/******************************************************************************
Print out usage information.
******************************************************************************/

void
usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] [in.ply] > out.ply\n", progname);
  fprintf (stderr, "   or: %s [flags] < in.ply > out.ply\n", progname);
  fprintf (stderr, "\n");
  fprintf (stderr, "   This program will subdivide triangles in the mesh\n");
  fprintf (stderr, "   to create a new file with even more triangles. \n");
  fprintf (stderr, "   As a general operator, it will create a new vertex\n");
  fprintf (stderr, "   at the midpoint of each triangle edge, and then\n");
  fprintf (stderr, "   re-tesselate it with 4 triangles instead of 1.\n");
  fprintf (stderr, "   E.g.:\n");
  fprintf (stderr, "                                                     \n");
  fprintf (stderr, "        o                      o                     \n");
  fprintf (stderr, "       / `                    / `                    \n");
  fprintf (stderr, "      /   `     ------>      o---o                   \n");
  fprintf (stderr, "     /     `                / ` / `                  \n");
  fprintf (stderr, "    o-------o              o---o---o                 \n");
  fprintf (stderr, "\n");
  fprintf (stderr, "   It does nothing intelligent with other properties\n");
  fprintf (stderr, "   (such as confidence or normals).  Faces copy the\n");
  fprintf (stderr, "   original face's properties, and new vertices copy\n");
  fprintf (stderr, "   the attributes of original vertices.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "Known bugs:\n");
  fprintf (stderr, "   - It allocates a little more memory than it needs,\n");
  fprintf (stderr, "     and doesn't check array limits well.\n");
  fprintf (stderr, "   - It only works for triangles, not 4+ sided faces.\n");
  fprintf (stderr, "\n");
  exit(-1);
}



/******************************************************************************
Generate a new plyobject by subdividing tris in old
******************************************************************************/

void
PlyObject::subdivide(PlyObject *inply)
{
  // First, set all the random junk to be equal
  *this = *inply;

  // compute number of vertices, triangles to allocate
  // These computations require only triangles (not squares, etc...)
  nfacesallocated = 4 * nfaces;
  nvertsallocated = nverts + 3 * nfaces;
  flist = (Face **) malloc (sizeof (Face *) * nfacesallocated);
  vlist = (Vertex **) malloc (sizeof (Vertex *) * nvertsallocated);
  mpt = new MidpointTable(nverts);

  nfaces = 0;
  nverts = 0;
  
  // First, copy all existing vertices into vlist...
  for (nverts = 0; nverts < inply->nverts; nverts++) {
    vlist[nverts] = inply->vlist[nverts];
    // cerr << "Copying vertex " << nverts << endl;
  }

  // Allocate space to store new faces  (memory leak, but this is a 
  // really short program...)
  Face *newfaces = (Face *) malloc (sizeof(Face) * nfacesallocated);
  Vertex *newverts = (Vertex *) malloc (sizeof(Vertex) * nvertsallocated);
  int newfacei = 0;
  int newverti = 0;

  // Assign numbers to all the new midpoints...
  // (iterate over every input vertex....)
  for (int facei = 0; facei < inply->nfaces; facei++) {
    Face *inface = inply->flist[facei];
    assert(inface->nverts == 3);
    int v1n = inface->verts[0];
    int v2n = inface->verts[1];
    int v3n = inface->verts[2];
    Vertex *v1 = inply->vlist[v1n];
    Vertex *v2 = inply->vlist[v2n];
    Vertex *v3 = inply->vlist[v3n];

    //
    //          3
    //         * *
    //        *   *
    //       13***23
    //      * *   * *
    //     *   * *   *
    //    1*****12****2
    //
    // First edge...
    int v12n = mpt->GetMidpoint(v1n, v2n);
    if (v12n == -1) {  
      v12n = mpt->SetMidpoint(v1n, v2n, nverts);
      // cerr << "MidVert#: " << v12n << endl;
      Vertex *newv = &(newverts[newverti++] = *v1);
      newv->x = (v1->x + v2->x) / 2;
      newv->y = (v1->y + v2->y) / 2;
      newv->z = (v1->z + v2->z) / 2;
      newv->index = v12n;
      vlist[nverts++] = newv;
    }
    // Second edge...
    int v23n = mpt->GetMidpoint(v2n, v3n);
    if (v23n == -1) {  
      v23n = mpt->SetMidpoint(v2n, v3n, nverts);
      // cerr << "MidVert#: " << v23n << endl;
      Vertex *newv = &(newverts[newverti++] = *v2);
      newv->x = (v2->x + v3->x) / 2;
      newv->y = (v2->y + v3->y) / 2;
      newv->z = (v2->z + v3->z) / 2;
      newv->index = v23n;
      vlist[nverts++] = newv;
    }
    // Third edge...
    int v31n = mpt->GetMidpoint(v3n, v1n);
    if (v31n == -1) {  
      v31n = mpt->SetMidpoint(v3n, v1n, nverts);
      // cerr << "MidVert#: " << v31n << endl;
      Vertex *newv = &(newverts[newverti++] = *v3);
      newv->x = (v3->x + v1->x) / 2;
      newv->y = (v3->y + v1->y) / 2;
      newv->z = (v3->z + v1->z) / 2;
      newv->index = v31n;
      vlist[nverts++] = newv;
    }
  
    // Generate 4 faces...
    // just copies "other properties" from inface...
    Face *newf;
    // 1, 12, 31
    newf = &(newfaces[newfacei++] = *inface);
    newf->verts = (int *) new int[3];
    newf-> verts[0] = v1n;
    newf-> verts[1] = v12n;
    newf-> verts[2] = v31n;
    flist[nfaces++] = newf;
    // 12, 23, 31
    newf = &(newfaces[newfacei++] = *inface);
    newf->verts = (int *) new int[3];
    newf-> verts[0] = v12n;
    newf-> verts[1] = v23n;
    newf-> verts[2] = v31n;
    flist[nfaces++] = newf;
    // 2, 23, 12
    newf = &(newfaces[newfacei++] = *inface);
    newf->verts = (int *) new int[3];
    newf-> verts[0] = v2n;
    newf-> verts[1] = v23n;
    newf-> verts[2] = v12n;
    flist[nfaces++] = newf;
    // 3, 31, 23
    newf = &(newfaces[newfacei++] = *inface);
    newf->verts = (int *) new int[3];
    newf-> verts[0] = v3n;
    newf-> verts[1] = v31n;
    newf-> verts[2] = v23n;
    flist[nfaces++] = newf;
  }
 
}

/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/

void
PlyObject::read_file(FILE *inFile)
{
  int i,j,k;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;


  /*** Read in the original PLY object ***/


  ply  = ply_read (inFile, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
  

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;
      
      /* set up for getting vertex elements */
      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      vert_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Vertex,other_props));

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      /* set up for getting face elements */

      ply_get_property (ply, elem_name, &face_props[0]);
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);

      }
    }
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);
}


/******************************************************************************
Write out the PLY file to standard out.
Ignore all the points (and corresponding faces) that have confidence
below the specified threshold....
******************************************************************************/

void
PlyObject::write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, nelems, elist, file_type);

  // count the vertices that have valid confidence
  vert_count = 0;
  for (i = 0; i < nverts; i++) {
    // Set the index to either the index number, or -1...
    if (1) {
      vlist[i]->index = vert_count;
      vert_count++;
    } else {
      vlist[i]->index = -1;
    }
  }

  // count the faces that are still valid
  face_count = 0;
  for (i = 0; i < nfaces; i++) {
    bool valid = (flist[i]->nverts > 0);
    for (j = 0; j < flist[i]->nverts; j++) {
      if (vlist[flist[i]->verts[j]]->index == -1) {
	valid = false;
	break;
      }
    }
    
    // If face not valid, set nverts to 0, so it won't
    // get written out later.
    if (valid) {
      face_count++;
    } else {
      flist[i]->nverts = 0;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", face_count);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */

  ply_put_element_setup (ply, "vertex");

  for (i = 0; i < nverts; i++)
    if (vlist[i]->index > -1)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");

  for (i = 0; i < nfaces; i++) {
    if (flist[i]->nverts == 0)
      continue;
    for (j = 0; j < flist[i]->nverts; j++)
      flist[i]->verts[j] = (vlist[flist[i]->verts[j]])->index;
    ply_put_element (ply, (void *) flist[i]);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}
