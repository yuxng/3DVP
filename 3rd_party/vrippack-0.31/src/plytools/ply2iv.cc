//
// Brian L. Curless
// Stanford University
// May 1994
//

#include <stdio.h>
#include <Inventor/So.h>
#include <ply.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream.h>

#define MAX_VERT_PROPS 20

typedef unsigned char uchar;
typedef float Normal[3];

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    uchar diff_r, diff_g, diff_b;
    float intensity;
    float std_dev;
};

struct Face {
    uchar nverts;
    int *verts;
};



static PlyProperty vert_prop_x =  
   {"x", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_y =  
  {"y", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_z =  
  {"z", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_nx =  
   {"nx", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_ny =  
  {"ny", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_nz =  
  {"nz", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_intens =  
  {"intensity", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_std_dev =  
  {"std_dev", PLY_FLOAT, PLY_FLOAT, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_r =  
  {"diffuse_red", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_g =  
  {"diffuse_green", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};
static PlyProperty vert_prop_diff_b =  
  {"diffuse_blue", PLY_UCHAR, PLY_UCHAR, 0, 0, PLY_START_TYPE, PLY_START_TYPE, 0};

static PlyProperty vert_props[MAX_VERT_PROPS];


static PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, 0, 1, PLY_UCHAR, PLY_UCHAR, 0},
};


/* dummy variables and associated macros for computing field offsets */

Vertex *vert_dummy;
#define voffset(field) ((char *) (&vert_dummy->field) - (char *) vert_dummy)
Face *face_dummy;
#define foffset(field) ((char *) (&face_dummy->field) - (char *) face_dummy)


static int DIFFUSE_COLORS;
static int INTENSITIES;
static int FOUND_NORMALS;

// Function defintions
PlyFile *readPlyFile(FILE *inFile, Vertex **pVerts, Face **pFaces, 
		     int *pNumVerts, int *pNumFaces);
SoIndexedFaceSet *buildFaceSet(Face *faces, int numFaces, int flipNormals);
SoIndexedTriangleStripSet *buildTMesh(Face *faces, int numFaces);
int isTriangleMesh(Face *faces, int numFaces);
void printUsage();
Normal *createNormals(Vertex *verts, Face *faces, int numVerts, int numFaces);
void getAreaAndNormal(Face *tri, Vertex *verts, float *area, Normal normal);


void
main(int argc, char**argv)
{    
    Vertex *verts;
    Face *faces;
    int numVerts, numFaces, i;
    SoMaterialBinding *mtlBndg;

    int useIntensities = FALSE;
    int flipNormals = FALSE;
    int computeNormals = FALSE;

    char *plyName = NULL;
    FILE *inFile;

    // Process input parameters

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-n")) {
	    computeNormals = TRUE;
	} else if (!strcmp(argv[i], "-intens")) {
	    useIntensities = TRUE;
	} else if (!strcmp(argv[i], "-flipn")) {
	    flipNormals = TRUE;
	} else if (!strcmp(argv[i], "-h")) {
	    printUsage();
	    exit(0);
	} else if (argv[i][0] != '-' && plyName == NULL) {
	  // First non-'-flag' arg, assume input ply file
	  plyName = argv[i];
	} else {
	  // Unhandled arg -- print usage
	  printUsage();
	  exit(-1);
	}
    }

    // open input file
    if (plyName == NULL) {
      inFile = stdin;
    } else {
      inFile = fopen(plyName, "r");
      if (inFile == NULL) {
	fprintf(stderr, "Error: Couldn't open input ply file %s.\n", plyName);
	printUsage();
	exit(-1);
      }
    }
    readPlyFile(inFile, &verts, &faces, &numVerts, &numFaces);

    if (verts == NULL) {
	fprintf(stderr, "Obtained no vertices.\n");
	exit(1);
    }

    if (faces == NULL)	
	computeNormals = FALSE;

    // Initialize Inventor
    SoDB::init();


    // Build scene graph

    SoSeparator *root = new SoSeparator;
    root->ref();

    /*  Used to have to insert scale node 10x and then scale the 
        vertices by 0.1x .  I believe this bug has been fixed.

    float scale = 10;
    SoScale *sc = new SoScale;
    root->addChild(sc);
    sc->scaleFactor.setValue(1/scale, 1/scale, 1/scale);
    */


    Normal *normals;
    SoNormalBinding *normalBinding;
    if (FOUND_NORMALS && !computeNormals) {
	SoNormal *normal = new SoNormal;
	root->addChild(normal);
	normal->vector.insertSpace(0, numVerts);
	for (i = 0; i < numVerts; i++) {
	    if (!flipNormals) {
		normal->vector.set1Value(i, verts[i].nx, 
					 verts[i].ny, verts[i].nz);
	    }
	    else {
		normal->vector.set1Value(i, -verts[i].nx, 
					 -verts[i].ny, -verts[i].nz);
	    }
	}
	normalBinding = new SoNormalBinding;
	root->addChild(normalBinding);	
    }
    else if (computeNormals) {
	normals = createNormals(verts, faces, numVerts, numFaces);

	if (normals != NULL) {
	    if (flipNormals) {
		for (i = 0; i < numVerts; i++) {
		    normals[i][0] = -normals[i][0];
		    normals[i][1] = -normals[i][1];
		    normals[i][2] = -normals[i][2];
		}
	    }
	    SoNormal *normal = new SoNormal;
	    root->addChild(normal);
	    normal->vector.insertSpace(0, numVerts);
	    normal->vector.setValues(0, numVerts, normals);
	    normalBinding = new SoNormalBinding;
	    root->addChild(normalBinding);
	} else {
	    computeNormals = FALSE;
	    // Assume smooth shading
	    SoShapeHints *shints = new SoShapeHints;
	    shints->creaseAngle.setValue(M_PI*0.999);
	    root->addChild(shints);
	}
    } else {
	// Assume smooth shading
	SoShapeHints *shints = new SoShapeHints;
	shints->creaseAngle.setValue(M_PI*0.999);
	root->addChild(shints);
    }

    // Insert the coordinates
    SoCoordinate3 *coord = new SoCoordinate3;
    root->addChild(coord);

    coord->point.insertSpace(0, numVerts-1);
    for (i = 0; i < numVerts; i++) {
	coord->point.set1Value(i, verts[i].x, verts[i].y, verts[i].z);
    }

    SoMaterial *mtl = new SoMaterial;
    root->addChild(mtl);
    mtl->ambientColor.setValue(0,0,0);
    mtl->diffuseColor.setValue(1.0,1.0,1.0);

    // Build materials if necessary
    if (DIFFUSE_COLORS || (useIntensities&&INTENSITIES)) {
	mtl->diffuseColor.insertSpace(0, numVerts);
	for (i = 0; i < numVerts; i++) {
	  if (useIntensities&&INTENSITIES)
	    mtl->diffuseColor.set1Value(i, verts[i].intensity, 
					verts[i].intensity, 
					verts[i].intensity);
	  else
	    mtl->diffuseColor.set1Value(i, 
					verts[i].diff_r/255.0, 
					verts[i].diff_g/255.0, 
					verts[i].diff_b/255.0);
	}
	mtlBndg = new SoMaterialBinding;
	root->addChild(mtlBndg);
    } else {
	mtl->diffuseColor.setValue(0.5,0.5,0.5);
	mtl->specularColor.setValue(0.4,0.4,0.4);
	mtl->shininess.setValue(0.1);
    }
   
    if (faces == NULL) {
	if (DIFFUSE_COLORS || (useIntensities&&INTENSITIES))
	    mtlBndg->value.setValue(SoMaterialBinding::PER_VERTEX_INDEXED);
	SoPointSet *ptSet = new SoPointSet;
	root->addChild(ptSet);    
    } else {
	if (DIFFUSE_COLORS || (useIntensities&&INTENSITIES))
	    mtlBndg->value.setValue(SoMaterialBinding::PER_VERTEX_INDEXED);

	if (computeNormals || FOUND_NORMALS)
	    normalBinding->value.setValue
		(SoNormalBinding::PER_VERTEX_INDEXED);	
	SoIndexedFaceSet *faceSet = 
	    buildFaceSet(faces, numFaces, flipNormals);
	faceSet->normalIndex.setValue(-1);
	root->addChild(faceSet);    
    }


    // Write the Inventor file

    SoWriteAction wa;
    wa.getOutput()->setFilePointer(stdout);
    wa.getOutput()->setBinary(TRUE);
    wa.apply(root);    

    exit(0);
}



//
// Read in data from the ply file
//

PlyFile *
readPlyFile(FILE *inFile, Vertex **pVerts, Face **pFaces, int *pNumVerts, 
	    int *pNumFaces)
{
    int j;
    int nelems;
    char **elist;
    char *elem_name;
    int nprops, num_vert_props;
    int num_elems;

    vert_props[0].offset = voffset(x);
    vert_props[1].offset = voffset(y);
    vert_props[2].offset = voffset(z);
    vert_props[3].offset = voffset(diff_r);
    vert_props[4].offset = voffset(diff_g);
    vert_props[5].offset = voffset(diff_b);
    
    face_props[0].offset = foffset(verts);
    face_props[0].count_offset = foffset(nverts);  /* count offset */
    
    PlyFile *ply  = ply_read (inFile, &nelems, &elist);

    if (!ply)
	exit(1);

    int nvp = 0;

    INTENSITIES = FALSE;
    DIFFUSE_COLORS = FALSE;
    FOUND_NORMALS = FALSE;

    if (ply_is_valid_property(ply, "vertex", vert_prop_x.name)) {
	vert_props[nvp] = vert_prop_x;
	vert_props[nvp].offset = voffset(x); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_y.name)) {
	vert_props[nvp] = vert_prop_y;
	vert_props[nvp].offset = voffset(y); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_z.name)) {
	vert_props[nvp] = vert_prop_z;
	vert_props[nvp].offset = voffset(z); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_nx.name)) {
	FOUND_NORMALS = TRUE;
	vert_props[nvp] = vert_prop_nx;
	vert_props[nvp].offset = voffset(nx); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_ny.name)) {
	vert_props[nvp] = vert_prop_ny;
	vert_props[nvp].offset = voffset(ny); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_nz.name)) {
	vert_props[nvp] = vert_prop_nz;
	vert_props[nvp].offset = voffset(nz); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_intens.name)) {
	INTENSITIES = TRUE;
	vert_props[nvp] = vert_prop_intens;
	vert_props[nvp].offset = voffset(intensity); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", vert_prop_std_dev.name)) {
	vert_props[nvp] = vert_prop_std_dev;
	vert_props[nvp].offset = voffset(std_dev); nvp++;
    }
    
    if (ply_is_valid_property(ply, "vertex", "diffuse_red") &&
	ply_is_valid_property(ply, "vertex", "diffuse_green") &&
	ply_is_valid_property(ply, "vertex", "diffuse_blue")) 
    {
	DIFFUSE_COLORS = TRUE;
	vert_props[nvp] = vert_prop_diff_r;
	vert_props[nvp].offset = voffset(diff_r); nvp++;
	vert_props[nvp] = vert_prop_diff_g;
	vert_props[nvp].offset = voffset(diff_g); nvp++;
	vert_props[nvp] = vert_prop_diff_b;
	vert_props[nvp].offset = voffset(diff_b); nvp++;
    }
    
    num_vert_props = nvp;

    Vertex *verts = NULL;
    Face *faces = NULL;

    for (int i = 0; i < nelems; i++) {

	/* get the description of the first element */
	elem_name = elist[i];
	ply_get_element_description 
	    (ply, elem_name, &num_elems, &nprops);
	
	/* if we're on vertex elements, read them in */
	if (equal_strings ("vertex", elem_name)) {
	    
	    int numVerts = *pNumVerts = num_elems;
	    verts = new Vertex[numVerts];
	    
	    /* set up for getting vertex elements */
	    ply_get_element_setup (ply, elem_name, num_vert_props, vert_props);
	    
	    /* grab all the vertex elements */
	    for (j = 0; j < numVerts; j++)
		ply_get_element (ply, (void *) &verts[j]);
	}

	if (equal_strings ("face", elem_name)) {

	    ply_get_element_setup (ply, elem_name, 1, face_props);

	    int numFaces = *pNumFaces = num_elems;

	    if (numFaces == 0)
		continue;

	    faces = new Face[numFaces];

	    for (j = 0; j < numFaces; j++)		
		ply_get_element (ply, (void *) &faces[j]);
	}
    }

    *pVerts = verts;
    *pFaces = faces;

    return ply;
}



