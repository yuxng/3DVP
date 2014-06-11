CFLAGS = 
LDFLAGS = -lm -lGL -lGLU -lglut

all: render_depth_images

render_depth_images: render_depth_images.o
	gcc render_depth_images.o -o render_depth_images $(LDFLAGS)

render_depth_images.o: render_depth_images.c
	gcc -c $(CFLAGS) render_depth_images.c -o render_depth_images.o

clean:
	rm render_depth_images
	rm render_depth_images.o
