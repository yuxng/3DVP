ifeq ($(shell uname),Linux)
CC = g++ -O2 -g
else
CC = CC -O
endif

LIBS = -lGLU -lGL -lglut

volfill : $(wildcard *.cc) $(wildcard *.cpp)
	$(CC) -o $@ $^ $(LIBS)
