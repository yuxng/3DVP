CFLAGS = 
LDFLAGS = -lm -lGL -lGLU -lglut

all: render_depth_images render_cad_model

render_depth_images: render_depth_images.o
	gcc render_depth_images.o -o render_depth_images $(LDFLAGS)

render_depth_images.o: render_depth_images.c
	gcc -c $(CFLAGS) render_depth_images.c -o render_depth_images.o

render_cad_model: render_cad_model.o
	mex render_cad_model.o -o render_cad_model $(LDFLAGS)

render_cad_model.o: render_cad_model.c
	mex -c $(CFLAGS) render_cad_model.c -o render_cad_model.o

clean:
	rm render_depth_images
	rm render_depth_images.o
	rm render_cad_model
	rm render_cad_model.o
