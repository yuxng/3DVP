
//
//  plycullmaxx.cc -- David Koller (dk@cs.stanford.edu), 8/16/98
//
//  This program takes 6 parameters (xmin,ymin,zmin,xmax,ymax,zmax)
//  defining an axis-aligned bounding box.  A .ply file is read
//  from stdin, and is culled such that any face whose maximum-X-
//  coordinate vertex lies outside the bounding box is removed, along
//  with any unused vertices.  The resulting culled model is written
//  to stdout as a .ply file.
//
//  This program was written with the intention of being used with
//  the "vripsplit" program.
//

#include <stdio.h>
#include <stdlib.h>
#include <ply.h>


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))


typedef struct {
   float x,y,z;
   void *other_props;
   int num;             // used first as reference count, then new id #
} Vertex;

typedef struct {
   unsigned char nverts;
   int *verts;
   void *other_props;
   int in;              // boolean true if face is in bbox
} Face;

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
int nelems;
char **elist;
int file_type;
int numVerts;
Vertex *verts;
PlyOtherProp *vert_other;
int numFaces;
Face *faces;
PlyOtherProp *face_other;
PlyOtherElems *other_elements;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;

int numOutputVerts;
int numOutputFaces;


///////////////////////////////////////////////////////////////////////


void ReadPlyFile()
{
    PlyFile *ply;
    float version;
    int i,j;
    char *elem_name;
    int num_elems;
    int nprops;

    ply = ply_read (stdin, &nelems, &elist);
    ply_get_info (ply, &version, &file_type);

    for (i=0; i<nelems; ++i)  {

	elem_name = elist[i];
	ply_get_element_description (ply, elem_name, &num_elems, &nprops);

	if (equal_strings ("vertex", elem_name)) {

	    numVerts = num_elems;
	    verts = new Vertex[numVerts];

	    ply_get_property (ply, elem_name, &vert_props[0]);
	    ply_get_property (ply, elem_name, &vert_props[1]);
	    ply_get_property (ply, elem_name, &vert_props[2]);
	    vert_other = ply_get_other_properties (ply, elem_name,
			   offsetof(Vertex,other_props));

            for (j=0; j<num_elems; ++j)
		ply_get_element (ply, (void *) &verts[j]);
	}

        else if (equal_strings ("face", elem_name)) {

            numFaces = num_elems;
            faces = new Face[numFaces];

	    ply_get_property (ply, elem_name, &face_props[0]);
            face_other = ply_get_other_properties (ply, elem_name,
                           offsetof(Face,other_props));

            for (j=0; j<num_elems; ++j)
		ply_get_element (ply, (void *) &faces[j]);
        }

	else
	    other_elements=ply_get_other_element(ply, elem_name, num_elems);
    }

    comments = ply_get_comments (ply, &num_comments);
    obj_info = ply_get_obj_info (ply, &num_obj_info);

    ply_close (ply);
}


void WritePlyFile()
{
    PlyFile *ply;
    int i;

    ply = ply_write (stdout, nelems, elist, file_type);

    ply_element_count (ply, "vertex", numOutputVerts);
    ply_describe_property (ply, "vertex", &vert_props[0]);
    ply_describe_property (ply, "vertex", &vert_props[1]);
    ply_describe_property (ply, "vertex", &vert_props[2]);
    ply_describe_other_properties(ply,vert_other,offsetof(Vertex,other_props));

    ply_element_count (ply, "face", numOutputFaces);
    ply_describe_property (ply, "face", &face_props[0]);
    ply_describe_other_properties(ply,face_other,offsetof(Face,other_props));

    ply_describe_other_elements (ply, other_elements);

    for (i = 0; i < num_comments; i++)
	ply_put_comment (ply, comments[i]);

    for (i = 0; i < num_obj_info; i++)
        ply_put_obj_info (ply, obj_info[i]);

    ply_header_complete (ply);

    ply_put_element_setup (ply, "vertex");
    for (i = 0; i < numVerts; i++)
        if (verts[i].num >= 0)
            ply_put_element (ply, (void *) &verts[i]);

    ply_put_element_setup (ply, "face");
    for (i = 0; i < numFaces; i++)
        if (faces[i].in)
            ply_put_element (ply, (void *) &faces[i]);

    ply_put_other_elements (ply);

    ply_close (ply);
}


int Keep_Face(float xmin, float ymin, float zmin,
              float xmax, float ymax, float zmax, Face *face)
{
    float maxx;         // The maximum X-coord of the face
    Vertex *maxvert;    // The vertex with maximum X-coord
    int i;

    if (face->nverts <= 0)
        return 0;

    maxvert = &verts[face->verts[0]];
    maxx = maxvert->x;

    for (i=1; i<face->nverts; ++i)
	if (verts[face->verts[i]].x > maxx)  {
            maxvert = &verts[face->verts[i]];
	    maxx = maxvert->x;
        }

    if (maxvert->x > xmax) return 0;
    if (maxvert->x < xmin) return 0;
    if (maxvert->y > ymax) return 0;
    if (maxvert->y < ymin) return 0;
    if (maxvert->z > zmax) return 0;
    if (maxvert->z < zmin) return 0;

    return 1;
}


int
main(int argc, char *argv[])
{
    float x1, y1, z1, x2, y2, z2;
    float xmin, ymin, zmin;
    float xmax, ymax, zmax;
    int i, j;
    float epsilon = 0;

    if (argc != 7 && argc != 8) {
        fprintf(stderr, "USAGE: %s <xmin> <ymin> <zmin> <xmax> <ymax> <zmax> [epsilon] < in.ply > out.ply\n", argv[0]);
        fprintf(stderr, "Where it will clip all triangles beyond epsilon\n");
        fprintf(stderr, "of the bounding box.\n");
        exit(-1);
    }

    x1 = atof(argv[1]);
    y1 = atof(argv[2]);
    z1 = atof(argv[3]);
    x2 = atof(argv[4]);
    y2 = atof(argv[5]);
    z2 = atof(argv[6]);

    if (argc == 8) {
      epsilon = atof(argv[7]);
    }

    xmin = min(x1,x2) - epsilon;
    xmax = max(x1,x2) + epsilon;
    ymin = min(y1,y2) - epsilon;
    ymax = max(y1,y2) + epsilon;
    zmin = min(z1,z2) - epsilon;
    zmax = max(z1,z2) + epsilon;

    ReadPlyFile();

    for (i=0; i<numVerts; ++i)
        verts[i].num = 0;

    for (i=0; i<numFaces; ++i)  {

	if (Keep_Face(xmin,ymin,zmin,xmax,ymax,zmax,&faces[i]))  {

	    faces[i].in = 1;

	    // Increment reference counts of relevant vertexes

	    for (j=0; j<faces[i].nverts; ++j)
		++(verts[faces[i].verts[j]].num);
	}

	else
	    faces[i].in = 0;
    }

    // Count up the number of verts and faces to be output,
    // and update the marking data for valid verts and faces

    for (i=0, numOutputVerts=0; i<numVerts; ++i)
        if (verts[i].num > 0)
            verts[i].num = numOutputVerts++;
        else
            verts[i].num = -1;

    for (i=0, numOutputFaces=0; i<numFaces; ++i)
        if (faces[i].in)  {

            ++numOutputFaces;

	    for (j=0; j<faces[i].nverts; ++j)
                faces[i].verts[j] = verts[faces[i].verts[j]].num;
        }

    WritePlyFile();
}

