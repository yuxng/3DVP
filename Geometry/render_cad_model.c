#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "mex.h"

#define WIDTH 1242
#define HEIGHT 375
#define BUFFERSIZE 256

/* global variables */
int Nvertice, Nface;
GLdouble *Vertices;
GLdouble *Fnormals;
GLdouble *Vnormals;
GLuint *Faces;

/* load a CAD model from a .off file */
int load_off_file(int* pnv, GLdouble** pvertices, int* pnf, GLuint** pfaces, char* filename)
{
  FILE* fp;
  char buffer[BUFFERSIZE];
  int nv, nf, aux, i;
  GLdouble *vertices;
  GLuint *faces;

  /* open file */
  fp = fopen(filename, "r");
  if(fp == NULL)
  {
    printf("Can not open file %s\n", filename);
    *pnv = 0;
    *pvertices = NULL;
    *pnf = 0;
    *pfaces = NULL;
    return 1;
  }

  /* read file header */
  fgets(buffer, 256, fp);
  if(strncmp(buffer, "OFF", 3) != 0)
  {
    printf("Wrong format .off file %s\n", filename);
    return 1;
  }

  /* read numbers */
  fscanf(fp, "%d", &nv);
  fscanf(fp, "%d", &nf);
  fscanf(fp, "%d", &aux);

  /* allocate memory */
  vertices = (GLdouble*)malloc(sizeof(GLdouble)*nv*3);
  if(vertices == NULL)
  {
    printf("Out of memory!\n");
    return 1;
  }

  /* read vertices */
  for(i = 0; i < 3*nv; i++)
    fscanf(fp, "%lf", vertices+i);

  /* allocate memory */
  if(nf != 0)
  {
    faces = (GLuint*)malloc(sizeof(GLuint)*nf*3);
    if(faces == NULL)
    {
      printf("Out of memory\n");
      return 1;
    }

    /* read faces */
    for(i = 0; i < nf; i++)
    {
      fscanf(fp, "%d", &aux);
      if(aux != 3)
      {
        printf("Face contains more than 3 vertices!\n");
        return 1;
      }
      fscanf(fp, "%d", faces + 3*i);
      fscanf(fp, "%d", faces + 3*i+1);
      fscanf(fp, "%d", faces + 3*i+2);
    }
  }
  else
    faces = NULL;

  fclose(fp);
  *pnv = nv;
  *pvertices = vertices;
  *pnf = nf;
  *pfaces = faces;
  return 0;
}

static GLvoid cross_product(GLdouble* u, GLdouble* v, GLdouble* n)
{
  assert(u);
  assert(v);
  assert(n);
    
  n[0] = u[1]*v[2] - u[2]*v[1];
  n[1] = u[2]*v[0] - u[0]*v[2];
  n[2] = u[0]*v[1] - u[1]*v[0];
}

