CC = g++
CFLAGS = -Wall
PROG = carrom

SRCS = carrom.cpp

ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT
else
	LIBS = -lglut -lGL -lGLU -lm 
endif

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -std=c++11 -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
