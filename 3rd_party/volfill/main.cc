#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <iostream>
#include <strings.h>
#include <stdlib.h>

#include "OccGridRLE.h"
#include "ActiveGrid.h"

#include "fb.h" // the drawing frame buffer
#include "util.h"

using namespace std;

FrameBuffer fb;
ActVoxelGrid actGrid;
OccGridRLE *ogSrc;
int sweep = 73; //147; //236; //43; //236; //65;
int sweepMax;

int drawType = 3;
bool useGUI = false;
int nIterations = 10;
char *inFile, *outfile;
unsigned char d1limit = 10, d2limit = 20;
bool propD2 = false;

// structure to hold general information about the window
struct {
	int width, height;
} winInfo;

void Usage() {
	cerr << "Usage: volfill <in.vri> <out.vri> ...  " << endl; 
	cerr << "-g" << endl;
	cerr << "-n <number of Iterations>" << endl;
	cerr << "-d1 <d1 distance>" << endl;
	cerr << "-d2 <d2 distance>" << endl;
	cerr << "-p" << endl;
	exit(1);
}

// OpenGL display function
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	actGrid.draw(&fb, sweep, drawType, 10, 20);
	fb.Draw();

	glutSwapBuffers();
	glFlush();
}

// initialization function
void init(void)
{
	glClearColor(0,0,0,0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void idle(void)
{
	//display();
}

void Keyboard( unsigned char key, int x, int y )
{
	OccGridRLE *ogDst;
	FILE *fp;
	char buffer[500];

	switch (key) {
	case 'm':
		fp = fopen("movie.list", "wt");
		for (int i=0; i<nIterations; i++) {
			actGrid.blur(d1limit, d2limit);
			display();
			sprintf(buffer, "blur.%d.ppm", i+1);
			fb.Write(buffer);
			fprintf(fp, "%s\n", buffer);
		}
		fclose(fp);

		display();
		break;
	case 'l':
		for (int i=0; i<nIterations; i++) { 
			actGrid.blur(d1limit, d2limit);
			PrintResourceUsage("After an Iteration");
		}

		display();
		break;
	case 'b':
		PrintResourceUsage("Before an Iteration");
		actGrid.blur(d1limit, d2limit);
		PrintResourceUsage("After an Iteration");
		display();
		break;
	case 'w':
		cerr << "Writing output...";
		ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
		if (!ogSrc->read(inFile)) Usage();

		ogDst = new
			OccGridRLE(ogSrc->xdim,ogSrc->ydim,ogSrc->zdim,CHUNK_SIZE);
		ogDst->origin[0] = ogSrc->origin[0];
		ogDst->origin[1] = ogSrc->origin[1];
		ogDst->origin[2] = ogSrc->origin[2];
		ogDst->resolution = ogSrc->resolution;
		
		actGrid.saveToOccGridRLE(ogSrc, ogDst);
		ogDst->write(outfile);			 
		
		delete ogSrc;
		delete ogDst;

		cerr << "Done" << endl;

		break;
	case '1':
		drawType = 0;
		display();
		break;
	case '2':
		drawType = 1;
		display();
		break;
	case '3':
		drawType = 2;
		display();
		break;
	case '4':
		drawType = 3;
		display();
		break;
	case 'a':
		sweep++;
		if (sweep >= sweepMax)
			sweep = sweepMax - 1;
		display();
		break;
	case 's':
		sweep--;
		if (sweep < 0)
			sweep = 0;
		display();
		break;
	}
}

void ProcessArguments(int argc, char *argv[])
{
	for (int i=3; i<argc; i++) {
		if (!strcasecmp(argv[i], "-g"))
			useGUI = true;
		if (!strcasecmp(argv[i], "-n")) {
			if (++i < argc) {
				nIterations = atoi(argv[i]);
			} else 
				Usage();
		}
		if (!strcasecmp(argv[i], "-d1")) {
			if (++i < argc)
				d1limit = atoi(argv[i]);
			else
				Usage();
		}
		if (!strcasecmp(argv[i], "-d2")) {
			if (++i < argc)
				d2limit = atoi(argv[i]);
			else
				Usage();
		}
		if (!strcasecmp(argv[i], "-p")) {
			propD2 = true;
		}
	}
}

int main(int argc, char *argv[]) {
	PrintResourceUsage("At Beginning of program");
	// load the input
	if (argc < 3) Usage();

	// read the in and out files
	inFile = argv[1];
	outfile = argv[2];
	
	ProcessArguments(argc, argv);
	
	ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
	if (!ogSrc->read(inFile)) Usage();
	PrintResourceUsage("After loading grid");
	
	// load in the grid
	actGrid.loadFromOccGridRLE(ogSrc, d2limit);

	winInfo.width = ogSrc->xdim; 
	winInfo.height = ogSrc->ydim;
	
	sweep = ogSrc->zdim / 2;
	sweepMax = ogSrc->zdim;
	
	// delete the old source
	delete ogSrc;

	// initialize distance information based on the OccGridRLE
	actGrid.initDistances(d2limit, propD2);

	PrintResourceUsage("After loading in OccGrid");

	if (useGUI) {		
		// init glut and the framebuffer
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
		
		PrintResourceUsage("After initialization and before starting OpenGL/Frame Buffer");
		
		fb.Init(winInfo.width, winInfo.height);
		
		glutInitWindowSize(winInfo.width, winInfo.height);
		glutInitWindowPosition(100, 100);
		glutCreateWindow("holeGL");
		
		init();
		
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutKeyboardFunc(Keyboard);
		
		glutMainLoop();
	} else {
		OccGridRLE *ogDst;

		// non graphical version
		for (int i=0; i<nIterations; i++) {
			cout << i+1 << ":";
			actGrid.blur(d1limit, d2limit);
			PrintResourceUsage("After an Iteration");
		}
		
		cerr << "Writing output...";
		ogSrc = new OccGridRLE(1,1,1, CHUNK_SIZE);
		if (!ogSrc->read(inFile)) Usage();

		ogDst = new
			OccGridRLE(ogSrc->xdim,ogSrc->ydim,ogSrc->zdim,CHUNK_SIZE);
		ogDst->origin[0] = ogSrc->origin[0];
		ogDst->origin[1] = ogSrc->origin[1];
		ogDst->origin[2] = ogSrc->origin[2];
		ogDst->resolution = ogSrc->resolution;
		
		actGrid.saveToOccGridRLE(ogSrc, ogDst);
		ogDst->write(outfile);			 
		
		delete ogSrc;
		delete ogDst;

		cerr << "Done" << endl;
	}

	return 0;
}