static GLvoid normalize(GLdouble* v)
{
    GLdouble l;
    
    assert(v);
    
    l = (GLdouble)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

/* compute face normals */
GLdouble * compute_face_normals(int nv, GLdouble* pvertices, int nf, GLuint* pfaces)
{
  int i;
  GLuint v1, v2, v3;
  GLdouble u[3];
  GLdouble v[3];
  GLdouble *pfnormals;

  /* allocate memory */
  if(nf != 0)
  {
    pfnormals = (GLdouble*)malloc(sizeof(GLdouble)*nf*3);
    if(pfnormals == NULL)
    {
      printf("Out of memory\n");
      return NULL;
    }

    /* for each face */
    for(i = 0; i < nf; i++)
    {
      v1 = pfaces[3*i];
      v2 = pfaces[3*i+1];
      v3 = pfaces[3*i+2];

      u[0] = pvertices[3*v2] - pvertices[3*v1];
      u[1] = pvertices[3*v2+1] - pvertices[3*v1+1];
      u[2] = pvertices[3*v2+2] - pvertices[3*v1+2];

      v[0] = pvertices[3*v3] - pvertices[3*v1];
      v[1] = pvertices[3*v3+1] - pvertices[3*v1+1];
      v[2] = pvertices[3*v3+2] - pvertices[3*v1+2];

      cross_product(u, v, pfnormals+3*i);
    }
  }
  else
    pfnormals = NULL;

  return pfnormals;
}

/* compute vertex normals */
GLdouble * compute_vertex_normals(int nv, GLdouble* pvertices, int nf, GLuint* pfaces, GLdouble* pfnormals)
{
  int i, j;
  int *num;
  GLuint v;
  GLuint **pindex;
  GLdouble u[3];
  GLdouble *pvnormals;

  /* allocate memory */
  if(nv != 0)
  {
    num = (int*)malloc(sizeof(int)*nv);
    pindex = (GLuint**)malloc(sizeof(GLuint*)*nv);
    memset(num, 0, sizeof(int)*nv);
    memset(pindex, 0, sizeof(GLuint*)*nv);

    /* for each face, create a list for each vertex */
    for(i = 0; i < nf; i++)
    {
      for(j = 0; j < 3; j++)
      {
        v = pfaces[3*i+j];
        pindex[v] = realloc(pindex[v], sizeof(GLuint)*(num[v]+1));
        pindex[v][num[v]] = i;
        num[v]++;
      }
    }

    pvnormals = (GLdouble*)malloc(sizeof(GLdouble)*nv*3);
    if(pvnormals == NULL)
    {
      printf("Out of memory\n");
      return NULL;
    }

    /* for each vertex, average the normals of its faces */
    for(i = 0; i < nv; i++)
    {
      u[0] = 0;
      u[1] = 0;
      u[2] = 0;
      for(j = 0; j < num[i]; j++)
      {
        u[0] += pfnormals[3*pindex[i][j]];
        u[1] += pfnormals[3*pindex[i][j]+1];
        u[2] += pfnormals[3*pindex[i][j]+2];
      }
      normalize(u);
      
      pvnormals[3*i] = u[0];
      pvnormals[3*i+1] = u[1];
      pvnormals[3*i+2] = u[2];
    }

    /* clean up */
    free(num);
    for(i = 0; i < nv; i++)
    {
      if(pindex[i])
        free(pindex[i]);
    }
    free(pindex);
  }
  else
    pvnormals = NULL;

  return pvnormals;
}


/* drawing function */
void display(void)
{
  int i;
  GLdouble near = 10, far = 100;
  GLdouble persp[16] = {721.5377, 0, 0, 0,
                         0, 721.5377, 0, 0,
                         -609.5593, -172.8540, near+far, -1,
                         0, 0, near*far, 0};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  /* vertice array */
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, Vertices);
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_DOUBLE, 0, Vnormals);

  /* render CAD model */
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WIDTH, HEIGHT, 0, near, far);
  glMultMatrixd(persp);
  glViewport(0, 0, WIDTH, HEIGHT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* draw lines */
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColor3f(0.0, 0.0, 1.0);
  glDrawElements(GL_TRIANGLES, 3*Nface, GL_UNSIGNED_INT, Faces);

  /* hidden line removal */
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f(1.0, 1.0, 1.0);
  glDrawElements(GL_TRIANGLES, 3*Nface, GL_UNSIGNED_INT, Faces);
  glDisable(GL_POLYGON_OFFSET_FILL);

  glFlush();
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

int main(int argc, char** argv)
{
  char filename[BUFFERSIZE];

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("render_cad_model");

  /* filename of the off file */
  sprintf(filename, "%s/%02d.off", argv[1], atoi(argv[2])); 

  /* load off file */
  load_off_file(&Nvertice, &Vertices, &Nface, &Faces, filename);
  printf("load off file done\n");

  /* compute face normals */
  Fnormals = compute_face_normals(Nvertice, Vertices, Nface, Faces);

  /* compute vertex normals */
  Vnormals = compute_vertex_normals(Nvertice, Vertices, Nface, Faces, Fnormals);

  /* draw the CAD model */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMainLoop();
  return 0;
}

/* matlab interface */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  int i;
  double *p;
  int argc = 1;
  char* argv[1];
  argv[0] = "";

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("render_cad_model");
 
  if (nrhs != 2)
    mexErrMsgTxt("Wrong number of inputs");

  /* parse vertices and faces */
  p = (double*)mxGetPr(prhs[0]);
  Nvertice = mxGetN(prhs[0]);
  Vertices = (GLdouble*)malloc(sizeof(GLdouble)*Nvertice*3);
  for(i = 0; i < 3*Nvertice; i++)
    Vertices[i] = (GLdouble)p[i];

  p = (double*)mxGetPr(prhs[1]);
  Nface = mxGetN(prhs[1]);
  Faces = (GLuint*)malloc(sizeof(GLuint)*Nface*3);
  for(i = 0; i < 3*Nface; i++)
    Faces[i] = (GLuint)p[i];

  /* compute face normals */
  Fnormals = compute_face_normals(Nvertice, Vertices, Nface, Faces);

  /* compute vertex normals */
  Vnormals = compute_vertex_normals(Nvertice, Vertices, Nface, Faces, Fnormals);

  /* draw the CAD model */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
  glutMainLoop();
  return;
}
