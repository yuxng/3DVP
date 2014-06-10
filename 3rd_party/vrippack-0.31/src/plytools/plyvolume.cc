/*

Compute the volume of a mesh

Sean Anderson, February 1999

*/

#include <assert.h>
#include <stdio.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>
#include <limits.h>

#include <Linear.h>

#include <vector>
using namespace std;
//#include "plyio.h"


typedef Vec3f PlyVertex;


struct PlyFace 
{
   uchar nverts;
   int * verts;
};

struct PlyTriStrip 
{
   int nverts;
   int * verts;
};

#define MAX_VERT_PROPS 3

static PlyProperty vert_prop_x =  
{"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_y =  
{"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_z =  
{"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};


static PlyProperty vert_props[MAX_VERT_PROPS];


static PlyProperty face_props[] = 
{ 
   {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};

static PlyProperty tristrip_props[] = 
{
   {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_INT, PLY_INT},
};

#define voffset(field) ((long) &(((PlyVertex *) 0)->field))
#define foffset(field) ((long) &(((PlyFace *) 0)->field))



class Plane
{
public:
   Plane()
   {
      n = Vec3f(0, 0, 1);
      d = 0;
   }
   Vec3f n;
   float d;
};

inline void
computeFaceContribution(const vector<PlyVertex> & verts, 
			const PlyFace & plyFace, 
			const Plane & plane, 
			double * subvolume, 
			double * subarea)
{
   const int * v = plyFace.verts;
   const int k = plyFace.nverts;
   Vec3f area;      
   float vol;
   Vec3f avePt;
   
   if (k == 3) // Make the common case fast:  triangle
   {
      area = 0.5 * 
	 (verts[v[1]] - verts[v[0]]).cross(verts[v[2]] - verts[v[0]]);
      avePt = (1. / 3) * (verts[v[0]] + verts[v[1]] + verts[v[2]]);      
      vol = (plane.n.dot(avePt) - plane.d) * plane.n.dot(area);
   }
   else
   {
      area.clear();
      vol = 0;
      const Vec3f v0 = verts[v[0]];
      for (int i = 0; i < k - 1; i++)
      {
	 Vec3f a = (verts[v[i]] - v0).cross(verts[v[i + 1]] - v0);
	 area += a;
	 avePt = (1. / 3) * (v0 + verts[v[i]] + verts[v[i + 1]]);      
	 vol += (plane.n.dot(avePt) - plane.d) * plane.n.dot(a);
      }
      vol *= 0.5;
      area *= 0.5;
   }

   if (subvolume)
   {
      *subvolume += vol;
   }
   if (subarea)
   {
      *subarea += area.length();
   } 
}
	    

//
// Read in data from the ply file and compute the volume and area of faces
// with plane.  The volume and area contributions are ADDED to what was 
// passed in.
//

bool
computeSignedVolume(FILE * fp, const Plane & plane, 
		    double * volume, double * area)
{
   int     j;
   int     nelems;
   char ** elist;
   int     file_type;
   float   version;
   char *  elem_name;
   int     num_vert_props;
   int     num_elems;
   
   vector<PlyVertex> verts;
   
   face_props[0].offset = foffset(verts);
   face_props[0].count_offset = foffset(nverts);  /* count offset */
    
   PlyFile * ply = 
      ply_read(fp, &nelems, &elist);

   
   if (!ply)
   {
      return true;
   }

   int nvp = 0;
   
   if (ply_is_valid_property(ply, "vertex", vert_prop_x.name)) 
   {
      vert_props[nvp] = vert_prop_x;
      vert_props[nvp].offset = voffset(x); 
      nvp++;
   }
   
   if (ply_is_valid_property(ply, "vertex", vert_prop_y.name)) 
   {
      vert_props[nvp] = vert_prop_y;
      vert_props[nvp].offset = voffset(y); 
      nvp++;
   }
   
   if (ply_is_valid_property(ply, "vertex", vert_prop_z.name)) 
   {
      vert_props[nvp] = vert_prop_z;
      vert_props[nvp].offset = voffset(z); 
      nvp++;
   }
   

   num_vert_props = nvp;

   PlyVertex plyVert;
   PlyFace plyFace;
   int faceIndicesMem[256];
   plyFace.verts = faceIndicesMem;

   for (int i = 0; i < nelems; i++) 
   {
      /* get the description of the first element */
      elem_name = elist[i];
      int nprops;
      ply_get_element_description(ply, elem_name, &num_elems, &nprops);
      
      /* if we're on vertex elements, read them in */
      if (equal_strings("vertex", elem_name)) 
      {
	 /* set up for getting vertex elements */
	 ply_get_element_setup(ply, elem_name, num_vert_props, vert_props);
	 
	 cerr << "Reading vertices . . .\n";

	 int percent = -1;
	 
	 /* grab all the vertex elements */
	 for (j = 0; j < num_elems; j++) 
	 {
	    int newpercent = 100 * j / num_elems;
	    if (newpercent != percent)
	    {
	       cerr << "\r" << (percent = newpercent) << "% complete" ;
	       cerr.flush();
	    }
	    
	    ply_get_element_noalloc(ply, (void *) &plyVert);
	    
	    verts.push_back(plyVert);
	 }
	 
	 cerr << "\rDone.        \n";
      } 
      else if (equal_strings("face", elem_name)) 
      {      
	 ply_get_element_setup(ply, elem_name, 1, face_props);
	 
	 if (num_elems == 0)
	 {
	    continue;
	 }

	 cerr << "Reading faces . . .\n";
	 
	 if (verts.size() == 0)
	 {
	    cerr << "No vertices read yet; can't read faces\n";
	    continue;
	 }
	 
	 int percent = -1;
	 
	 for (j = 0; j < num_elems; j++) 
	 {		
	    int newpercent = 100 * j / num_elems;
	    if (newpercent != percent)
	    {
	       cerr << "\r" << (percent = newpercent) << "% complete";
	       cerr.flush();
	    }
	    
	    ply_get_element_noalloc(ply, (void *) &plyFace);
	    assert(plyFace.verts == faceIndicesMem);
	    
	    computeFaceContribution(verts, plyFace, plane, volume, area);
	     
	    //faces->push_back(plyFace);
	 }
	 
	 cerr << "\rDone.        \n";
      }
      else if (equal_strings("tristrips", elem_name)) 
      {      
	 ply_get_element_setup(ply, elem_name, 1, tristrip_props);
	 
	 if (num_elems == 0)
	 {
	    continue;
	 }
	 	 
	 cerr << "Reading tristrips . . .\n";
	 
	 if (verts.size() == 0)
	 {
	    cerr << "No vertices read yet; can't read tristrip\n";
	    continue;
	 }
	 
	 PlyTriStrip plyTriStrip;
	 
	 for (j = 0; j < num_elems; j++) 
	 {
	    ply_get_element(ply, (void *) &plyTriStrip);
	    
	    int percent = -1;
	 
	    bool clockwise = false;
	    for (int k = 2; k < plyTriStrip.nverts; )
	    {
	       int newpercent = 100 * j / num_elems;
	       if (newpercent != percent)
	       {
		  cerr << "\r" << (percent = newpercent) << "% complete";
		  cerr.flush();
	       }
	    
	       plyFace.nverts = 3;
	       //plyFace.verts = new int[3];
	       plyFace.verts[0]             = plyTriStrip.verts[k - 2];
	       plyFace.verts[1 + clockwise] = plyTriStrip.verts[k - 1];
	       plyFace.verts[2 - clockwise] = plyTriStrip.verts[k - 0];

	       computeFaceContribution(verts, plyFace, plane, 
				       volume, area);
	       
	       //faces->push_back(plyFace);	       
	       
	       clockwise = !clockwise;

	       k++;

	       if (k < plyTriStrip.nverts && plyTriStrip.verts[k] == -1)
	       {
		  k += 3;
		  clockwise = false;
	       }
	    }
	    
	    free(plyTriStrip.verts);
	 }
	 
	 cerr << "\rDone.        \n";
      }
   }

   ply_close(ply);

   return false;
}

void
printUsage(const char * name)
{
   cerr << "Usage:\n"
	<< "  " << name 
	<< " -h | [-p nx ny nz d] (- | file1.ply [file2.ply [...]])\n"
	<< "where:\n"
	<< "  -h         Displays this help\n"
        << "  -p         Specifies a plane to use as the base; \n"
           "             the plane has normal (nx, ny, nz) and is distance \n"
           "             d from the origin in the normal's direction\n"
	<< "  -          Reads one ply file from standard input\n"
	<< "  file.ply   Is a ply file whose volume is added to the total.\n";
}

int 
main(int argc, char * argv[])
{
   double volume = 0, area = 0;
   Plane plane;
   
   for (int c = 1; c < argc; c++)
   {
      if (!strcmp("-h", argv[c]))
      {
	 printUsage(argv[0]);
	 return 1;
      }
      else if (!strcmp("-p", argv[c]))
      {
	 if (c + 4 >= argc)
	 {
	    cerr << "Insufficient parameters for plane description\n";
	    return 1;
	 }
	 else
	 {
	    plane.n.x = atof(argv[c + 1]);
	    plane.n.y = atof(argv[c + 2]);
	    plane.n.z = atof(argv[c + 3]);
	    plane.d  = atof(argv[c + 4]);
	    c += 4;
	 }
      }
      else if (!strcmp("-", argv[c]))
      {
	 cerr << "stdin:\n";
	 if (computeSignedVolume(stdin, plane, &volume, &area))
	 {
	    cerr << "Couldn't read ply file from stdin\n";
	 }
      }
      else
      {
	 cerr << argv[c] << ":\n";
	 FILE * fp = fopen(argv[c], "r");
	 if (!fp)
	 {
	    cerr << "Couldn't open file '" << argv[c] << "'\n";
	 }
	 else
	 {
	    if (computeSignedVolume(fp, plane, &volume, &area))
	    {
	       cerr << "Couldn't read ply file '" << argv[c] << "'\n";
	    }
	    fclose(fp);
	 }
      }
   }
   
   //cout.flags(ios::fixed);
   cout.precision(10);
   cout << "Volume:  " << volume << endl
	<< "Area:  " << area << endl;
   
   return 0;
}
