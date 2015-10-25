#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define WIDTH 200
#define HEIGHT 200
#define BUFFERSIZE 256
#define AZIMUTH_NUM 8
#define ELEVATION_NUM 6

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


/* compare the depth value to its neighborhoods */
int is_good_point(GLfloat *depth, int i, int j, GLdouble threshold)
{
  int flag;
  GLdouble d, dn;

  flag = 1;
  d = depth[j*WIDTH+i];

  if(i >= 1)
  {
    dn = depth[j*WIDTH+i-1];
    if(fabs(d - dn) > threshold)
      flag = 0;
  }

  if(i+1 < WIDTH)
  {
    dn = depth[j*WIDTH+i+1];
    if(fabs(d - dn) > threshold)
      flag = 0;
  }

  if(j >= 1)
  {
    dn = depth[(j-1)*WIDTH+i];
    if(fabs(d - dn) > threshold)
      flag = 0;
  }

  if(j+1 < HEIGHT)
  {
    dn = depth[(j+1)*WIDTH+i];
    if(fabs(d - dn) > threshold)
      flag = 0;
  }

  return flag;
}


/* drawing function */
void display(void)
{
  FILE *fp;
  char filename[BUFFERSIZE];
  int i, j, aind, eind;
  int num, flag;
  GLint viewport[4];
  GLdouble mvmatrix[16], projmatrix[16];
  GLdouble x, y, z, threshold = 0.01;
  GLdouble *data;
  GLdouble a = 0, e = 0, d = 3;
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

  /* render CAD model */
  data = NULL;
  num = 0;  
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
      /* gluPerspective(30.0, 1.0, d-0.5, d+0.5); */
      glOrtho(-0.5, 0.5, -0.5, 0.5, d-0.5, d+0.5);
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

      /* get the matrices */
      glGetIntegerv(GL_VIEWPORT, viewport);
      glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
      glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

      /* read depth buffer */
      glReadPixels(1, 1, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

      /* get 3D points */
      for(j = HEIGHT-1; j >= 0; j--)
      {
        for(i = 0; i < WIDTH; i++)
        {
          if(depth[j*WIDTH+i] < 1 && depth[j*WIDTH+i] > 0)
          {
            flag = is_good_point(depth, i, j, threshold);
            if(flag == 0)
              continue;
            num++;
            data = (GLdouble*)realloc(data, sizeof(GLdouble)*num*3);
            gluUnProject((GLdouble)i, (GLdouble)j, depth[j*WIDTH+i], mvmatrix, projmatrix, viewport, &x, &y, &z);
            data[(num-1)*3] = x;
            data[(num-1)*3+1] = y;
            data[(num-1)*3+2] = z;
          }
        }
      }
      printf("a = %.2f, e = %.2f, %d points in total\n", a, e, num);

      glFlush();
    }
  }

  /* open  file */
  sprintf(filename, "%s_surf.off", Filebase);
  fp = fopen(filename, "w");  

  /* write off header */
  fprintf(fp, "OFF\n");
  fprintf(fp, "%d 0 0\n", num);

  /* write 3D points */
  for(i = 0; i < num; i++)
  {
    x = data[i*3];
    y = data[i*3+1];
    z = data[i*3+2];
    fprintf(fp, "%f %f %f\n", x, y, z);
  }

  fclose(fp);
  free(data);
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
  glutCreateWindow("render_depth_images");

  /* filename of the off file */
  sprintf(filename, "%s/%s.off", argv[1], argv[2]);
  sprintf(Filebase, "%s/%s", argv[1], argv[2]); 

  /* load off file */
  load_off_file(&Nvertice, &Vertices, &Nface, &Faces, filename);
  printf("load off file done\n");

  /* draw the CAD model */
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMainLoop();
  return 0;
}