//
// Build Inventor face set from faces
//

SoIndexedFaceSet *
buildFaceSet(Face *faces, int numFaces, int flipNormals)
{
    SoIndexedFaceSet *faceSet = new SoIndexedFaceSet;

    int total = 0;
    for (int i = 0; i < numFaces; i++) {
	total += int(faces[i].nverts + 1);
    }

    faceSet->coordIndex.insertSpace(0, total-1);

    int counter = 0;
    for (i = 0; i < numFaces; i++) {
	if (!flipNormals) {
	    for (int j = 0; j < faces[i].nverts; j++) {
		faceSet->coordIndex.set1Value(counter, faces[i].verts[j]);
		counter++;
	    }
	} else {
	    for (int j = faces[i].nverts-1; j >= 0 ; j--) {
		faceSet->coordIndex.set1Value(counter, faces[i].verts[j]);
		counter++;
	    }
	}
	faceSet->coordIndex.set1Value(counter, SO_END_FACE_INDEX);	
	counter++;
    }

    return faceSet;
}



//
// Build Inventor triangle mesh from faces.
//

SoIndexedTriangleStripSet *
buildTMesh(Face *faces, int numFaces)
{
    SoIndexedTriangleStripSet *tmesh = new SoIndexedTriangleStripSet;

    tmesh->coordIndex.insertSpace(0, 4*numFaces);


    // Make sure all the faces are triangles
    if (!isTriangleMesh(faces, numFaces)) {
	cerr << "Not a triangle mesh.\n";
	return NULL;
    }

    int index = 0;

    tmesh->coordIndex.set1Value(index, faces[0].verts[0] );	
    index++;
    tmesh->coordIndex.set1Value(index, faces[0].verts[1] );
    index++;
    tmesh->coordIndex.set1Value(index, faces[0].verts[2] );
    index++;

    int newStrip = TRUE;
    for (int i = 1; i < numFaces; i++) {
	if (faces[i-1].verts[1] == faces[i].verts[1] 
	    && faces[i-1].verts[2] == faces[i].verts[0]) {
	    tmesh->coordIndex.set1Value(index, faces[i].verts[2] );	
	    index++;
	    newStrip = FALSE;
	}
	else if (newStrip
		 && faces[i-1].verts[1] == faces[i].verts[0] 
		 && faces[i-1].verts[2] == faces[i].verts[1]) {

// Can't swap???  Broken!!!
//	    tmesh->coordIndex.set1Value(index, SO_SWAP_MESH_INDEX);	


	    index++;
	    tmesh->coordIndex.set1Value(index, faces[i].verts[2] );	
	    index++;
	    newStrip = FALSE;
	}
	else {
	    tmesh->coordIndex.set1Value(index, SO_END_STRIP_INDEX);	
	    index++;
	    tmesh->coordIndex.set1Value(index, faces[i].verts[0] );
	    index++;
	    tmesh->coordIndex.set1Value(index, faces[i].verts[1] );	
	    index++;
	    tmesh->coordIndex.set1Value(index, faces[i].verts[2] );	
	    index++;
	    newStrip = TRUE;
	}
    }

    tmesh->coordIndex.deleteValues(index, -1);

    return tmesh;
}



