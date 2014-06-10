
//
//  plymerge.cc -- David Koller (dk@cs.stanford.edu), 8/17/98
//
//  This program takes a list of .ply files, and outputs to stdout
//  a .ply file representing the merged geometry.  Note that only
//  vertex x,y,z and face connectivity information is preserved;
//  other attributes are dropped.
//
//  Redundant/shared vertices and faces are not combined; other
//  programs such as "plyshared" exist for doing this.
//

#include <stdio.h>
#include <stdlib.h>
#include <ply.h>


typedef struct {
   float x,y,z;
   void *other_props;
} Vertex;

typedef struct {
   unsigned char nverts;
   int *verts;
   void *other_props;
} Face;

typedef Vertex *VertArray;
typedef Face *FaceArray;

PlyProperty vert_props[] = {
   {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
   {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
   {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = {
   {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
    1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


// Variables for the PLY object
int *numVerts;
Vertex **verts;
int *numFaces;
Face **faces;
int numFiles;
int numTotalVerts;
int numTotalFaces;
PlyOtherProp *vert_other;
PlyOtherProp *face_other;
PlyOtherElems *other_elements;


///////////////////////////////////////////////////////////////////////


void ReadPlyFile(FILE *plyfile, int index)
{
    PlyFile *ply;
    float version;
    int i,j;
    char *elem_name;
    int num_elems;
    int nprops;
    int nelems;
    int file_type;
    char **elist;

    ply = ply_read (plyfile, &nelems, &elist);
    ply_get_info (ply, &version, &file_type);

    for (i=0; i<nelems; ++i)  {

	elem_name = elist[i];
	ply_get_element_description (ply, elem_name, &num_elems, &nprops);

	if (equal_strings ("vertex", elem_name)) {

	    numVerts[index] = num_elems;
	    verts[index] = new Vertex[num_elems];

	    ply_get_property (ply, elem_name, &vert_props[0]);
	    ply_get_property (ply, elem_name, &vert_props[1]);
	    ply_get_property (ply, elem_name, &vert_props[2]);
	    vert_other = ply_get_other_properties (ply, elem_name,
			   offsetof(Vertex,other_props));

            for (j=0; j<num_elems; ++j)
		ply_get_element (ply, (void *) &verts[index][j]);
	}

        else if (equal_strings ("face", elem_name)) {

            numFaces[index] = num_elems;
            faces[index] = new Face[num_elems];
	    
	    ply_get_property (ply, elem_name, &face_props[0]);
	    face_other = ply_get_other_properties (ply, elem_name,
						   offsetof(Face,other_props));

            for (j=0; j<num_elems; ++j)
		ply_get_element (ply, (void *) &faces[index][j]);
        }

	else
	    other_elements=ply_get_other_element(ply, elem_name, num_elems);
    }

    ply_close (ply);
}


void WritePlyFile()
{
    PlyFile *ply;
    int i, n;
    char *elist[] = {"vertex", "face"};

    ply = ply_write (stdout, 2, elist, PLY_BINARY_BE);

    ply_element_count (ply, "vertex", numTotalVerts);
    ply_describe_property (ply, "vertex", &vert_props[0]);
    ply_describe_property (ply, "vertex", &vert_props[1]);
    ply_describe_property (ply, "vertex", &vert_props[2]);
    ply_describe_other_properties(ply,vert_other,offsetof(Vertex,other_props));

    ply_element_count (ply, "face", numTotalFaces);
    ply_describe_property (ply, "face", &face_props[0]);
    ply_describe_other_properties(ply,face_other,offsetof(Face,other_props));

    ply_describe_other_elements (ply, other_elements);

    ply_header_complete (ply);

    ply_put_element_setup (ply, "vertex");
    for (n=0; n<numFiles; ++n)
        for (i = 0; i < numVerts[n]; i++)
            ply_put_element (ply, (void *) &verts[n][i]);

    ply_put_element_setup (ply, "face");
    for (n=0; n<numFiles; ++n)
        for (i = 0; i < numFaces[n]; i++)
            ply_put_element (ply, (void *) &faces[n][i]);

    ply_put_other_elements (ply);

    ply_close (ply);
}


int
main(int argc, char *argv[])
{
    int i, j, k;
    FILE *f;

    numFiles = argc - 1;

    numVerts = new int[numFiles];
    verts = new VertArray[numFiles];
    numFaces = new int[numFiles];
    faces = new FaceArray[numFiles];

    numTotalVerts = 0;
    numTotalFaces = 0;

    for (i=0; i<numFiles; ++i)  {

	f = fopen(argv[i+1], "r");
	if (!f)  {
	    fprintf(stderr, "Unable to open file (argv[%d])\n", i+1);
	    exit(-1);
	}

	ReadPlyFile(f, i);

	// Offset the vertex indices of the new faces just read in

        for (j=0; j<numFaces[i]; ++j)
            for (k=0; k<faces[i][j].nverts; ++k)
                faces[i][j].verts[k] += numTotalVerts;

	numTotalVerts += numVerts[i];
	numTotalFaces += numFaces[i];

	fclose(f);
    }

    WritePlyFile();
}

