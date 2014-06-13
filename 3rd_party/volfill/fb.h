#ifndef FB_H
#define FB_H

#include <stdio.h>

class FrameBuffer {
 public:
	FrameBuffer() : width(0), height(0) { framebuffer = NULL; }
	FrameBuffer(int cx, int cy);
	~FrameBuffer();
	
	void Init(int cx, int cy);

	void ClearFrameBuffer(unsigned char red, unsigned char green, unsigned
												char blue);
	void DrawPixel(unsigned char red, unsigned char green, unsigned char blue,
								 int x, int y);
	void ReadPixel(unsigned char *red, unsigned char *green, unsigned char
								 *blue, int x, int y);
	unsigned char *getFramebuffer() { return framebuffer; };
	
	void Draw();
	
	void Write(char *filename);

 private:
	unsigned char *framebuffer;

	int width, height;
};

#endif 