// 
// Check to see that all faces are triangles.
//

int
isTriangleMesh(Face *faces, int numFaces)
{
    for (int i = 0; i < numFaces; i++) {
	if (faces[i].nverts != 3)
	    return FALSE;
    }

    return TRUE;
}


Normal *
createNormals(Vertex *verts, Face *tris, int numVerts, int numTris)
{
    int i, j, index;

    if (!isTriangleMesh(tris, numTris)) {
	return NULL;
    }

    Normal *normals = new Normal[numVerts];

    for (i = 0; i < numVerts; i++) {
	normals[i][0] = normals[i][1] = normals[i][2] = 0;
    }

    float triArea, mag;
    Normal triNormal;

    for (i = 0; i < numTris; i++) {
	getAreaAndNormal(&tris[i], verts, &triArea, triNormal);
	for (j = 0; j < tris[i].nverts; j++) {
	    index = tris[i].verts[j];
	    // I'm doing straight averaging of normals here!
	    // What's the best answer???
#if 0
	    normals[index][0] += triArea*triNormal[0];
	    normals[index][1] += triArea*triNormal[1];
	    normals[index][2] += triArea*triNormal[2];
#else
	    normals[index][0] += triNormal[0];
	    normals[index][1] += triNormal[1];
	    normals[index][2] += triNormal[2];
#endif
	}
    }

    for (i = 0; i < numVerts; i++) {
	mag = sqrt(normals[i][0] * normals[i][0] +
		   normals[i][1] * normals[i][1] +
		   normals[i][2] * normals[i][2]);
	if (mag != 0) {
	    normals[i][0] /= mag;
	    normals[i][1] /= mag;
	    normals[i][2] /= mag;
	}
    }

    return normals;
}



