all: learn evaluate_object

learn: learn.cc
	g++ -O3 -D_FILE_OFFSET_BITS=64 -o learn learn.cc
evaluate_object: evaluate_object.cpp
	g++ -O3 -g -o evaluate_object evaluate_object.cpp

clean:
	/bin/rm learn
	/bin/rm *.mex*
	/bin/rm evaluate_object
