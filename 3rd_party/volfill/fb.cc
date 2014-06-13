#include <GL/gl.h>
#include <string.h>

#include "fb.h"

FrameBuffer::FrameBuffer(int cx, int cy)
{
	framebuffer = NULL;
	
	Init(cx, cy);
}

FrameBuffer::~FrameBuffer()
{
	if (framebuffer)
		delete [] framebuffer;
}

void FrameBuffer::Init(int cx, int cy)
{
	if (framebuffer)
		delete [] framebuffer;
	
	framebuffer = new unsigned char [3 * cx * cy];
	
	width = cx;
	height = cy;
}

void FrameBuffer::ClearFrameBuffer(unsigned char red, unsigned char green, unsigned
																	 char blue)
{
	int i, j;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			framebuffer[3 * (i * width + j)] = red;
			framebuffer[3 * (i * width + j)+1] = green;
			framebuffer[3 * (i * width + j)+2] = blue;
		}
	}
}

void FrameBuffer::DrawPixel(unsigned char red, unsigned char green, unsigned char blue,
														int x, int y)
{
	if (0 <= x && x < width &&
			0 <= y && y < height) {
		int offset = 3 * (y * width + x);
		
		framebuffer[offset + 0] = red;
		framebuffer[offset + 1] = green;
		framebuffer[offset + 2] = blue;
	}
}

void FrameBuffer::ReadPixel(unsigned char *red, unsigned char *green, unsigned char
														*blue, int x, int y)
{
	if (0 <= x && x < width &&
			0 <= y && y < height) {
		int offset = 3 * (y * width + x);
		
		*red = framebuffer[offset+0];
		*green = framebuffer[offset+1];
		*blue = framebuffer[offset+2];
	}
}

void FrameBuffer::Draw()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glOrtho(0, width, 0, height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);

	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
	
	// restore state
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
}

void FrameBuffer::Write(char *filename)
{
	// PPM code taken from
	// http://www.cs.orst.edu/~tgd/classes/450/mpeg.html
	register int x, y;
	unsigned char   r, g, b;
	FILE *fp;
	
	printf(filename);
	fp = fopen(filename, "wb");
	if (fp == NULL)
		return;
	
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "%d\n", 255);
	
	for ( y = 0; y < height; y++ ) {
		for ( x = 0; x < width; x++ )
			{
				r = framebuffer[3 * (y * width + x) + 0];  
				g = framebuffer[3 * (y * width + x) + 1];    
				b = framebuffer[3 * (y * width + x) + 2];
				
				fwrite(&r, 1, 1, fp);
				fwrite(&g, 1, 1, fp);
				fwrite(&b, 1, 1, fp);
			}
	}

	fclose(fp);
}