void
getAreaAndNormal(Face *tri, Vertex *verts, float *area, Normal normal)
{
    Vertex *vert0 = &verts[tri->verts[0]];
    Vertex *vert1 = &verts[tri->verts[1]];
    Vertex *vert2 = &verts[tri->verts[2]];

    float v0[3], v1[3], norm[3];

    v0[0] = vert2->x - vert1->x;
    v0[1] = vert2->y - vert1->y;
    v0[2] = vert2->z - vert1->z;

    v1[0] = vert0->x - vert1->x;
    v1[1] = vert0->y - vert1->y;
    v1[2] = vert0->z - vert1->z;

    norm[0] = v0[1]*v1[2] - v0[2]*v1[1];
    norm[1] = -v0[0]*v1[2] + v0[2]*v1[0];
    norm[2] = v0[0]*v1[1] - v0[1]*v1[0];

    float length = sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);

    *area = length/2;

    if (length != 0) {
	normal[0] = norm[0]/length;
	normal[1] = norm[1]/length;
	normal[2] = norm[2]/length;
    }
    else {
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
    }
}


void
printUsage()
{
    printf("\n");
    printf("Usage: ply2iv [-p -pf -intens -n -flipn -h] [in.ply] > out.iv\n");
    printf("   or: ply2iv [-p -pf -intens -n -flipn -h] < in.ply > out.iv\n");
    printf("\n");
    printf("  Plytoiv converts a Ply file containing vertices and faces into an\n");
    printf("  Inventor file containing a point set and/or a face set.  The default\n");
    printf("  behavior is to generate a face set.  Options:\n");
    printf("\n");
    printf("  -h      Print this help message.\n");
    printf("  -p      Generate a point set (no faces).\n");
    printf("  -pf     Generate both a point set and a face set.\n");
    printf("  -ic     Map 'intensity' to color.\n");
    printf("  -n      Generate and store normals.\n");
    printf("  -flipn  Flip normals.\n");
    printf("\n");
}

