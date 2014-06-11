#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WIDTH 500
#define HEIGHT 500
#define BUFFERSIZE 256
#define AZIMUTH_NUM 24
#define ELEVATION_NUM 12

/* global variables */
char Filebase[BUFFERSIZE];
int Nvertice, Nface;
GLfloat *Vertices;
GLuint *Faces;

/* load a CAD model from a .off file */
int load_off_file(int* pnv, GLfloat** pvertices, int* pnf, GLuint** pfaces, char* filename)
{
  FILE* fp;
  char buffer[BUFFERSIZE];
  int nv, nf, aux, i;
  GLfloat *vertices;
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
  vertices = (GLfloat*)malloc(sizeof(GLfloat)*nv*3);
  if(vertices == NULL)
  {
    printf("Out of memory!\n");
    return 1;
  }

  /* read vertices */
  for(i = 0; i < 3*nv; i++)
    fscanf(fp, "%f", vertices+i);

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

/* drawing function */
void display(void)
{
  FILE *fp;
  char filename[BUFFERSIZE];
  int i, j, aind, eind;
  GLdouble a = 0, e = 0, d = 2;
  GLfloat *depth;

  depth = (GLfloat*)malloc(sizeof(GLfloat)*WIDTH*HEIGHT);
  if(depth == NULL)
  {
    printf("Malloc error for depth\n");
    exit(1);
  }

  glEnable(GL_DEPTH_TEST);

  /* vertice array */
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, Vertices);

  for(aind = 0; aind < AZIMUTH_NUM; aind++)
  {
    a = aind * (360.0 / AZIMUTH_NUM);
    for(eind = 0; eind < ELEVATION_NUM+1; eind++)
    {
      e = -90 + eind * (180.0 / ELEVATION_NUM);

      glClearColor(1.0, 1.0, 1.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(30.0, 1.0, d-0.5, d+0.5);
      glViewport(0, 0, WIDTH, HEIGHT);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(0.0, 0.0, -d);
      glRotatef(e-90.0, 1.0, 0.0, 0.0);
      glRotatef(-a, 0.0, 0.0, 1.0);

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

      /* read depth buffer */
      glReadPixels(1, 1, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

      sprintf(filename, "%s_a%02d_e%02d.txt", Filebase, aind, eind);
      fp = fopen(filename, "w");
      for(j = 0; j < HEIGHT; j++)
      {
        for(i = 0; i < WIDTH; i++)
          fprintf(fp, "%f ", depth[j*WIDTH+i]);
        fprintf(fp, "\n");
      }
      fclose(fp);

      glFlush();
    }
  }
  free(depth);
  exit(0);
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
  glutCreateWindow("anchor_depth_test");

  /* filename of the off file */
  sprintf(filename, "%s/%02d.off", argv[1], atoi(argv[2]));
  sprintf(Filebase, "%s/%02d", argv[1], atoi(argv[2]));  

  /* load off file */
  load_off_file(&Nvertice, &Vertices, &Nface, &Faces, filename);
  printf("load off file done\n");

  /* draw the CAD model */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMainLoop();
  return 0;
}
